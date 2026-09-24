# Volume 18: AEAD Fundamentals

**nonces, tags, and the nonce-reuse catastrophe**

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

- What **AEAD** is and the two guarantees it gives
- The **nonce rule** and the nonce-reuse catastrophe
- The one API this repo uses for both backends
- How to test the tag actually rejects forged frames

## 1. What AEAD Is

**AEAD** = Authenticated Encryption with Associated Data. One construction gives:

- **Confidentiality** - the ciphertext hides the plaintext.
- **Integrity / authenticity** - a modified frame fails a 128-bit tag.

Confidentiality alone is not enough: without integrity, an attacker can flip bits and
the receiver accepts the forgery. AEAD closes that.

## 2. The Nonce Rule

> **Never reuse a nonce under one key.**

For GCM, nonce reuse is catastrophic: it reveals the authentication key and enables
forgery. This is the single most important operational rule in the course.

| nonce strategy | works for |
| -------------- | --------- |
| counter (never repeats) | GCM (96-bit) |
| random 192-bit | XChaCha20 (24-byte) |

The XChaCha20 192-bit nonce is large enough to choose **at random** per message with no
counter state - one fewer way to shoot yourself.

## 3. The One API

`src/aead.c` presents **one** interface with **two** backends:

```c
typedef enum aead_algo {
    AEAD_ALGO_AES_256_GCM = 0,
    AEAD_ALGO_XCHACHA20_POLY1305 = 1,
} aead_algo_t;

bool aead_init(aead_algo_t algo);
size_t aead_nonce_size(void);
aead_result_t aead_seal(const uint8_t key[32], const uint8_t *nonce,
                        const uint8_t plaintext[AEAD_PAYLOAD_SIZE],
                        uint8_t sealed_out[AEAD_SEALED_SIZE]);
aead_result_t aead_open(const uint8_t key[32], const uint8_t *nonce,
                        const uint8_t sealed[AEAD_SEALED_SIZE],
                        uint8_t plaintext_out[AEAD_PAYLOAD_SIZE]);
```

## 4. The Seal / Open Contract

- **Seal** writes ciphertext then a 16-byte tag.
- **Open** verifies the tag **first**; plaintext is released only if it verifies.

```c
/* the open path clears plaintext on failure */
if (r != AEAD_RESULT_SUCCESS) clear_bytes(plaintext_out, AEAD_PAYLOAD_SIZE);
```

## 5. The Nonce Guard

The API records the last nonce and flags a repeat in debug builds, so a mistake is
**loud** instead of silent:

```c
static bool nonce_reused(const uint8_t *nonce, size_t len) {
    bool seen = g_have_last_nonce && (memcmp(g_last_nonce, nonce, len) == 0);
    memcpy(g_last_nonce, nonce, len);
    g_have_last_nonce = true;
    return seen;
}
```

## 6. Test the Tag

The suite proves the tag works by forging:

```bash
$ python3 scripts/run_tests.py | grep -E 'aead'
:test_aead_seal_open_aes:PASS
:test_aead_open_aes_bad_tag:PASS
:test_aead_seal_open_xchacha:PASS
:test_aead_open_xchacha_bad_tag:PASS
```

`test_aead_open_*_bad_tag` flips one bit and expects
`AEAD_RESULT_AUTHENTICATION_FAILED`.

## Labs

1. Seal a payload and open it; confirm the plaintext matches.
2. Flip one ciphertext bit and confirm open fails.
3. Explain, in your own words, why AEAD needs a tag and a nonce.
4. Describe what goes wrong if you reuse a GCM nonce.

## Reference

- `src/aead.c`, `include/aead.h`
- `docs/19` (AES-GCM), `docs/20` (XChaCha20)
