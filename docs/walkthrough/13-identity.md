# Walkthrough 13: `identity`

**every line of `src/identity.c`, explained**

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

## Why This Module

`identity.c` gives the TELESCREEN an **Ed25519 identity** (`docs/22`): derive a public key,
sign a message, verify a signature. It is the primitive that would enable **secure boot**,
and it replaces RSA for every place you would have used it.

```
identity_priv  (private)   build a private key object
identity_pub   (private)   build a public key object
identity_pubkey (public)   derive the public key
ed_sign_init   (private)   init a signing context
identity_sign  (public)    sign a message
ed_verify_init (private)   init a verifying context
identity_verify (public)   verify a signature
```

## 1. The Key Objects

```c
static EVP_PKEY *identity_priv(const uint8_t priv[32]) {
    return EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL, priv, 32u);
}

static EVP_PKEY *identity_pub(const uint8_t pub[32]) {
    return EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, NULL, pub, 32u);
}
```

Ed25519 keys are **raw 32 bytes**. These two helpers wrap OpenSSL's constructors so the
rest of the module never repeats the raw-key boilerplate.

## 2. `identity_pubkey`

```c
bool identity_pubkey(const uint8_t priv[IDENTITY_KEY_SIZE],
                     uint8_t pub[IDENTITY_KEY_SIZE]) {
    EVP_PKEY *k = identity_priv(priv);
    size_t n = 32u;
    bool ok = (k != NULL) && (EVP_PKEY_get_raw_public_key(k, pub, &n) == 1);
    EVP_PKEY_free(k);
    return ok && (n == 32u);
}
```

| line | what it does |
| ---- | ------------ |
| `k = identity_priv(priv)` | build the private key |
| `n = 32` | expected public-key length |
| `ok = (k) && get_raw_public_key(...)` | derive the public key |
| `EVP_PKEY_free(k)` | release the key |
| `return ok && (n == 32)` | success and expected length |

The public key is **derived**, not stored - you always compute it from the private seed on
demand.

## 3. Signing

```c
static bool ed_sign_init(EVP_MD_CTX *c, EVP_PKEY *k) {
    return EVP_DigestSignInit(c, NULL, NULL, NULL, k) == 1;
}
```

Ed25519 is a **one-shot** signature: `EVP_DigestSignInit` takes no digest (the `NULL`
arguments), because Ed25519 hashes internally.

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

| line | what it does |
| ---- | ------------ |
| `k`, `c` | build the key and a message-digest context |
| `n = 64` | the Ed25519 signature length |
| `ok = k && c && init && sign` | the four required successes, short-circuited |
| frees | release both objects |
| `return ok && (n == 64)` | success and expected length |

The `&&` chain means: if the key or context is null, the init/sign are skipped (they would
crash otherwise). Short-circuit evaluation is the guard.

## 4. Verifying

```c
static bool ed_verify_init(EVP_MD_CTX *c, EVP_PKEY *k) {
    return EVP_DigestVerifyInit(c, NULL, NULL, NULL, k) == 1;
}
```

```c
bool identity_verify(const uint8_t pub[IDENTITY_KEY_SIZE], const uint8_t *msg,
                     size_t msg_len, const uint8_t sig[IDENTITY_SIG_SIZE]) {
    EVP_PKEY *k = identity_pub(pub);
    EVP_MD_CTX *c = EVP_MD_CTX_new();
    bool ok = (k != NULL) && (c != NULL) && ed_verify_init(c, k) &&
              (EVP_DigestVerify(c, sig, 64u, msg, msg_len) == 1);
    EVP_MD_CTX_free(c);
    EVP_PKEY_free(k);
    return ok;
}
```

The mirror of signing, with `EVP_DigestVerify`. The signature length is passed literally
as `64u` (the only legal value).

## 5. Signature vs AEAD Tag

Students conflate these; they are different tools:

| property | signature (Ed25519) | tag (AEAD) |
| -------- | ------------------- | ---------- |
| proves | **who** wrote it | it was not modified |
| key | asymmetric (public verify) | symmetric (shared secret) |
| third-party | anyone can verify | only the key holders |
| use | firmware, identity, commands | transport integrity |

A signature lets a **verifier who does not hold the signing key** check authenticity -
which is exactly what a bootloader needs.

## 6. Verified Against RFC 8032

```bash
$ python3 scripts/run_tests.py | grep identity
:test_identity_rfc8032:PASS
:test_identity_verify_bad:PASS
```

- `rfc8032` pins the public key `d75a9801...` and the signature `e5564300...` to the
  **RFC 8032 test 1** vector (empty message).
- `verify_bad` flips one signature byte and expects verification to fail.

## 7. Where Identity Enables Secure Boot

```
U-Boot  -- verify Ed25519(kernel_sig, kernel, pubkey) -- jump to kernel
```

If U-Boot held a public key and checked a signature over the kernel before jumping, the
entire "replace the kernel" class would be closed. The TELESCREEN does **not** do this -
which is why the CTF can rebuild the firmware freely (`docs/27`).

## Exercises

1. Run the RFC 8032 test and record the public key and signature.
2. Sign a message; verify it; flip a signature byte and show rejection.
3. Explain why Ed25519 signing takes no digest argument.
4. Describe how this module would be wired into U-Boot for secure boot.

## Reference

- `src/identity.c`, `include/identity.h`
- RFC 8032 (Ed25519)
- `docs/22` (identity), `docs/27` (flash and verify)
