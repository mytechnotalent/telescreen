# Walkthrough 11: `aead`

**every line of `src/aead.c`, explained**

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

`aead.c` is the largest, most important module: **one API, two AEADs**. It is the module
that turns the flawed beacon (`docs/17`) into a defensible channel. It is also a master
class in the house style: long cryptographic routines factored into functions of **at most
eight executable lines** each, with no blank lines in any body.

Read it in four layers:

1. **Globals and primitives** - state, clearing, word loads.
2. **The HChaCha20 core** - the XChaCha subkey derivation.
3. **The two backends** - AES-256-GCM and XChaCha20-Poly1305 through OpenSSL EVP.
4. **The public API** - `aead_init`, `aead_nonce_size`, `aead_seal`, `aead_open`.

## Layer 1: Globals and Primitives

### The globals

```c
static aead_algo_t g_aead_algo;                    // selected backend
static bool        g_aead_ready;                   // init done?
static uint8_t     g_last_nonce[24];               // last nonce seen
static bool        g_have_last_nonce;              // any nonce yet?
static const uint32_t HCHACHA_CONST[4] = {         // "expand 32-byte k"
    0x61707865u, 0x3320646Eu, 0x79622D32u, 0x6B206574u,
};
```

- `g_aead_algo` and `g_aead_ready` are separate because **zero is a valid algorithm**;
  a single enum would make "AES selected" and "not initialised" indistinguishable.
- `g_last_nonce` / `g_have_last_nonce` implement the reuse guard.
- `HCHACHA_CONST` is the four ChaCha constant words: `"expa" "nd 3" "2-by" "te k"`.
  In little-endian they are `0x61707865, 0x3320646E, 0x79622D32, 0x6B206574`.

### `clear_bytes` - wipe secrets

```c
static void clear_bytes(uint8_t *buf, size_t len) {
    size_t i;
    for (i = 0u; i < len; ++i) {
        buf[i] = 0u;
    }
}
```

Writes zero over a buffer. Used to wipe derived keys, subkeys, and plaintext-on-failure.
A byte loop (not `memset`) so the compiler cannot optimise it away as a dead store on a
buffer that then goes out of scope - a deliberate, common crypto habit.

### `rotl32` - rotate left

```c
static uint32_t rotl32(uint32_t value, uint8_t shift) {
    return (value << shift) | (value >> (32u - shift));
}
```

Shift left by `shift`, shift right by `32 - shift`, OR them. The rotation is the **R** in
the ARX (add-rotate-xor) family. ChaCha20 uses rotations of 16, 12, 8, and 7.

### `load32_le` / `store32_le`

```c
static uint32_t load32_le(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}
```

```c
static void store32_le(uint8_t *dst, uint32_t value) {
    dst[0] = (uint8_t)(value & 0xFFu);
    dst[1] = (uint8_t)((value >> 8u) & 0xFFu);
    dst[2] = (uint8_t)((value >> 16u) & 0xFFu);
    dst[3] = (uint8_t)((value >> 24u) & 0xFFu);
}
```

Portable, alignment-safe word/bytes conversions. The same pattern as `partition.c` and
`jffs2.c` - a deliberate house consistency.

## Layer 2: The HChaCha20 Core

### `quarter_round` - the ARX primitive

```c
static void quarter_round(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    *a += *b; *d ^= *a; *d = rotl32(*d, 16u);
    *c += *d; *b ^= *c; *b = rotl32(*b, 12u);
    *a += *b; *d ^= *a; *d = rotl32(*d, 8u);
    *c += *d; *b ^= *c; *b = rotl32(*b, 7u);
}
```

Four quarters, each: add, xor, rotate. Eight statements, four operations each. Because
there is **no data-dependent branch and no S-box**, this is constant-time by
construction - the reason ChaCha is the portable choice (`docs/20`).

### `hchacha_cols` / `hchacha_diag`

```c
static void hchacha_cols(uint32_t *s) {
    quarter_round(&s[0], &s[4], &s[8], &s[12]);
    quarter_round(&s[1], &s[5], &s[9], &s[13]);
    quarter_round(&s[2], &s[6], &s[10], &s[14]);
    quarter_round(&s[3], &s[7], &s[11], &s[15]);
}
```

```c
static void hchacha_diag(uint32_t *s) {
    quarter_round(&s[0], &s[5], &s[10], &s[15]);
    quarter_round(&s[1], &s[6], &s[11], &s[12]);
    quarter_round(&s[2], &s[7], &s[8], &s[13]);
    quarter_round(&s[3], &s[4], &s[9], &s[14]);
}
```

