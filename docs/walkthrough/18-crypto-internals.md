# Walkthrough 18: Cryptography Internals

**what is actually happening inside the primitives**

***
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. 

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:

1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws regarding cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**

***

## Why Read Internals

The API hides the math, which is usually a good thing. But a security engineer should know
*why* a nonce must not repeat, *why* a tag catches a forgery, and *why* a public-derived
key is worthless. This volume opens the box far enough to make those facts obvious, using
the same symbols the code uses.

## 1. AES - the Block Cipher

AES (Rijndael) is a **substitution-permutation network** on a 16-byte block. The 128-bit
block is arranged as a 4x4 matrix of bytes - the **state**.

For **AES-256** there are **14 rounds**. Each round is four transforms:

| step | what it does |
| ---- | ------------ |
| `SubBytes` | replace every byte via the S-box (non-linear) |
| `ShiftRows` | rotate each row left by 0,1,2,3 |
| `MixColumns` | mix each column (linear diffusion) |
| `AddRoundKey` | XOR the round key |

The **non-linearity** (S-box) provides confusion; `ShiftRows` + `MixColumns` provide
diffusion across the whole state within a few rounds.

### The ARMv8 instructions

| instruction | maps to |
| ----------- | ------- |
| `AESE` | `AddRoundKey` then `SubBytes` then `ShiftRows` |
| `AESMC` | `MixColumns` |
| `AESD`, `AESIMC` | the decrypt equivalents |

A single `AESE` does three of the four steps in one instruction, which is why hardware AES
is both fast and (because there is no data-dependent table lookup) **not** vulnerable to
cache-timing attacks. That is the whole reason AES-256-GCM is the RP5 choice
(`docs/19`).

### The key schedule

AES-256 expands the 32-byte key into **15 round keys** (the initial + 14). The expansion
uses rotations, S-box lookups, and XOR with `Rcon`. It is deterministic and not secret
beyond the key itself.

## 2. GCM - the AEAD

GCM combines **CTR mode** (confidentiality) with **GHASH** (authentication). It is a
"two-for-one".

### 2.1 CTR mode

For each 16-byte **counter block**, compute `AES_K(counter)` and XOR it with the
plaintext. The counter starts from `J0`. Unlike CTR's usual 128-bit counter, GCM's `J0`
for a 96-bit nonce is simply:

```
J0 = nonce || 0x00000001
```

Then each subsequent block increments the low 32 bits. The **nonce must be unique**: two
messages with the same `J0` produce the same keystream, and `C1 XOR C2 = P1 XOR P2` - the
plaintexts are revealed without ever breaking AES.

### 2.2 GHASH

GHASH is a polynomial evaluation over `GF(2^128)`:

```
H     = AES_K(0^128)
S     = AAD * H^m + ... + C * H + len
tag   = GHASH_H(AAD, C) XOR AES_K(J0)
```

The multiplication uses `PMULL` (carry-less multiply) on ARMv8. Because it is a
polynomial MAC, a change to a **single** ciphertext bit changes the tag unpredictably.

### 2.3 Why the tag matters

`aead_open` (`docs/walkthrough/11`) sets the expected tag and calls
`EVP_DecryptFinal_ex`, which recomputes `GHASH` and compares. A **one-bit** change in the
ciphertext (or tag) fails. That is why `test_aead_open_aes_bad_tag` flips one bit and
expects failure.

### 2.4 The nonce-reuse catastrophe

If a nonce repeats:

- the keystream repeats, so `C1 XOR C2 = P1 XOR P2` (plaintext recovered);
- GHASH allows recovery of `H` from two messages, which allows **forging** tags.

So nonce reuse breaks **both** confidentiality and authenticity. This is the single most
important rule, and why `aead_seal` records the last nonce (`nonce_reused`).

## 3. ChaCha20 - the Stream Cipher

ChaCha20 is an **ARX** cipher: Add, Rotate, XOR. No S-box, no tables, constant-time in
software.

### 3.1 The state

The state is a 4x4 matrix of 32-bit words:

```
constant  constant  constant  constant
key[0]    key[1]    key[2]    key[3]
key[4]    key[5]    key[6]    key[7]
counter   counter   nonce     nonce
```

The four constants are `"expa" "nd 3" "2-by" "te k"` - exactly `HCHACHA_CONST` in
`aead.c` (`docs/walkthrough/11`).

### 3.2 The quarter-round

```
a += b; d ^= a; d = rotl(d, 16);
c += d; b ^= c; b = rotl(b, 12);
a += b; d ^= a; d = rotl(d,  8);
c += d; b ^= c; b = rotl(b,  7);
```

Twenty rounds (10 double-rounds of columns then diagonals) diffuse the state. Add the
original state to the diffused state, serialize, and you have a 64-byte keystream block.

### 3.3 The keystream and the stream

XOR the keystream with the plaintext. Decryption is the same operation. The counter in the
state is what makes each block unique - and, like GCM, the **nonce must be unique** or the
keystream repeats.

## 4. Poly1305 - the MAC

Poly1305 is a one-time authenticator:

