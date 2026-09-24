# Volume 22: Ed25519 Identity

**signatures instead of RSA**

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

- What a digital signature gives you (authenticity + integrity, not secrecy)
- Why Ed25519 replaces RSA for device identity
- The real `identity.c` and how it is verified against RFC 8032
- Where identity fits in the hardened stack

## 1. What a Signature Gives You

A signature proves **who** produced a message and that the message is unchanged.
It does **not** hide the message (that is encryption). On a device you use signatures to:

- sign firmware so the bootloader can verify it (secure boot);
- sign telemetry so a collector can trust it;
- sign commands so the device can trust them.

## 2. Why Ed25519, Not RSA

| property | Ed25519 | RSA-2048/3072 |
| -------- | ------- | ------------- |
| public key size | 32 B | 256-384 B |
| signature size | 64 B | 256-384 B |
| signing speed | fast | slow |
| padding pitfalls | none | PKCS#1/ OAEP/PSS |
| constant-time | yes | needs care |

RSA survives only for **X.509/TLS compatibility** (`docs/19`).

## 3. The Real Source - `include/identity.h`

```c
#define IDENTITY_KEY_SIZE 32u
#define IDENTITY_SIG_SIZE 64u

bool identity_pubkey(const uint8_t priv[IDENTITY_KEY_SIZE],
                     uint8_t pub[IDENTITY_KEY_SIZE]);
bool identity_sign(const uint8_t priv[IDENTITY_KEY_SIZE], const uint8_t *msg,
                   size_t msg_len, uint8_t sig[IDENTITY_SIG_SIZE]);
bool identity_verify(const uint8_t pub[IDENTITY_KEY_SIZE], const uint8_t *msg,
                     size_t msg_len, const uint8_t sig[IDENTITY_SIG_SIZE]);
```

## 4. The Real Source - `src/identity.c`

The signing path builds the key object, initialises the context, and signs:

```c
bool identity_sign(const uint8_t priv[IDENTITY_KEY_SIZE], const uint8_t *msg,
                   size_t msg_len, uint8_t sig[IDENTITY_SIG_SIZE]) {
    EVP_PKEY *k = identity_priv(priv);
    EVP_MD_CTX *c = EVP_MD_CTX_new();
    size_t n = 64u;
    bool ok = (k != NULL) && (c != NULL) && ed_sign_init(c, k) &&
              (EVP_DigestSign(c, sig, &n, msg, msg_len) == 1);
    EVP_MD_CTX_free(c);
    EVP_PKEY_free(k);
    return ok && (n == 64u);
}
```

## 5. Verified Against RFC 8032

```bash
$ python3 scripts/run_tests.py | grep -E 'identity'
:test_identity_rfc8032:PASS
:test_identity_verify_bad:PASS
```

`test_identity_rfc8032` pins the public key and signature to the **RFC 8032 test 1**
vector; `test_identity_verify_bad` flips a signature byte and expects rejection.

## 6. Where Identity Fits

| layer | primitive | use |
| ----- | --------- | --- |
| identity | **Ed25519** | sign/verify firmware, telemetry, commands |
| key agreement | X25519 + HKDF | session keys (`docs/21`) |
| data | AES-256-GCM | protect the bytes (`docs/19`) |

Secure boot is the headline use: if U-Boot verified an Ed25519 signature over the kernel
before jumping, the entire image-modification class would close.

## Labs

1. Run `test_identity_rfc8032` and record the public key and signature.
2. Sign a message and verify it; then flip one signature bit and show rejection.
3. Explain why a signature and an AEAD tag are different things.
4. Describe how Ed25519 would enable secure boot on the TELESCREEN.

## Reference

- `src/identity.c`, `include/identity.h`
- RFC 8032 (Ed25519)
- `docs/21` (X25519/HKDF), `docs/27` (flash and verify)