The state is a 4x4 matrix of words. **Columns** quarter-round down each column; **diagonals**
quarter-round along each diagonal. The pairing of columns-then-diagonals is what makes the
round diffuse.

### `hchacha_rounds` - ten double rounds

```c
static void hchacha_rounds(uint32_t *s) {
    uint8_t r;
    for (r = 0u; r < 10u; ++r) {
        hchacha_cols(s);
        hchacha_diag(s);
    }
}
```

Ten iterations, each doing columns then diagonals = **20 rounds**. That is ChaCha20's
count. The factorisation kept every function within the eight-line rule while keeping the
math exact.

### `hchacha_state` - fill the state

```c
static void hchacha_state(uint32_t *s, const uint8_t *key, const uint8_t *nonce) {
    uint8_t i;
    for (i = 0u; i < 4u; ++i) s[i] = HCHACHA_CONST[i];
    for (i = 0u; i < 8u; ++i) s[4u + i] = load32_le(&key[i * 4u]);
    for (i = 0u; i < 4u; ++i) s[12u + i] = load32_le(&nonce[i * 4u]);
}
```

The 16-word state is: **4 constants + 8 key words + 4 nonce words**. Note it uses the
**first 16 bytes** of the 24-byte nonce (4 words); the last 8 bytes are handled separately
(`build_inner_nonce`).

### `hchacha_store` - emit the subkey

```c
static void hchacha_store(uint8_t *subkey, const uint32_t *s) {
    store32_le(&subkey[0], s[0]);
    store32_le(&subkey[4], s[1]);
    store32_le(&subkey[8], s[2]);
    store32_le(&subkey[12], s[3]);
    store32_le(&subkey[16], s[12]);
    store32_le(&subkey[20], s[13]);
    store32_le(&subkey[24], s[14]);
    store32_le(&subkey[28], s[15]);
}
```

HChaCha20's output is a **specific eight words**: words 0-3 and 12-15 - the first row and
the last row of the matrix. **Not** the whole state. This is the detail people get wrong:
it is not "serialize all 16 words"; it is those eight.

### `hchacha20` - the composition

```c
static void hchacha20(const uint8_t key[32], const uint8_t nonce[24], uint8_t subkey[32]) {
    uint32_t state[16];
    hchacha_state(state, key, nonce);
    hchacha_rounds(state);
    hchacha_store(subkey, state);
}
```

Four lines that are a complete course in cryptography: **build the state, run the rounds,
emit the subkey.** Each step is a named function you just read.

## Layer 3a: AES-256-GCM (OpenSSL EVP)

### `build_inner_nonce`

```c
static void build_inner_nonce(const uint8_t nonce[24], uint8_t out[12]) {
    memset(out, 0, 4u);
    memcpy(&out[4], &nonce[16], 8u);
}
```

XChaCha converts the 24-byte outer nonce to the 12-byte IETF nonce: **four zero bytes**,
then the **last eight** nonce bytes. Four zero + eight = twelve.

### `nonce_reused` - the guard

```c
static bool nonce_reused(const uint8_t *nonce, size_t len) {
    bool seen = g_have_last_nonce && (memcmp(g_last_nonce, nonce, len) == 0);
    memcpy(g_last_nonce, nonce, len);
    g_have_last_nonce = true;
    return seen;
}
```

Detects a repeated nonce across calls. It records the nonce and reports whether the
**previous** one was equal. `aead_seal` calls it and discards the result in release; in
debug you break on a `true`. This turns a silent GCM catastrophe into a loud one.

> An earlier version clamped `len` to 24; under the 100% coverage rule that line was
> unreachable (the nonce is never longer than 24), so it was removed. The coverage gate
> keeps the code honest.

### `gcm_enc_init` / `gcm_enc_update` / `gcm_enc_final`

```c
static int gcm_enc_init(EVP_CIPHER_CTX *ctx, const uint8_t *key, const uint8_t *nonce) {
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) return 0;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)AEAD_AES_NONCE_SIZE, NULL) != 1) return 0;
    return EVP_EncryptInit_ex(ctx, NULL, NULL, key, nonce) == 1;
}
```

Three OpenSSL calls: choose the cipher, **set the IV length to 12**, then set the key and
nonce. Splitting them into one function keeps the sequence literal.

```c
static int gcm_enc_update(EVP_CIPHER_CTX *ctx, uint8_t *sealed, const uint8_t *plaintext) {
    int len = 0;
    return EVP_EncryptUpdate(ctx, sealed, &len, plaintext, (int)AEAD_PAYLOAD_SIZE) == 1;
}
```

