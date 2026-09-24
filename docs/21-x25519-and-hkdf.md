# Volume 21: X25519 and HKDF

**key agreement and derivation**

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

- How X25519 produces a shared secret (and why it is not the key)
- How HKDF turns that secret into a session key
- The real `kex.c` and how it is verified against RFC vectors
- Why this is the **correct** replacement for the weak KDF (`docs/17`)

## 1. The Two Steps

You cannot AES-encrypt with a raw ECDH output. The correct two-step is:

```
1. X25519(priv, peer_pub) -> shared secret        ; a point/scalar, not a key
2. HKDF(shared, salt, info) -> session key        ; a uniform key
```

## 2. X25519

X25519 is ECDH on Curve25519. Each side multiplies its private scalar by the other's
public key and both arrive at the **same** shared secret. It gives **forward secrecy**:
a stolen long-term key does not reveal past sessions.

```c
bool kex_x25519(const uint8_t priv[KEX_KEY_SIZE], const uint8_t peer[KEX_KEY_SIZE],
                uint8_t out[KEX_KEY_SIZE]);
```

## 3. HKDF-SHA256

HKDF is **extract** then **expand**:

```
PRK = HMAC-SHA256(salt, IKM)                      ; extract
OKM = T(1) || T(2) || ...                          ; expand
T(i)= HMAC-SHA256(PRK, T(i-1) || info || i)
```

```c
bool kex_hkdf(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
              size_t salt_len, const uint8_t *info, size_t info_len,
              uint8_t *out, size_t out_len);
```

## 4. Verified Against the Standards

This is the part that makes the implementation trustworthy: the tests pin the outputs to
the **published vectors**, not to themselves.

| primitive | standard | vector |
| --------- | -------- | ------ |
| X25519 | **RFC 7748** | Alice/Bob published keys -> shared secret |
| HKDF-SHA256 | **RFC 5869** | test case 1, L = 42 |

```bash
$ python3 scripts/run_tests.py | grep -E 'rfc'
:test_kex_x25519_rfc7748:PASS
:test_kex_hkdf_rfc5869:PASS
```

> During development the RFC 5869 vector **caught a real bug**: the HKDF block function
> returned a `bool` where the caller consumed a length, collapsing expansion to one byte
> per block. The vector found it. This is why standard vectors are mandatory.

## 5. Use It

```c
uint8_t shared[32];
uint8_t key[32];
static const uint8_t salt[16] = { /* per-session salt */ };
static const uint8_t info[] = "telescreen-session";

kex_x25519(my_priv, peer_pub, shared);
kex_hkdf(shared, sizeof(shared), salt, sizeof(salt), info, sizeof(info),
         key, sizeof(key));
```

Then seal with the key (`docs/18`).

## 6. The Full Hardened Stack

| job | primitive |
| --- | --------- |
| key agreement | X25519 |
| key derivation | HKDF-SHA256 |
| data protection | AES-256-GCM (or XChaCha20-Poly1305) |
| identity | Ed25519 (`docs/22`) |
| password -> key | Argon2id (`docs/23`) |

## Labs

1. Run `test_kex_x25519_rfc7748` and confirm the shared secret matches RFC 7748.
2. Run `test_kex_hkdf_rfc5869` and confirm the OKM matches RFC 5869.
3. Derive a second key with a different `info` and show it differs.
4. Rewrite `beacon_weak_key` to use HKDF and show the beacon still opens.

## Reference

- `src/kex.c`, `include/kex.h`
- RFC 7748 (X25519), RFC 5869 (HKDF)
- `docs/17` (the failure), `docs/22` (identity)
