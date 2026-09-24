# Walkthrough 12: `kex`

**every line of `src/kex.c`, explained**

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

`kex.c` is the **correct** way to get a secret key (`docs/17`): X25519 for a shared secret,
HKDF-SHA256 to turn it into a session key. It is also the module where a real bug was
**caught by an RFC vector** during development - the best teaching moment in the project.

```
hkdf_hmac       (private)   one HMAC-SHA256
hkdf_assemble   (private)   build one expand block
hkdf_block      (private)   one T(i)
hkdf_step       (private)   produce + copy one block
hkdf_expand     (private)   the expand loop
kex_hkdf        (public)    extract + expand
x25519_derive   (private)   drive the EVP context
kex_free        (private)   release EVP objects
kex_x25519      (public)    the shared secret
```

## 1. HKDF, Recapped

```
extract:  PRK = HMAC-SHA256(salt, IKM)
expand :  T(1) = HMAC(PRK, info || 0x01)
          T(2) = HMAC(PRK, T(1) || info || 0x02)
          ...
          OKM  = T(1) || T(2) || ...   truncated to L
```

### `hkdf_hmac`

```c
static bool hkdf_hmac(const uint8_t *key, size_t key_len, const uint8_t *data,
                      size_t data_len, uint8_t out[32]) {
    unsigned int n = 0u;
    return HMAC(EVP_sha256(), key, (int)key_len, data, data_len, out, &n) != NULL;
}
```

One OpenSSL `HMAC` call. `n` receives the MAC length (32). Returns true only if OpenSSL
returned non-NULL.

### `hkdf_assemble`

```c
static size_t hkdf_assemble(uint8_t *buf, const uint8_t *tprev, size_t tlen,
                            const uint8_t *info, size_t info_len, uint8_t counter) {
    memcpy(buf, tprev, tlen);
    memcpy(&buf[tlen], info, info_len);
    buf[tlen + info_len] = counter;
    return tlen + info_len + 1u;
}
```

Builds the HMAC input for one block: `T(i-1) || info || counter`. Returns the total
length so the caller does not recompute it. This is the **expand** input format, exactly.

### `hkdf_block` - and the bug

```c
static size_t hkdf_block(const uint8_t prk[32], const uint8_t *info, size_t info_len,
                         const uint8_t *tprev, size_t tlen, uint8_t counter,
                         uint8_t out[32]) {
    uint8_t buf[97];
    size_t blen = hkdf_assemble(buf, tprev, tlen, info, info_len, counter);
    if (!hkdf_hmac(prk, 32u, buf, blen, out)) return 0u;
    return 32u;
}
```

**This function was wrong.** Its first version returned `bool` (the result of
`hkdf_hmac`), but the caller consumed the return as a **length**. `true` is `1`, so the
expand loop copied **one byte per block** instead of 32, and HKDF produced garbage.

The **RFC 5869 test vector caught it**: `test_kex_hkdf_rfc5869` failed with a memory
mismatch, and the diff pointed straight here. The fix was to return a `size_t` - the block
length on success, `0` on failure.

> This is why the lab pins cryptographic output to **published vectors** and not to
> "it round-trips". A round-trip would have passed (seal then open with the same wrong
> key still round-trips). Only the standard vector exposed the truth.

### `hkdf_step`

```c
static bool hkdf_step(const uint8_t prk[32], const uint8_t *info, size_t info_len,
                      uint8_t *t, size_t *tlen, uint8_t counter, uint8_t *out,
                      size_t *done, size_t out_len) {
    size_t n = hkdf_block(prk, info, info_len, t, *tlen, counter, t);
    size_t copy;
    if (n == 0u) return false;
    *tlen = n;
    copy = (out_len - *done < n) ? (out_len - *done) : n;
    memcpy(&out[*done], t, copy);
    *done += copy;
    return true;
}
```

| line | what it does |
| ---- | ------------ |
| `n = hkdf_block(...)` | produce T(i); **`n` is the length, 0 on failure** |
| `if (n == 0u) return false;` | propagate failure |
| `*tlen = n;` | remember T(i) for the next block |
| `copy = min(remaining, n)` | copy at most what is left (the final block may be partial) |
| `memcpy(&out[*done], t, copy)` | append T(i) to the output |
| `*done += copy;` | advance the write cursor |

Note `t` is both the previous block (input) and the new block (output) - `hkdf_block`
assembles the input into a local `buf` **before** writing `out`, so there is no aliasing
bug.

### `hkdf_expand`