Encrypt the payload. `len` is an output parameter OpenSSL writes; the ciphertext goes to
`sealed[0..]`.

```c
static int gcm_enc_final(EVP_CIPHER_CTX *ctx, uint8_t *sealed) {
    int len = 0;
    if (EVP_EncryptFinal_ex(ctx, &sealed[AEAD_PAYLOAD_SIZE], &len) != 1) return 0;
    return EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, (int)AEAD_TAG_SIZE,
                               &sealed[AEAD_PAYLOAD_SIZE]) == 1;
}
```

Finalise and **append the 16-byte tag** after the ciphertext. The layout is
`[ciphertext][tag]`, exactly `AEAD_SEALED_SIZE`.

### `gcm_dec_init` / `gcm_dec_final`

```c
static int gcm_dec_init(EVP_CIPHER_CTX *ctx, const uint8_t *key, const uint8_t *nonce) {
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) return 0;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)AEAD_AES_NONCE_SIZE, NULL) != 1) return 0;
    return EVP_DecryptInit_ex(ctx, NULL, NULL, key, nonce) == 1;
}
```

```c
static int gcm_dec_final(EVP_CIPHER_CTX *ctx, uint8_t *plaintext_out, const uint8_t *sealed) {
    int len = 0;
    int total = 0;
    if (EVP_DecryptUpdate(ctx, plaintext_out, &len, sealed, (int)AEAD_PAYLOAD_SIZE) != 1) return 0;
    total = len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, (int)AEAD_TAG_SIZE,
                            (void *)&sealed[AEAD_PAYLOAD_SIZE]) != 1) return 0;
    return EVP_DecryptFinal_ex(ctx, &plaintext_out[total], &len) == 1;
}
```

The critical order: **set the expected tag**, then call `EVP_DecryptFinal_ex`, which
**returns success only if the tag verifies**. If it returns 0, `aead_open` discards the
plaintext. Integrity before confidentiality.

### `seal_aes_gcm` / `open_aes_gcm`

```c
static aead_result_t seal_aes_gcm(const uint8_t key[32],
                                  const uint8_t nonce[AEAD_AES_NONCE_SIZE],
                                  const uint8_t plaintext[AEAD_PAYLOAD_SIZE],
                                  uint8_t sealed_out[AEAD_SEALED_SIZE]) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    aead_result_t r = AEAD_RESULT_INTERNAL_ERROR;
    if (ctx == NULL) return AEAD_RESULT_INTERNAL_ERROR;
    if (gcm_enc_init(ctx, key, nonce) && gcm_enc_update(ctx, sealed_out, plaintext)) {
        if (gcm_enc_final(ctx, sealed_out)) r = AEAD_RESULT_SUCCESS;
    }
    EVP_CIPHER_CTX_free(ctx);
    return r;
}
```

The composition: get a context, initialise, update, finalise, free. Failure at any step
leaves the result as an error. `open_aes_gcm` is the mirror and **clears the plaintext on
failure**:

```c
if (r != AEAD_RESULT_SUCCESS) clear_bytes(plaintext_out, AEAD_PAYLOAD_SIZE);
```

## Layer 3b: XChaCha20-Poly1305

The `chacha_enc_*` / `chacha_dec_*` helpers are the same shape as the `gcm_*` ones, only
with `EVP_chacha20_poly1305()` and `EVP_CTRL_AEAD_*`. Then:

### `xchacha_wipe`, `xchacha_encrypt`, `xchacha_decrypt`

```c
static void xchacha_wipe(uint8_t *subkey, uint8_t *inner) {
    clear_bytes(subkey, 32u);
    clear_bytes(inner, 12u);
}
```

```c
static aead_result_t xchacha_decrypt(const uint8_t *subkey, const uint8_t *inner,
                                     const uint8_t *sealed, uint8_t *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    aead_result_t r = AEAD_RESULT_AUTHENTICATION_FAILED;
    if (ctx == NULL) return AEAD_RESULT_INTERNAL_ERROR;
    if (chacha_dec_init(ctx, subkey, inner) &&
        chacha_dec_final(ctx, plaintext, sealed)) r = AEAD_RESULT_SUCCESS;
    EVP_CIPHER_CTX_free(ctx);
    if (r != AEAD_RESULT_SUCCESS) clear_bytes(plaintext, AEAD_PAYLOAD_SIZE);
    return r;
}
```

Note this function also clears plaintext on failure, so `open_xchacha` stays short.

### `seal_xchacha` / `open_xchacha`

