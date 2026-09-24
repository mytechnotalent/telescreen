# Volume 20: XChaCha20-Poly1305

**the portable twin with a 192-bit nonce**

***
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. 

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:

1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws relating to cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**

***

## What You'll Learn

- Why XChaCha20-Poly1305 exists and where ChaCha beats AES
- The HChaCha20 subkey derivation and the inner nonce
- The real implementation in `aead.c`
- When to pick it over AES-GCM

## 1. Why ChaCha

AES with table-based implementations leaks through cache timing. The ARMv8 AES
instructions remove that risk on the RP5, but on **software-only or 32-bit** targets
ChaCha20 is the better choice: it is built from **ARX** operations (add, rotate, xor)
that are naturally constant-time and fast in software, and it has **no S-box**.

## 2. The XChaCha Construction

XChaCha20-Poly1305 = HChaCha20 subkey derivation + IETF ChaCha20-Poly1305.

```
subkey      = HChaCha20(key, nonce[0:16])
inner_nonce = 00000000 || nonce[16:24]      ; 12-byte IETF nonce
```

The **192-bit outer nonce** means you can pick a **random** nonce per message with no
counter - the ergonomic win.

## 3. The Real Source - the HChaCha20 core

```c
static void quarter_round(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    *a += *b; *d ^= *a; *d = rotl32(*d, 16u);
    *c += *d; *b ^= *c; *b = rotl32(*b, 12u);
    *a += *b; *d ^= *a; *d = rotl32(*d, 8u);
    *c += *d; *b ^= *c; *b = rotl32(*b, 7u);
}
```

```c
static void hchacha20(const uint8_t key[32], const uint8_t nonce[24], uint8_t subkey[32]) {
    uint32_t state[16];
    hchacha_state(state, key, nonce);
    hchacha_rounds(state);
    hchacha_store(subkey, state);
}
```

## 4. The Inner Nonce

```c
static void build_inner_nonce(const uint8_t nonce[24], uint8_t out[12]) {
    memset(out, 0, 4u);
    memcpy(&out[4], &nonce[16], 8u);
}
```

Four zero bytes then the last eight nonce bytes: the IETF 96-bit nonce.

## 5. Seal and Open

The XChaCha path derives the subkey, builds the inner nonce, and seals with the IETF
construction. The derived `subkey` and `inner_nonce` are **wiped** afterward:

```c
r = xchacha_encrypt(subkey, inner, plaintext, sealed_out);
xchacha_wipe(subkey, inner);
```

## 6. AES-GCM vs XChaCha20-Poly1305

| property | AES-256-GCM | XChaCha20-Poly1305 |
| -------- | ----------- | ------------------ |
| hardware on RP5 | yes (ARMv8) | no (software) |
| nonce | 96-bit (counter) | **192-bit (random)** |
| constant-time | with ARMv8 instructions | yes in software |
| pick when | RP5 default | portable / random nonces |

**Recommendation:** implement both behind one API (this repo does) so students see the
same guarantees and learn *why* you choose one.

## 7. Verify It

```bash
$ python3 scripts/run_tests.py | grep -E 'xchacha'
:test_aead_init_xchacha:PASS
:test_aead_seal_open_xchacha:PASS
:test_aead_open_xchacha_bad_tag:PASS
```

## Labs

1. Seal/open with XChaCha20-Poly1305; confirm the round trip.
2. Trace `hchacha20` and name the ten double-rounds.
3. Explain the 192-bit nonce advantage in one sentence.
4. Give one deployment where you would choose XChaCha over AES-GCM.

## Reference

- `src/aead.c`, `include/aead.h`
- RFC 8439 (ChaCha20-Poly1305), draft-irtf-cfrg-xchacha
- `docs/19` (AES-GCM), `docs/21` (X25519/HKDF)
