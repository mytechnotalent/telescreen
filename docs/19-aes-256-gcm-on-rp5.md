# Volume 19: AES-256-GCM on RP5

**using the ARMv8 crypto extensions**

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

## What You'll Learn

- Why AES-256-GCM is the natural choice on the RP5
- The ARMv8 Cryptography Extensions and the instructions involved
- The GCM construction (CTR + GHASH) and the nonce rules
- The real implementation call path

## 1. Why AES-256-GCM Here

The RP5's BCM2712 (Cortex-A76) implements the **ARMv8 Cryptography Extensions**:

| instruction | role |
| ----------- | ---- |
| `AESE` | AES encrypt round (SubBytes + ShiftRows) |
| `AESD` | AES decrypt round |
| `AESMC` | MixColumns |
| `AESIMC` | inverse MixColumns |
| `PMULL` | carry-less multiply (for GHASH) |

So AES-GCM is **hardware-accelerated** and, because the round instructions are
data-independent, **constant-time** against cache-timing attacks. This is why, on the
RP5, AES-256-GCM is the default backend.

## 2. The GCM Construction

GCM = **CTR mode** for confidentiality + **GHASH** for the tag.

```
H    = AES_K(0^128)
tag  = GHASH_H(AAD, ciphertext) XOR AES_K(J0)
```

The 96-bit nonce is used directly as the CTR block; it **must never repeat** under one
key.

## 3. The Real Call Path

`aead_init(AEAD_ALGO_AES_256_GCM)` selects the backend; `aead_seal` drives OpenSSL EVP,
which selects the ARMv8-accelerated AES implementation on the RP5:

```c
static int gcm_enc_init(EVP_CIPHER_CTX *ctx, const uint8_t *key, const uint8_t *nonce) {
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) return 0;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)AEAD_AES_NONCE_SIZE, NULL) != 1) return 0;
    return EVP_EncryptInit_ex(ctx, NULL, NULL, key, nonce) == 1;
}
```

`open` sets the expected tag (`EVP_CTRL_GCM_SET_TAG`) and `EVP_DecryptFinal_ex` returns
success **only** if the tag verifies.

## 4. Key and Nonce Sizes

| item | size |
| ---- | ---- |
| key | 32 bytes (AES-256) |
| nonce | 12 bytes (96-bit, recommended) |
| tag | 16 bytes (128-bit) |

## 5. Why Not RSA

RSA is slow, has large keys, has padding pitfalls (PKCS#1 v1.5 -> Bleichenbacher), and
gives **no forward secrecy**. On a router it is used only for **X.509/TLS
compatibility**. The modern stack is **ECC + AEAD**: X25519 to agree a key, AES-GCM to
protect the data (`docs/21`).

## 6. Verify It

```bash
$ python3 scripts/run_tests.py | grep -E 'aes'
:test_aead_init_aes:PASS
:test_aead_seal_open_aes:PASS
:test_aead_open_aes_bad_tag:PASS
```

## Labs

1. Seal with AES-256-GCM and open; confirm the round trip.
2. Corrupt the tag and confirm `AUTHENTICATION_FAILED`.
3. Name the four ARMv8 instructions AES uses.
4. Explain why the RP5's AES is constant-time while a table-based AES is not.

## Reference

- `src/aead.c`, `include/aead.h`
- ARMv8-A Cryptography Extensions; NIST SP 800-38D
- `docs/20` (XChaCha20), `docs/21` (X25519/HKDF)