```c
static aead_result_t seal_xchacha(const uint8_t key[32],
                                  const uint8_t nonce[AEAD_XCHACHA_NONCE_SIZE],
                                  const uint8_t plaintext[AEAD_PAYLOAD_SIZE],
                                  uint8_t sealed_out[AEAD_SEALED_SIZE]) {
    uint8_t subkey[32];
    uint8_t inner[12];
    aead_result_t r;
    hchacha20(key, nonce, subkey);
    build_inner_nonce(nonce, inner);
    r = xchacha_encrypt(subkey, inner, plaintext, sealed_out);
    xchacha_wipe(subkey, inner);
    return r;
}
```

The XChaCha flow: **derive the subkey** from key + first 16 nonce bytes, **build the inner
nonce** from the last 8, seal with the IETF construction, then **wipe** the subkey and
inner nonce. `open_xchacha` is the mirror.

## Layer 4: The Public API

### `aead_init`

```c
bool aead_init(aead_algo_t algo) {
    if ((algo != AEAD_ALGO_AES_256_GCM) && (algo != AEAD_ALGO_XCHACHA20_POLY1305)) {
        return false;
    }
    g_aead_algo = algo;
    g_have_last_nonce = false;
    memset(g_last_nonce, 0, sizeof(g_last_nonce));
    g_aead_ready = true;
    return true;
}
```

Rejects an unknown backend, records the choice, resets the nonce guard, marks ready.

### `aead_nonce_size`

```c
size_t aead_nonce_size(void) {
    if (!g_aead_ready) return 0u;
    if (g_aead_algo == AEAD_ALGO_AES_256_GCM) return AEAD_AES_NONCE_SIZE;
    return AEAD_XCHACHA_NONCE_SIZE;
}
```

Returns 12 for AES-GCM, 24 for XChaCha, 0 before init. The caller uses this to size the
nonce buffer - so the backend choice propagates.

### `aead_seal`

```c
aead_result_t aead_seal(const uint8_t key[32], const uint8_t *nonce,
                        const uint8_t plaintext[AEAD_PAYLOAD_SIZE],
                        uint8_t sealed_out[AEAD_SEALED_SIZE]) {
    if (!g_aead_ready || (key == NULL) || (nonce == NULL) ||
        (plaintext == NULL) || (sealed_out == NULL)) {
        return AEAD_RESULT_BAD_ARGUMENT;
    }
    (void)nonce_reused(nonce, aead_nonce_size());
    if (g_aead_algo == AEAD_ALGO_AES_256_GCM) {
        return seal_aes_gcm(key, nonce, plaintext, sealed_out);
    }
    return seal_xchacha(key, nonce, plaintext, sealed_out);
}
```

Guard, run the nonce guard, dispatch to the backend. `(void)` marks the nonce-reuse result
as intentionally unused in release.

### `aead_open`

The mirror of `aead_seal`: guard, dispatch to `open_aes_gcm` or `open_xchacha`. No nonce
guard (open does not create new ciphertext).

## The Lesson of the Factoring

Every function in this file is **at most eight executable lines**. A 634-line cryptographic
module was written as a tree of tiny, named, individually-testable steps. That is not a
stylistic flourish - it is why the module reaches **100% line coverage** and why each
function can be read against its assembly in the appendix.

## Tests

```bash
$ python3 scripts/run_tests.py | grep -E 'aead'
:test_aead_nonce_size_before_init:PASS
:test_aead_init_rejects_bad_algo:PASS
:test_aead_init_aes:PASS
:test_aead_seal_bad_args:PASS
:test_aead_open_bad_args:PASS
:test_aead_seal_open_aes:PASS
:test_aead_open_aes_bad_tag:PASS
:test_aead_nonce_reuse:PASS
:test_aead_init_xchacha:PASS
:test_aead_seal_open_xchacha:PASS
:test_aead_open_xchacha_bad_tag:PASS
```

Eleven cases cover both backends, both round trips, both tamper cases, the argument
guards, the not-ready path, and the reuse guard. That is the whole module.

## Exercises

1. Match each function in this walkthrough to its block in `docs/appendix/B-aarch64-disasm.md`.
2. Trace `hchacha_store` and explain why words 4-11 are omitted.
3. Explain the ordering in `gcm_dec_final` (set tag, then final).
4. Show that `seal_xchacha` wipes the subkey before returning.
5. Add a third backend stub and describe what must change in `aead_seal`/`aead_open`.

## Reference

- `src/aead.c`, `include/aead.h`
- `docs/18`-`docs/20`, `docs/appendix/B-aarch64-disasm.md`
