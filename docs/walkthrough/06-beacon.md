# Walkthrough 06: `beacon`

**every line of `src/beacon.c`, explained**

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

`beacon.c` is the exfiltration channel - and the CTF's crypto lesson (`docs/17`). It
contains **both** the deliberately broken key schedule and the hardened seal, so you can
put them side by side.

```
beacon_weak_key (public)   derive the key from the public UID   [THE FLAW]
beacon_nonce    (public)   build a nonce from the sequence
beacon_seal     (public)   seal with the hardened AEAD API
```

## 1. The Weak Key - the Flaw, on Purpose

```c
void beacon_weak_key(const char *uid, uint8_t out[BEACON_KEY_SIZE]) {
    size_t ulen = strlen(uid);
    uint32_t seed = crc32_le(0u, (const uint8_t *)uid, ulen);
    size_t i;
    for (i = 0u; i < BEACON_KEY_SIZE; ++i) {
        seed = crc32_le(seed, (const uint8_t *)uid, ulen);
        out[i] = (uint8_t)(seed & 0xFFu);
    }
}
```

| line | what it does |
| ---- | ------------ |
| `ulen = strlen(uid)` | the public UID length |
| `seed = crc32_le(0, uid, ulen)` | an initial fold of the UID |
| loop 0..31 | produce 32 key bytes |
| `seed = crc32_le(seed, uid, ulen)` | re-fold the UID, chaining the seed |
| `out[i] = seed & 0xFF` | take the low byte |

Read the security property carefully: **the output depends only on `uid`**. The UID is
public. Therefore the key is public. `beacon_weak_key` is **obfuscation**, not key
management. It is here to be *replaced*.

> This is the same failure the camera teardown found in a real device's PPPP stack: a key
> derived from a public device identifier. The lesson is identical and portable.

## 2. The Nonce

```c
void beacon_nonce(uint32_t seq, uint8_t *out) {
    out[0] = (uint8_t)(seq & 0xFFu);
    out[1] = (uint8_t)((seq >> 8u) & 0xFFu);
    out[2] = (uint8_t)((seq >> 16u) & 0xFFu);
    out[3] = (uint8_t)((seq >> 24u) & 0xFFu);
    memset(&out[4], 0, 20u);
}
```

| line | what it does |
| ---- | ------------ |
| lines 1-4 | write the 32-bit sequence little-endian into bytes 0..3 |
| `memset(&out[4], 0, 20)` | zero bytes 4..23 |

So the first four nonce bytes are the sequence, and the rest are zero. With a **monotonic
sequence**, the nonce never repeats under one key - which is exactly the rule
(`docs/18`). The fixed 20-byte zero region is fine because the sequence already
guarantees uniqueness.

> A production design might randomise the trailing bytes too. Here the sequence is the
> unique part and the zero pad makes the nonce size match the AEAD API.

## 3. `beacon_seal` - the hardened path

```c
bool beacon_seal(const uint8_t key[BEACON_KEY_SIZE], uint32_t seq,
                 const uint8_t *plaintext, uint8_t *sealed_out) {
    uint8_t nonce[AEAD_XCHACHA_NONCE_SIZE];
    beacon_nonce(seq, nonce);
    return aead_seal(key, nonce, plaintext, sealed_out) == AEAD_RESULT_SUCCESS;
}
```

| line | what it does |
| ---- | ------------ |
| `uint8_t nonce[24]` | a 24-byte XChaCha nonce buffer |
| `beacon_nonce(seq, nonce)` | fill it from the sequence |
| `aead_seal(...) == SUCCESS` | seal with the AEAD API; true only on success |

`beacon_seal` does **not** pick the key - the caller supplies it. That is the whole point:
the key source is swappable. Feed it `beacon_weak_key(...)` and you have the flaw; feed it
an HKDF-derived session key (`docs/21`) and you have the fix. The seal is identical.

## 4. Put Together: Flaw vs Fix

```c
/* the flaw: a public key */
uint8_t key[32];
beacon_weak_key(uid, key);
beacon_seal(key, seq, payload, out);      /* anyone can decrypt */

/* the fix: a secret key */
uint8_t key[32];
kex_hkdf(session_secret, ..., key);       /* docs/21 */
beacon_seal(key, seq, payload, out);      /* only the peer can decrypt */
```

The **only** difference is where `key` comes from. Teach that.

## 5. Tests

```bash
$ python3 scripts/run_tests.py | grep beacon
:test_beacon_weak_key:PASS
:test_beacon_weak_key_differs:PASS
:test_beacon_nonce:PASS
:test_beacon_seal:PASS
```

- `weak_key` shows the same UID yields the same key (**deterministic**).
- `weak_key_differs` shows different UIDs yield different keys - but both are still public.
- `nonce` checks the sequence lands little-endian and byte 4 is zero.
- `seal` shows the hardened seal succeeds with an AEAD key.

## 6. Recover the Key (the CTF task)

`scripts/weak_decrypt.py` reimplements `beacon_weak_key` in Python:

```bash
$ python3 scripts/weak_decrypt.py --uid SSAT-468547-FEEBD
da506e04af00c6f40394d2cd2295bfc8682e8b9f9e9b844cea50c08d5f483141
```

Same algorithm, same output: proof the key is public.

## Exercises

1. Derive the key for two UIDs; show they differ but are both deterministic.
2. Explain, in one paragraph, why "it uses an AEAD" does not save a public key.
3. Replace `beacon_weak_key` with an HKDF key and show `beacon_seal` still passes.
4. Show a nonce for `seq=1` and `seq=2`; confirm they differ.

## Reference

- `src/beacon.c`, `include/beacon.h`, `src/aead.c`, `src/kex.c`
- `docs/17` (weak KDF), `docs/21` (X25519/HKDF)