```
acc = 0
for each 16-byte block m:
    acc = (acc + (m || 1)) * r  mod (2^130 - 5)
tag = (acc + s) mod 2^128
```

`r` and `s` are derived from the ChaCha key. The `|| 1` appends a bit so each block is
distinct; the modulus `2^130 - 5` is prime. The result is a 16-byte tag that is unfEasible
to forge without the key. **Poly1305 is one-time**: reusing `(r, s)` for two messages
breaks it - which is exactly why ChaCha20-Poly1305 is an AEAD and not two separate
primitives.

## 5. HChaCha20 and XChaCha20

XChaCha20 exists so you can use a **192-bit random nonce**. The construction:

```
subkey      = HChaCha20(key, nonce[0:16])
inner_nonce = 00000000 || nonce[16:24]
ciphertext  = ChaCha20_Poly1305(subkey, inner_nonce, plaintext)
```

HChaCha20 is ChaCha20's block function with a twist: it outputs **specific state words**
(the first and last rows) rather than the diffused-sum block. That is the subtlety in
`hchacha_store` (`docs/walkthrough/11`): words 0-3 and 12-15, not all 16.

The 192-bit nonce is large enough to choose **at random** per message: the collision
probability stays negligible, so no counter state is needed.

## 6. X25519 - Key Agreement

X25519 is ECDH on **Curve25519**: `y^2 = x^3 + 486662x^2 + x` over the prime field
`2^255 - 19`.

The secret is a **scalar**; the public key is `scalar * basepoint`. Two parties compute:

```
alice_shared = alice_scalar * bob_public
bob_shared   = bob_scalar   * alice_public
```

Both equal `alice_scalar * bob_scalar * basepoint` - the same point. The computation is a
**Montgomery ladder**: a fixed sequence of point additions/doublings that is the same for
every bit pattern, so it is constant-time against timing attacks. The shared secret is the
**x-coordinate** of the point.

`kex_x25519` (`docs/walkthrough/12`) is the whole thing in one OpenSSL call, verified
against RFC 7748.

## 7. HKDF - Key Derivation

A raw ECDH output is **not uniformly random**; it is a curve point coordinate. HKDF turns
it into a uniform key:

```
PRK = HMAC-SHA256(salt, IKM)                 ; extract
T(i)= HMAC-SHA256(PRK, T(i-1) || info || i)  ; expand
OKM = T(1) || T(2) || ...
```

- **extract** concentrates the entropy;
- **expand** stretches it to the desired length and binds it to **context** (`info`).

The `info` field is how you derive **different** keys from **one** secret for different
purposes ("telescreen-session", "telescreen-firmware", ...). Change `info`, get a
different key - the property `test_kex_hkdf_rfc5869` exercises.

## 8. Ed25519 - Signatures

Ed25519 is EdDSA over the twisted Edwards curve **edwards25519**, birationally equivalent
to Curve25519.

The signature is `(R, S)` where `R = r*B` and `S = r + H(R || A || M) * a`. The verifier
checks `S*B = R + H(R || A || M) * A`. Because the hash binds the message, the public key,
**and** `R`, any change breaks verification - which is why
`test_identity_verify_bad` catches one flipped byte.

Ed25519 is **deterministic** (no per-signature randomness means no RNG failure mode) and
**small** (32-byte keys, 64-byte signatures) - the reasons it replaces RSA for identity
(`docs/22`).

## 9. Argon2id - Password Hashing

Argon2 fills a large memory array and hashes it with data-dependent and data-independent
passes. The **memory** requirement is the defence: a GPU cannot parallelise cheaply when
each guess must allocate tens of MiB. `id` mixes both access patterns to resist side-channel
and GPU attacks.

Use it for **human** secrets; use X25519/HKDF for **machine** secrets (`docs/23`).

## 10. The One Table

| primitive | property it gives | its weakness if misused |
| --------- | ----------------- | ----------------------- |
| AES-256-GCM | confidentiality + integrity | nonce reuse |
| XChaCha20-Poly1305 | confidentiality + integrity | nonce reuse, one-time Poly1305 |
| X25519 | a shared secret | low-order points (handled) |
| HKDF | a uniform key | wrong `info` -> wrong key |
| Ed25519 | authenticity | nonce is deterministic (safe) |
| Argon2id | slow guessing | too-small memory |
| CRC-32 | accidental-corruption detection | **not** security |

The last row is the recurring lesson: integrity is not authenticity. A CRC keeps a careful
engineer honest; an AEAD tag keeps an **attacker** honest.

## Exercises

1. Explain why `C1 XOR C2 = P1 XOR P2` on nonce reuse.
2. Trace one ChaCha quarter-round on the state words from `HCHACHA_CONST`.
3. Explain why HChaCha20 outputs words 0-3 and 12-15.
4. Show how two HKDF outputs differ only by `info`.
5. State why Poly1305 is "one-time".

## Reference

- NIST SP 800-38D (GCM), RFC 8439 (ChaCha20-Poly1305), RFC 7748, RFC 5869, RFC 8032, RFC 9106
- `docs/18`-`docs/23`, `docs/walkthrough/11`, `docs/walkthrough/12`
