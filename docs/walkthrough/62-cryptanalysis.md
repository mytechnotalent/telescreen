# Walkthrough 62: Cryptanalysis of Real-World Mistakes

**broken crypto in practice, and how to break it**

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

## Why Cryptanalysis

Real devices rarely ship broken **primitives**; they ship broken **usage**. This volume
catalogues the usage mistakes you will actually meet, and how each is broken. It is the
attacker's companion to the crypto volumes.

## 1. The Premise

> Modern ciphers are strong. **Key management, nonces, and modes are where devices fall.**

Every case below assumes AES or ChaCha is implemented correctly and still leaks.

## 2. Cases

### 2.1 Key Derived From Public Data

**Mistake.** `key = KDF(device_serial)`.
**Break.** Recompute the key from the (public) serial; decrypt.
**Lab.** B6 (`docs/17`).
**Test.** Two devices, different serials, keys are public.

### 2.2 Reused Nonce (GCM)

**Mistake.** A fixed or counter-reset nonce.
**Break.** `C1 XOR C2 = P1 XOR P2`; and GHASH lets you recover `H`, enabling tag forgery.
**Fix.** A monotonic sequence or a random 192-bit nonce (`docs/18`).
**Test.** Seal twice with one nonce, observe the guard.

### 2.3 ECB Mode

**Mistake.** ECB (no IV) on structured data.
**Break.** Equal plaintext blocks become equal ciphertext blocks - the classic "ECB penguin".
**Fix.** An AEAD, not ECB (`docs/18`).

### 2.4 Unauthenticated Encryption

**Mistake.** CTR/CBC without a MAC.
**Break.** Flip ciphertext bits to flip plaintext bits - a bit-flip attack; or pad-oracle in
CBC.
**Fix.** An AEAD (confidentiality **and** integrity).

### 2.5 Constant "Authentication"

**Mistake.** A fixed tag or magic.
**Break.** The "tag" never varies, so it authenticates nothing.
**Fix.** A real MAC, keyed.

### 2.6 Static IV

**Mistake.** The same IV for every message.
**Break.** In CBC, the same first block reveals the same plaintext prefix; in CTR, the
keystream repeats.
**Fix.** A per-message IV/nonce derived uniquely.

### 2.7 Weak Password KDF

**Mistake.** `sha256(password)`.
**Break.** GPU brute force at billions/s.
**Fix.** Argon2id (`docs/23`).

### 2.8 Embedded/Per-Model Keys

**Mistake.** One key for all units.
**Break.** Extract it from one device; own them all.
**Fix.** Per-device keys (`docs/21`).

### 2.9 Weak RNG

**Mistake.** A predictable RNG (a counter, `rand()`).
**Break.** Predict the "random" bytes (nonces, keys).
**Fix.** A CSPRNG.

### 2.10 Downgrade

**Mistake.** A fallback to weak crypto.
**Break.** Force the fallback.
**Fix.** No downgrade; a minimum strength.

## 3. The Breaking Method

```
1. Identify the cipher and the mode.
2. Identify the key source.
3. Identify the nonce/IV handling.
4. If the key is public   -> decrypt.
5. If the nonce repeats   -> XOR / forge.
6. If there is no tag     -> bit-flip.
7. If the KDF is weak     -> brute force.
```

## 4. The Detection (Blue Team)

| signal | meaning |
| ------ | ------- |
| identical IVs on the wire | static IV |
| repeated keystream patterns | nonce reuse |
| a constant byte block | ECB / no IV |
| a short or fixed "tag" | fake authentication |
| a key derivable from public data | no confidentiality |

## 5. The Lab's Mistakes

| lab | mistake | case |
| --- | ------- | ---- |
| B6 | key from public data | 2.1 |
| (historically) | constant tag | 2.5 |
| (historically) | nonce reuse | 2.2 |

The lab deliberately shipped these so the fixes (`docs/17`-`docs/23`) land.

## 6. The Engineering Answer

```
[ ] keys are secret and per-device
[ ] nonces never repeat
[ ] always an AEAD, never ECB, never unauthenticated
[ ] the KDF is memory-hard
[ ] the RNG is a CSPRNG
[ ] no downgrade
```

That is the whole of crypto engineering: not choosing a cipher, but **handling keys and
nonces**.

## Exercises

1. For each case, reproduce the break on the lab where possible.
2. Match each case to its control.
3. Add a case you have seen (e.g. a hardcoded IV in a firmware).
4. Write the five-line detection rule for nonce reuse.

## Reference

- `docs/17`-`docs/23`, `docs/walkthrough/18`
- NIST SP 800-38D; RFC 8439; the "Cryptographic Right Answers" guidance