```c
static bool hkdf_expand(const uint8_t prk[32], const uint8_t *info, size_t info_len,
                        uint8_t *out, size_t out_len) {
    uint8_t t[32];
    size_t done = 0u;
    size_t tlen = 0u;
    uint8_t counter = 1u;
    while (done < out_len) {
        if (!hkdf_step(prk, info, info_len, t, &tlen, counter, out, &done, out_len)) return false;
        counter += 1u;
    }
    return true;
}
```

The loop: start with `tlen = 0` (so the first block is just `info || 0x01`), produce blocks,
increment the counter, stop when `out_len` bytes are written. The `counter` starts at `1`
and increments - exactly the standard.

### `kex_hkdf`

```c
bool kex_hkdf(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
              size_t salt_len, const uint8_t *info, size_t info_len,
              uint8_t *out, size_t out_len) {
    uint8_t prk[32];
    if (!hkdf_hmac(salt, salt_len, ikm, ikm_len, prk)) return false;
    return hkdf_expand(prk, info, info_len, out, out_len);
}
```

Extract (`HMAC(salt, ikm)`) then expand. Two lines, total.

## 2. X25519

### `x25519_derive`

```c
static bool x25519_derive(EVP_PKEY_CTX *c, EVP_PKEY *b, uint8_t *out, size_t *n) {
    if (c == NULL) return false;
    if (EVP_PKEY_derive_init(c) != 1) return false;
    if (EVP_PKEY_derive_set_peer(c, b) != 1) return false;
    return EVP_PKEY_derive(c, out, n) == 1;
}
```

Three OpenSSL calls: init the derivation, set the peer key, derive. Each must return `1`;
splitting them keeps the required order literal and gives precise failure points.

### `kex_free`

```c
static void kex_free(EVP_PKEY_CTX *c, EVP_PKEY *a, EVP_PKEY *b) {
    EVP_PKEY_CTX_free(c);
    EVP_PKEY_free(a);
    EVP_PKEY_free(b);
}
```

Release the context and both keys. Frees are safe on `NULL` in OpenSSL.

### `kex_x25519`

```c
bool kex_x25519(const uint8_t priv[KEX_KEY_SIZE], const uint8_t peer[KEX_KEY_SIZE],
                uint8_t out[KEX_KEY_SIZE]) {
    EVP_PKEY *a = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, priv, 32u);
    EVP_PKEY *b = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, NULL, peer, 32u);
    EVP_PKEY_CTX *c = ((a != NULL) && (b != NULL)) ? EVP_PKEY_CTX_new(a, NULL) : NULL;
    size_t n = 32u;
    bool ok = x25519_derive(c, b, out, &n);
    kex_free(c, a, b);
    return ok && (n == 32u);
}
```

| line | what it does |
| ---- | ------------ |
| `a = new_raw_private_key(...)` | build the private key from the 32-byte scalar |
| `b = new_raw_public_key(...)` | build the peer public key from 32 bytes |
| `c = (a && b) ? new(a) : NULL` | make the context only if both keys exist |
| `n = 32` | expected output length |
| `ok = x25519_derive(c, b, out, &n)` | derive |
| `kex_free(c, a, b)` | release everything |
| `return ok && (n == 32)` | success **and** the expected length |

The `(a != NULL) && (b != NULL)` guard is what lets `x25519_derive` simply check `c ==
NULL` - the null handling is centralised.

## 3. Verified Against RFC

```bash
$ python3 scripts/run_tests.py | grep -E 'rfc'
:test_kex_x25519_rfc7748:PASS
:test_kex_hkdf_rfc5869:PASS
```

- **RFC 7748**: Alice's private + Bob's public (and vice versa) both yield the published
  shared secret `4a5d9d5b...`. Two directions, one secret.
- **RFC 5869 test case 1**: IKM/salt/info with `L = 42` yields the published OKM
  `3cb25f25...`. `L = 42` forces a **partial final block**, exercising the `copy = min(...)`
  path in `hkdf_step`.

## Exercises

1. Explain the bug `hkdf_block` had, and why only the RFC vector caught it.
2. Trace `hkdf_expand` for `out_len = 42` and name the two blocks.
3. Show that `kex_x25519` is symmetric: both sides derive the same secret.
4. Explain why `hkdf_assemble` copies `tprev` into a local before HMAC.

## Reference

- `src/kex.c`, `include/kex.h`
- RFC 7748, RFC 5869
- `docs/21` (X25519/HKDF), `docs/appendix/B-aarch64-disasm.md`
