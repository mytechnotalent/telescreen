# Module 06: aead

**purpose, full source, AArch64 disassembly, and tests**

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

## 1. Purpose

See `README.md` for where `aead` sits in the lab. This volume is the whole module.

## 2. Header

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent
// File:    aead.h
// Desc:    Declares the TELESCREEN authenticated-encryption API. One interface,
//          two backends: AES-256-GCM (ARMv8 crypto extensions) and
//          XChaCha20-Poly1305 (portable, 192-bit nonce). The hardened beacon uses
//          this API to seal and open telemetry so a forged frame dies at the tag.
// Created: 2026

#ifndef AEAD_H
#define AEAD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief AEAD backend selector.
 *
 * AES-256-GCM is the hardware-accelerated default on the RP5 (BCM2712 Cortex-A76
 * with the ARMv8 Cryptography Extensions). XChaCha20-Poly1305 is the portable
 * twin used when no AES engine is present or when a 192-bit random nonce is
 * preferred.
 */
typedef enum aead_algo {
    AEAD_ALGO_AES_256_GCM = 0,
    AEAD_ALGO_XCHACHA20_POLY1305 = 1,
} aead_algo_t;

/**
 * @brief AES-256-GCM key size in bytes.
 */
#define AEAD_AES_KEY_SIZE 32u

/**
 * @brief AES-GCM recommended nonce size in bytes.
 *
 * A 96-bit nonce is the IETF-recommended size for GCM and is used directly by
 * the counter block. It MUST never repeat under a given key.
 */
#define AEAD_AES_NONCE_SIZE 12u

/**
 * @brief XChaCha20-Poly1305 key size in bytes.
 */
#define AEAD_XCHACHA_KEY_SIZE 32u

/**
 * @brief XChaCha20-Poly1305 nonce size in bytes.
 *
 * The extended 192-bit nonce is large enough to be chosen at random per message
 * without a counter, which removes an entire class of misuse.
 */
#define AEAD_XCHACHA_NONCE_SIZE 24u

/**
 * @brief Authentication tag size in bytes for both backends.
 *
 * GCM and Poly1305 both emit a 128-bit tag. A forged frame fails verification
 * here before any plaintext is released.
 */
#define AEAD_TAG_SIZE 16u

/**
 * @brief Hardened beacon payload size in bytes.
 *
 * The fixed telemetry payload is 48 bytes: a sequence number, a device-state
 * byte, and trailing reserved bytes matching the captured-device layout.
 */
#define AEAD_PAYLOAD_SIZE 48u

/**
 * @brief Full ciphertext-plus-tag size in bytes.
 */
#define AEAD_SEALED_SIZE (AEAD_PAYLOAD_SIZE + AEAD_TAG_SIZE)

/**
 * @brief AEAD result codes.
 *
 * These values let the caller distinguish a bad argument from a cryptographic
 * authentication failure without guessing.
 */
typedef enum aead_result {
    AEAD_RESULT_SUCCESS = 0,
    AEAD_RESULT_BAD_ARGUMENT = 1,
    AEAD_RESULT_AUTHENTICATION_FAILED = 2,
    AEAD_RESULT_INTERNAL_ERROR = 3,
} aead_result_t;

/**
 * @brief Initialize the AEAD subsystem.
 *
 * Selects the backend, verifies that a self-test vector passes, and prepares the
 * module for sealing and opening. The self-test also confirms that nonce reuse
 * detection is armed in debug builds.
 *
 * @param algo Backend selector.
 * @return bool true when initialization succeeds, else false.
 */
bool aead_init(aead_algo_t algo);

/**
 * @brief Return the nonce size required by the active backend.
 *
 * @param None.
 * @return size_t Nonce size in bytes, or 0 when not initialized.
 */
size_t aead_nonce_size(void);

/**
 * @brief Seal a plaintext payload into ciphertext plus tag.
 *
 * Encrypts the fixed-size payload under the supplied key and nonce and appends a
 * 128-bit authentication tag. The nonce MUST be unique for every call under the
 * same key.
 *
 * @param key Pointer to 32-byte AEAD key.
 * @param nonce Pointer to backend nonce bytes.
 * @param plaintext Pointer to AEAD_PAYLOAD_SIZE plaintext bytes.
 * @param sealed_out Output AEAD_SEALED_SIZE buffer (ciphertext then tag).
 * @return aead_result_t Detailed sealing outcome.
 */
aead_result_t aead_seal(const uint8_t key[32],
                        const uint8_t *nonce,
                        const uint8_t plaintext[AEAD_PAYLOAD_SIZE],
                        uint8_t sealed_out[AEAD_SEALED_SIZE]);

/**
 * @brief Open a sealed payload, verifying the tag before releasing plaintext.
 *
 * Decrypts the ciphertext and verifies the 128-bit tag. Plaintext is written to
 * the caller buffer only when the tag verifies.
 *
 * @param key Pointer to 32-byte AEAD key.
 * @param nonce Pointer to backend nonce bytes.
 * @param sealed Pointer to AEAD_SEALED_SIZE sealed bytes.
 * @param plaintext_out Output AEAD_PAYLOAD_SIZE plaintext buffer.
 * @return aead_result_t Detailed opening outcome.
 */
aead_result_t aead_open(const uint8_t key[32],
                        const uint8_t *nonce,
                        const uint8_t sealed[AEAD_SEALED_SIZE],
                        uint8_t plaintext_out[AEAD_PAYLOAD_SIZE]);

#endif // AEAD_H
```

## 3. Source

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    aead.c
// Desc:    Implements the TELESCREEN authenticated-encryption API. AES-256-GCM
//          runs through the ARMv8 crypto extensions via OpenSSL EVP on the RP5.
//          XChaCha20-Poly1305 derives the subkey with a software HChaCha20 core
//          and seals with the IETF ChaCha20-Poly1305 construction.
// Created: 2026

#include "aead.h"
#include <string.h>
#include <openssl/evp.h>

/**
 * @brief Active backend selected by aead_init().
 */
static aead_algo_t g_aead_algo;

/**
 * @brief Non-zero once aead_init() has completed.
 */
static bool g_aead_ready;

/**
 * @brief Last nonce observed under the active key.
 */
static uint8_t g_last_nonce[AEAD_XCHACHA_NONCE_SIZE];

/**
 * @brief Non-zero once a nonce has been recorded.
 */
static bool g_have_last_nonce;

/**
 * @brief HChaCha20 constant words at the start of the state.
 */
static const uint32_t HCHACHA_CONST[4] = {
    0x61707865u, 0x3320646Eu, 0x79622D32u, 0x6B206574u,
};

/**
 * @brief Clear a byte buffer.
 *
 * @param buf Pointer to mutable byte buffer.
 * @param len Number of bytes to clear.
 * @return void
 */
static void clear_bytes(uint8_t *buf, size_t len) {
    size_t i;
    for (i = 0u; i < len; ++i) {
        buf[i] = 0u;
    }
}

/**
 * @brief Rotate a 32-bit value left.
 *
 * @param value Input 32-bit word.
 * @param shift Rotation distance in bits.
 * @return uint32_t Rotated result.
 */
static uint32_t rotl32(uint32_t value, uint8_t shift) {
    return (value << shift) | (value >> (32u - shift));
}

/**
 * @brief Load a 32-bit little-endian word from bytes.
 *
 * @param src Pointer to four readable bytes.
 * @return uint32_t Parsed 32-bit word.
 */
static uint32_t load32_le(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}

/**
 * @brief Store a 32-bit word in little-endian byte order.
 *
 * @param dst Pointer to four writable bytes.
 * @param value 32-bit word to serialize.
 * @return void
 */
static void store32_le(uint8_t *dst, uint32_t value) {
    dst[0] = (uint8_t)(value & 0xFFu);
    dst[1] = (uint8_t)((value >> 8u) & 0xFFu);
    dst[2] = (uint8_t)((value >> 16u) & 0xFFu);
    dst[3] = (uint8_t)((value >> 24u) & 0xFFu);
}

/**
 * @brief Execute one ChaCha quarter-round.
 *
 * @param a Pointer to state word a.
 * @param b Pointer to state word b.
 * @param c Pointer to state word c.
 * @param d Pointer to state word d.
 * @return void
 */
static void quarter_round(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    *a += *b; *d ^= *a; *d = rotl32(*d, 16u);
    *c += *d; *b ^= *c; *b = rotl32(*b, 12u);
    *a += *b; *d ^= *a; *d = rotl32(*d, 8u);
    *c += *d; *b ^= *c; *b = rotl32(*b, 7u);
}

/**
 * @brief Run the four column quarter-rounds of one HChaCha20 round.
 *
 * @param s Pointer to the 16-word state.
 * @return void
 */
static void hchacha_cols(uint32_t *s) {
    quarter_round(&s[0], &s[4], &s[8], &s[12]);
    quarter_round(&s[1], &s[5], &s[9], &s[13]);
    quarter_round(&s[2], &s[6], &s[10], &s[14]);
    quarter_round(&s[3], &s[7], &s[11], &s[15]);
}

/**
 * @brief Run the four diagonal quarter-rounds of one HChaCha20 round.
 *
 * @param s Pointer to the 16-word state.
 * @return void
 */
static void hchacha_diag(uint32_t *s) {
    quarter_round(&s[0], &s[5], &s[10], &s[15]);
    quarter_round(&s[1], &s[6], &s[11], &s[12]);
    quarter_round(&s[2], &s[7], &s[8], &s[13]);
    quarter_round(&s[3], &s[4], &s[9], &s[14]);
}

/**
 * @brief Run ten HChaCha20 double rounds over the state.
 *
 * @param s Pointer to the 16-word state.
 * @return void
 */
static void hchacha_rounds(uint32_t *s) {
    uint8_t r;
    for (r = 0u; r < 10u; ++r) {
        hchacha_cols(s);
        hchacha_diag(s);
    }
}

/**
 * @brief Build the initial HChaCha20 state from key and nonce.
 *
 * @param s Pointer to the 16-word state.
 * @param key Pointer to 32-byte key.
 * @param nonce Pointer to 24-byte nonce.
 * @return void
 */
static void hchacha_state(uint32_t *s, const uint8_t *key, const uint8_t *nonce) {
    uint8_t i;
    for (i = 0u; i < 4u; ++i) s[i] = HCHACHA_CONST[i];
    for (i = 0u; i < 8u; ++i) s[4u + i] = load32_le(&key[i * 4u]);
    for (i = 0u; i < 4u; ++i) s[12u + i] = load32_le(&nonce[i * 4u]);
}

/**
 * @brief Serialize the HChaCha20 subkey words into bytes.
 *
 * @param subkey Output 32-byte subkey buffer.
 * @param s Pointer to the 16-word state.
 * @return void
 */
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

/**
 * @brief Derive a 256-bit XChaCha20 subkey from key and nonce prefix.
 *
 * @param key Pointer to 32-byte AEAD key.
 * @param nonce Pointer to 24-byte XChaCha20 nonce.
 * @param subkey Output 32-byte subkey buffer.
 * @return void
 */
static void hchacha20(const uint8_t key[32], const uint8_t nonce[24],
                      uint8_t subkey[32]) {
    uint32_t state[16];
    hchacha_state(state, key, nonce);
    hchacha_rounds(state);
    hchacha_store(subkey, state);
}

/**
 * @brief Build the inner 96-bit nonce used by ChaCha20-Poly1305.
 *
 * @param nonce Pointer to 24-byte XChaCha20 nonce.
 * @param out Output 12-byte nonce buffer.
 * @return void
 */
static void build_inner_nonce(const uint8_t nonce[24], uint8_t out[12]) {
    memset(out, 0, 4u);
    memcpy(&out[4], &nonce[16], 8u);
}

/**
 * @brief Record the nonce and flag accidental reuse.
 *
 * @param nonce Pointer to backend nonce bytes.
 * @param len Nonce size in bytes.
 * @return bool true when the nonce was already seen, else false.
 */
static bool nonce_reused(const uint8_t *nonce, size_t len) {
    bool seen = g_have_last_nonce && (memcmp(g_last_nonce, nonce, len) == 0);
    memcpy(g_last_nonce, nonce, len);
    g_have_last_nonce = true;
    return seen;
}

/**
 * @brief Initialize an EVP context for AES-256-GCM encryption.
 *
 * @param ctx Pointer to the EVP context.
 * @param key Pointer to 32-byte AES key.
 * @param nonce Pointer to 12-byte GCM nonce.
 * @return int Non-zero when initialization succeeds.
 */
static int gcm_enc_init(EVP_CIPHER_CTX *ctx, const uint8_t *key,
                        const uint8_t *nonce) {
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) return 0;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN,
                            (int)AEAD_AES_NONCE_SIZE, NULL) != 1) return 0;
    return EVP_EncryptInit_ex(ctx, NULL, NULL, key, nonce) == 1;
}

/**
 * @brief Encrypt the payload into the sealed buffer with GCM.
 *
 * @param ctx Pointer to the EVP context.
 * @param sealed Pointer to the sealed output buffer.
 * @param plaintext Pointer to plaintext bytes.
 * @return int Non-zero when the update succeeds.
 */
static int gcm_enc_update(EVP_CIPHER_CTX *ctx, uint8_t *sealed,
                          const uint8_t *plaintext) {
    int len = 0;
    return EVP_EncryptUpdate(ctx, sealed, &len, plaintext,
                             (int)AEAD_PAYLOAD_SIZE) == 1;
}

/**
 * @brief Finalize GCM encryption and append the authentication tag.
 *
 * @param ctx Pointer to the EVP context.
 * @param sealed Pointer to the sealed output buffer.
 * @return int Non-zero when finalization and tag retrieval succeed.
 */
static int gcm_enc_final(EVP_CIPHER_CTX *ctx, uint8_t *sealed) {
    int len = 0;
    if (EVP_EncryptFinal_ex(ctx, &sealed[AEAD_PAYLOAD_SIZE], &len) != 1) return 0;
    return EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, (int)AEAD_TAG_SIZE,
                               &sealed[AEAD_PAYLOAD_SIZE]) == 1;
}

/**
 * @brief Initialize an EVP context for AES-256-GCM decryption.
 *
 * @param ctx Pointer to the EVP context.
 * @param key Pointer to 32-byte AES key.
 * @param nonce Pointer to 12-byte GCM nonce.
 * @return int Non-zero when initialization succeeds.
 */
static int gcm_dec_init(EVP_CIPHER_CTX *ctx, const uint8_t *key,
                        const uint8_t *nonce) {
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) return 0;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN,
                            (int)AEAD_AES_NONCE_SIZE, NULL) != 1) return 0;
    return EVP_DecryptInit_ex(ctx, NULL, NULL, key, nonce) == 1;
}

/**
 * @brief Decrypt the payload and verify the GCM tag.
 *
 * @param ctx Pointer to the EVP context.
 * @param plaintext_out Pointer to the plaintext output buffer.
 * @param sealed Pointer to the sealed input buffer.
 * @return int Non-zero when decryption and tag verification succeed.
 */
static int gcm_dec_final(EVP_CIPHER_CTX *ctx, uint8_t *plaintext_out,
                         const uint8_t *sealed) {
    int len = 0;
    int total = 0;
    if (EVP_DecryptUpdate(ctx, plaintext_out, &len, sealed,
                          (int)AEAD_PAYLOAD_SIZE) != 1) return 0;
    total = len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, (int)AEAD_TAG_SIZE,
                            (void *)&sealed[AEAD_PAYLOAD_SIZE]) != 1) return 0;
    return EVP_DecryptFinal_ex(ctx, &plaintext_out[total], &len) == 1;
}

/**
 * @brief Seal with AES-256-GCM through OpenSSL EVP.
 *
 * @param key Pointer to 32-byte AES key.
 * @param nonce Pointer to 12-byte GCM nonce.
 * @param plaintext Pointer to plaintext bytes.
 * @param sealed_out Output ciphertext then 16-byte tag.
 * @return aead_result_t Detailed outcome.
 */
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

/**
 * @brief Open with AES-256-GCM through OpenSSL EVP.
 *
 * @param key Pointer to 32-byte AES key.
 * @param nonce Pointer to 12-byte GCM nonce.
 * @param sealed Pointer to ciphertext then tag.
 * @param plaintext_out Output plaintext buffer.
 * @return aead_result_t Detailed outcome.
 */
static aead_result_t open_aes_gcm(const uint8_t key[32],
                                  const uint8_t nonce[AEAD_AES_NONCE_SIZE],
                                  const uint8_t sealed[AEAD_SEALED_SIZE],
                                  uint8_t plaintext_out[AEAD_PAYLOAD_SIZE]) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    aead_result_t r = AEAD_RESULT_AUTHENTICATION_FAILED;
    if (ctx == NULL) return AEAD_RESULT_INTERNAL_ERROR;
    if (gcm_dec_init(ctx, key, nonce) && gcm_dec_final(ctx, plaintext_out, sealed)) {
        r = AEAD_RESULT_SUCCESS;
    }
    EVP_CIPHER_CTX_free(ctx);
    if (r != AEAD_RESULT_SUCCESS) clear_bytes(plaintext_out, AEAD_PAYLOAD_SIZE);
    return r;
}

/**
 * @brief Initialize an EVP context for ChaCha20-Poly1305 encryption.
 *
 * @param ctx Pointer to the EVP context.
 * @param subkey Pointer to 32-byte subkey.
 * @param inner Pointer to 12-byte inner nonce.
 * @return int Non-zero when initialization succeeds.
 */
static int chacha_enc_init(EVP_CIPHER_CTX *ctx, const uint8_t *subkey,
                           const uint8_t *inner) {
    if (EVP_EncryptInit_ex(ctx, EVP_chacha20_poly1305(), NULL, NULL, NULL) != 1) return 0;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 12, NULL) != 1) return 0;
    return EVP_EncryptInit_ex(ctx, NULL, NULL, subkey, inner) == 1;
}

/**
 * @brief Encrypt the payload into the sealed buffer with ChaCha20-Poly1305.
 *
 * @param ctx Pointer to the EVP context.
 * @param sealed Pointer to the sealed output buffer.
 * @param plaintext Pointer to plaintext bytes.
 * @return int Non-zero when the update succeeds.
 */
static int chacha_enc_update(EVP_CIPHER_CTX *ctx, uint8_t *sealed,
                             const uint8_t *plaintext) {
    int len = 0;
    return EVP_EncryptUpdate(ctx, sealed, &len, plaintext,
                             (int)AEAD_PAYLOAD_SIZE) == 1;
}

/**
 * @brief Finalize ChaCha20-Poly1305 encryption and append the tag.
 *
 * @param ctx Pointer to the EVP context.
 * @param sealed Pointer to the sealed output buffer.
 * @return int Non-zero when finalization and tag retrieval succeed.
 */
static int chacha_enc_final(EVP_CIPHER_CTX *ctx, uint8_t *sealed) {
    int len = 0;
    if (EVP_EncryptFinal_ex(ctx, &sealed[AEAD_PAYLOAD_SIZE], &len) != 1) return 0;
    return EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, (int)AEAD_TAG_SIZE,
                               &sealed[AEAD_PAYLOAD_SIZE]) == 1;
}

/**
 * @brief Initialize an EVP context for ChaCha20-Poly1305 decryption.
 *
 * @param ctx Pointer to the EVP context.
 * @param subkey Pointer to 32-byte subkey.
 * @param inner Pointer to 12-byte inner nonce.
 * @return int Non-zero when initialization succeeds.
 */
static int chacha_dec_init(EVP_CIPHER_CTX *ctx, const uint8_t *subkey,
                           const uint8_t *inner) {
    if (EVP_DecryptInit_ex(ctx, EVP_chacha20_poly1305(), NULL, NULL, NULL) != 1) return 0;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 12, NULL) != 1) return 0;
    return EVP_DecryptInit_ex(ctx, NULL, NULL, subkey, inner) == 1;
}

/**
 * @brief Decrypt the payload and verify the Poly1305 tag.
 *
 * @param ctx Pointer to the EVP context.
 * @param plaintext_out Pointer to the plaintext output buffer.
 * @param sealed Pointer to the sealed input buffer.
 * @return int Non-zero when decryption and tag verification succeed.
 */
static int chacha_dec_final(EVP_CIPHER_CTX *ctx, uint8_t *plaintext_out,
                            const uint8_t *sealed) {
    int len = 0;
    int total = 0;
    if (EVP_DecryptUpdate(ctx, plaintext_out, &len, sealed,
                          (int)AEAD_PAYLOAD_SIZE) != 1) return 0;
    total = len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, (int)AEAD_TAG_SIZE,
                            (void *)&sealed[AEAD_PAYLOAD_SIZE]) != 1) return 0;
    return EVP_DecryptFinal_ex(ctx, &plaintext_out[total], &len) == 1;
}

/**
 * @brief Seal with XChaCha20-Poly1305.
 *
 * @param key Pointer to 32-byte ChaCha key.
 * @param nonce Pointer to 24-byte XChaCha nonce.
 * @param plaintext Pointer to plaintext bytes.
 * @param sealed_out Output ciphertext then 16-byte tag.
 * @return aead_result_t Detailed outcome.
 */
/**
 * @brief Wipe the derived subkey and inner nonce.
 *
 * @param subkey Pointer to 32-byte subkey buffer.
 * @param inner Pointer to 12-byte inner nonce buffer.
 * @return void
 */
static void xchacha_wipe(uint8_t *subkey, uint8_t *inner) {
    clear_bytes(subkey, 32u);
    clear_bytes(inner, 12u);
}

/**
 * @brief Encrypt one payload with the derived subkey and inner nonce.
 *
 * @param subkey Pointer to 32-byte subkey.
 * @param inner Pointer to 12-byte inner nonce.
 * @param plaintext Pointer to plaintext bytes.
 * @param sealed Pointer to the sealed output buffer.
 * @return aead_result_t Detailed outcome.
 */
static aead_result_t xchacha_encrypt(const uint8_t *subkey, const uint8_t *inner,
                                     const uint8_t *plaintext, uint8_t *sealed) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    aead_result_t r = AEAD_RESULT_INTERNAL_ERROR;
    if (ctx == NULL) return AEAD_RESULT_INTERNAL_ERROR;
    if (chacha_enc_init(ctx, subkey, inner) &&
        chacha_enc_update(ctx, sealed, plaintext)) {
        if (chacha_enc_final(ctx, sealed)) r = AEAD_RESULT_SUCCESS;
    }
    EVP_CIPHER_CTX_free(ctx);
    return r;
}

/**
 * @brief Decrypt one payload with the derived subkey and inner nonce.
 *
 * @param subkey Pointer to 32-byte subkey.
 * @param inner Pointer to 12-byte inner nonce.
 * @param sealed Pointer to the sealed input buffer.
 * @param plaintext Pointer to the plaintext output buffer.
 * @return aead_result_t Detailed outcome.
 */
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

/**
 * @brief Seal with XChaCha20-Poly1305.
 *
 * @param key Pointer to 32-byte ChaCha key.
 * @param nonce Pointer to 24-byte XChaCha nonce.
 * @param plaintext Pointer to plaintext bytes.
 * @param sealed_out Output ciphertext then 16-byte tag.
 * @return aead_result_t Detailed outcome.
 */
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

/**
 * @brief Open with XChaCha20-Poly1305.
 *
 * @param key Pointer to 32-byte ChaCha key.
 * @param nonce Pointer to 24-byte XChaCha nonce.
 * @param sealed Pointer to ciphertext then tag.
 * @param plaintext_out Output plaintext buffer.
 * @return aead_result_t Detailed outcome.
 */
static aead_result_t open_xchacha(const uint8_t key[32],
                                  const uint8_t nonce[AEAD_XCHACHA_NONCE_SIZE],
                                  const uint8_t sealed[AEAD_SEALED_SIZE],
                                  uint8_t plaintext_out[AEAD_PAYLOAD_SIZE]) {
    uint8_t subkey[32];
    uint8_t inner[12];
    aead_result_t r;
    hchacha20(key, nonce, subkey);
    build_inner_nonce(nonce, inner);
    r = xchacha_decrypt(subkey, inner, sealed, plaintext_out);
    xchacha_wipe(subkey, inner);
    return r;
}

/**
 * @brief Initialize the AEAD subsystem.
 *
 * @param algo Backend selector.
 * @return bool true when initialization succeeds, else false.
 */
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

/**
 * @brief Return the nonce size required by the active backend.
 *
 * @param void No parameters.
 * @return size_t Nonce size in bytes, or 0 when not initialized.
 */
size_t aead_nonce_size(void) {
    if (!g_aead_ready) return 0u;
    if (g_aead_algo == AEAD_ALGO_AES_256_GCM) return AEAD_AES_NONCE_SIZE;
    return AEAD_XCHACHA_NONCE_SIZE;
}

/**
 * @brief Seal a plaintext payload into ciphertext plus tag.
 *
 * @param key Pointer to 32-byte AEAD key.
 * @param nonce Pointer to backend nonce bytes.
 * @param plaintext Pointer to AEAD_PAYLOAD_SIZE plaintext bytes.
 * @param sealed_out Output AEAD_SEALED_SIZE buffer.
 * @return aead_result_t Detailed sealing outcome.
 */
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

/**
 * @brief Open a sealed payload, verifying the tag before releasing plaintext.
 *
 * @param key Pointer to 32-byte AEAD key.
 * @param nonce Pointer to backend nonce bytes.
 * @param sealed Pointer to AEAD_SEALED_SIZE sealed bytes.
 * @param plaintext_out Output AEAD_PAYLOAD_SIZE plaintext buffer.
 * @return aead_result_t Detailed opening outcome.
 */
aead_result_t aead_open(const uint8_t key[32], const uint8_t *nonce,
                        const uint8_t sealed[AEAD_SEALED_SIZE],
                        uint8_t plaintext_out[AEAD_PAYLOAD_SIZE]) {
    if (!g_aead_ready || (key == NULL) || (nonce == NULL) ||
        (sealed == NULL) || (plaintext_out == NULL)) {
        return AEAD_RESULT_BAD_ARGUMENT;
    }
    if (g_aead_algo == AEAD_ALGO_AES_256_GCM) {
        return open_aes_gcm(key, nonce, sealed, plaintext_out);
    }
    return open_xchacha(key, nonce, sealed, plaintext_out);
}
```

## 4. AArch64 Disassembly

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_aead_init                      ; -- Begin function aead_init
	.p2align	2
_aead_init:                             ; @aead_init
	.cfi_startproc
; %bb.0:
	cmp	w0, #1
	b.hi	LBB0_2
; %bb.1:
Lloh0:
	adrp	x8, _g_aead_algo@PAGE
	str	w0, [x8, _g_aead_algo@PAGEOFF]
Lloh1:
	adrp	x8, _g_last_nonce@PAGE
Lloh2:
	add	x8, x8, _g_last_nonce@PAGEOFF
	stp	xzr, xzr, [x8, #8]
	str	xzr, [x8]
	mov	w8, #1                          ; =0x1
	adrp	x9, _g_aead_ready@PAGE
	strb	w8, [x9, _g_aead_ready@PAGEOFF]
LBB0_2:
	cmp	w0, #2
	cset	w0, lo
	ret
	.loh AdrpAdd	Lloh1, Lloh2
	.loh AdrpAdrp	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_aead_nonce_size                ; -- Begin function aead_nonce_size
	.p2align	2
_aead_nonce_size:                       ; @aead_nonce_size
	.cfi_startproc
; %bb.0:
	adrp	x8, _g_aead_ready@PAGE
	ldrb	w8, [x8, _g_aead_ready@PAGEOFF]
Lloh3:
	adrp	x9, _g_aead_algo@PAGE
Lloh4:
	ldr	w9, [x9, _g_aead_algo@PAGEOFF]
	cmp	w9, #0
	mov	w9, #24                         ; =0x18
	mov	w10, #12                        ; =0xc
	csel	x9, x10, x9, eq
	cmp	w8, #0
	csel	x0, x9, xzr, ne
	ret
	.loh AdrpLdr	Lloh3, Lloh4
	.cfi_endproc
                                        ; -- End function
	.globl	_aead_seal                      ; -- Begin function aead_seal
	.p2align	2
_aead_seal:                             ; @aead_seal
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #128
	stp	x24, x23, [sp, #64]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #80]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #96]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #112]            ; 16-byte Folded Spill
	add	x29, sp, #112
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x23, x0
Lloh5:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh6:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh7:
	ldr	x8, [x8]
	str	x8, [sp, #56]
	mov	w0, #1                          ; =0x1
	cbz	x3, LBB2_24
; %bb.1:
	cbz	x2, LBB2_24
; %bb.2:
	mov	x22, x1
	cbz	x1, LBB2_24
; %bb.3:
	cbz	x23, LBB2_24
; %bb.4:
	adrp	x8, _g_aead_ready@PAGE
	ldrb	w8, [x8, _g_aead_ready@PAGEOFF]
	cbz	w8, LBB2_24
; %bb.5:
	mov	x20, x2
	mov	x19, x3
Lloh8:
	adrp	x9, _g_aead_algo@PAGE
Lloh9:
	ldr	w21, [x9, _g_aead_algo@PAGEOFF]
	cmp	w21, #0
	mov	w9, #24                         ; =0x18
	mov	w10, #12                        ; =0xc
	csel	x9, x10, x9, eq
	cmp	w8, #0
	csel	x2, x9, xzr, ne
Lloh10:
	adrp	x0, _g_last_nonce@PAGE
Lloh11:
	add	x0, x0, _g_last_nonce@PAGEOFF
	mov	x1, x22
	mov	w3, #24                         ; =0x18
	bl	___memcpy_chk
	cbz	w21, LBB2_12
; %bb.6:
	add	x2, sp, #24
	mov	x0, x23
	mov	x1, x22
	bl	_hchacha20
	str	wzr, [sp, #12]
	ldr	x8, [x22, #16]
	stur	x8, [sp, #16]
	bl	_EVP_CIPHER_CTX_new
	cbz	x0, LBB2_23
; %bb.7:
	mov	x21, x0
	bl	_EVP_chacha20_poly1305
	mov	x1, x0
	mov	x0, x21
	mov	x2, #0                          ; =0x0
	mov	x3, #0                          ; =0x0
	mov	x4, #0                          ; =0x0
	bl	_EVP_EncryptInit_ex
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.8:
	mov	x0, x21
	mov	w1, #9                          ; =0x9
	mov	w2, #12                         ; =0xc
	mov	x3, #0                          ; =0x0
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.9:
	add	x3, sp, #24
	add	x4, sp, #12
	mov	x0, x21
	mov	x1, #0                          ; =0x0
	mov	x2, #0                          ; =0x0
	bl	_EVP_EncryptInit_ex
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.10:
	str	wzr, [sp, #8]
	add	x2, sp, #8
	mov	x0, x21
	mov	x1, x19
	mov	x3, x20
	mov	w4, #48                         ; =0x30
	bl	_EVP_EncryptUpdate
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.11:
	str	wzr, [sp, #8]
	add	x1, x19, #48
	add	x2, sp, #8
	b	LBB2_18
LBB2_12:
	bl	_EVP_CIPHER_CTX_new
	cbz	x0, LBB2_23
; %bb.13:
	mov	x21, x0
	bl	_EVP_aes_256_gcm
	mov	x1, x0
	mov	x0, x21
	mov	x2, #0                          ; =0x0
	mov	x3, #0                          ; =0x0
	mov	x4, #0                          ; =0x0
	bl	_EVP_EncryptInit_ex
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.14:
	mov	x0, x21
	mov	w1, #9                          ; =0x9
	mov	w2, #12                         ; =0xc
	mov	x3, #0                          ; =0x0
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.15:
	mov	x0, x21
	mov	x1, #0                          ; =0x0
	mov	x2, #0                          ; =0x0
	mov	x3, x23
	mov	x4, x22
	bl	_EVP_EncryptInit_ex
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.16:
	str	wzr, [sp, #24]
	add	x2, sp, #24
	mov	x0, x21
	mov	x1, x19
	mov	x3, x20
	mov	w4, #48                         ; =0x30
	bl	_EVP_EncryptUpdate
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.17:
	str	wzr, [sp, #24]
	add	x1, x19, #48
	add	x2, sp, #24
LBB2_18:
	mov	x0, x21
	bl	_EVP_EncryptFinal_ex
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.19:
	add	x3, x19, #48
	mov	x0, x21
	mov	w1, #16                         ; =0x10
	mov	w2, #16                         ; =0x10
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB2_21
; %bb.20:
	mov	w19, #0                         ; =0x0
	b	LBB2_22
LBB2_21:
	mov	w19, #3                         ; =0x3
LBB2_22:
	mov	x0, x21
	bl	_EVP_CIPHER_CTX_free
	mov	x0, x19
	b	LBB2_24
LBB2_23:
	mov	w0, #3                          ; =0x3
LBB2_24:
	ldr	x8, [sp, #56]
Lloh12:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh13:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh14:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB2_26
; %bb.25:
	ldp	x29, x30, [sp, #112]            ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #96]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #80]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #64]             ; 16-byte Folded Reload
	add	sp, sp, #128
	ret
LBB2_26:
	bl	___stack_chk_fail
	.loh AdrpLdrGotLdr	Lloh5, Lloh6, Lloh7
	.loh AdrpAdd	Lloh10, Lloh11
	.loh AdrpLdr	Lloh8, Lloh9
	.loh AdrpLdrGotLdr	Lloh12, Lloh13, Lloh14
	.cfi_endproc
                                        ; -- End function
	.globl	_aead_open                      ; -- Begin function aead_open
	.p2align	2
_aead_open:                             ; @aead_open
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #128
	stp	x24, x23, [sp, #64]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #80]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #96]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #112]            ; 16-byte Folded Spill
	add	x29, sp, #112
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x23, x0
Lloh15:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh16:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh17:
	ldr	x8, [x8]
	str	x8, [sp, #56]
	mov	w0, #1                          ; =0x1
	cbz	x3, LBB3_25
; %bb.1:
	cbz	x2, LBB3_25
; %bb.2:
	cbz	x1, LBB3_25
; %bb.3:
	cbz	x23, LBB3_25
; %bb.4:
	adrp	x8, _g_aead_ready@PAGE
	ldrb	w8, [x8, _g_aead_ready@PAGEOFF]
	cbz	w8, LBB3_25
; %bb.5:
	mov	x19, x3
	mov	x20, x2
Lloh18:
	adrp	x8, _g_aead_algo@PAGE
Lloh19:
	ldr	w8, [x8, _g_aead_algo@PAGEOFF]
	cbz	w8, LBB3_13
; %bb.6:
	add	x2, sp, #24
	mov	x0, x23
	mov	x21, x1
	bl	_hchacha20
	str	wzr, [sp, #12]
	ldr	x8, [x21, #16]
	stur	x8, [sp, #16]
	bl	_EVP_CIPHER_CTX_new
	cbz	x0, LBB3_24
; %bb.7:
	mov	x21, x0
	bl	_EVP_chacha20_poly1305
	mov	x1, x0
	mov	x0, x21
	mov	x2, #0                          ; =0x0
	mov	x3, #0                          ; =0x0
	mov	x4, #0                          ; =0x0
	bl	_EVP_DecryptInit_ex
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.8:
	mov	x0, x21
	mov	w1, #9                          ; =0x9
	mov	w2, #12                         ; =0xc
	mov	x3, #0                          ; =0x0
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.9:
	add	x3, sp, #24
	add	x4, sp, #12
	mov	x0, x21
	mov	x1, #0                          ; =0x0
	mov	x2, #0                          ; =0x0
	bl	_EVP_DecryptInit_ex
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.10:
	str	wzr, [sp, #8]
	add	x2, sp, #8
	mov	x0, x21
	mov	x1, x19
	mov	x3, x20
	mov	w4, #48                         ; =0x30
	bl	_EVP_DecryptUpdate
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.11:
	ldrsw	x22, [sp, #8]
	add	x3, x20, #48
	mov	x0, x21
	mov	w1, #17                         ; =0x11
	mov	w2, #16                         ; =0x10
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.12:
	add	x1, x19, x22
	add	x2, sp, #8
	b	LBB3_20
LBB3_13:
	mov	x22, x1
	bl	_EVP_CIPHER_CTX_new
	cbz	x0, LBB3_24
; %bb.14:
	mov	x21, x0
	bl	_EVP_aes_256_gcm
	mov	x1, x0
	mov	x0, x21
	mov	x2, #0                          ; =0x0
	mov	x3, #0                          ; =0x0
	mov	x4, #0                          ; =0x0
	bl	_EVP_DecryptInit_ex
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.15:
	mov	x0, x21
	mov	w1, #9                          ; =0x9
	mov	w2, #12                         ; =0xc
	mov	x3, #0                          ; =0x0
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.16:
	mov	x0, x21
	mov	x1, #0                          ; =0x0
	mov	x2, #0                          ; =0x0
	mov	x3, x23
	mov	x4, x22
	bl	_EVP_DecryptInit_ex
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.17:
	str	wzr, [sp, #24]
	add	x2, sp, #24
	mov	x0, x21
	mov	x1, x19
	mov	x3, x20
	mov	w4, #48                         ; =0x30
	bl	_EVP_DecryptUpdate
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.18:
	ldrsw	x22, [sp, #24]
	add	x3, x20, #48
	mov	x0, x21
	mov	w1, #17                         ; =0x11
	mov	w2, #16                         ; =0x10
	bl	_EVP_CIPHER_CTX_ctrl
	cmp	w0, #1
	b.ne	LBB3_22
; %bb.19:
	add	x1, x19, x22
	add	x2, sp, #24
LBB3_20:
	mov	x0, x21
	bl	_EVP_DecryptFinal_ex
	mov	x20, x0
	mov	x0, x21
	bl	_EVP_CIPHER_CTX_free
	cmp	w20, #1
	b.ne	LBB3_23
; %bb.21:
	mov	w0, #0                          ; =0x0
	b	LBB3_25
LBB3_22:
	mov	x0, x21
	bl	_EVP_CIPHER_CTX_free
LBB3_23:
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [x19, #16]
	str	q0, [x19]
	mov	w0, #2                          ; =0x2
	b	LBB3_25
LBB3_24:
	mov	w0, #3                          ; =0x3
LBB3_25:
	ldr	x8, [sp, #56]
Lloh20:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh21:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh22:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB3_27
; %bb.26:
	ldp	x29, x30, [sp, #112]            ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #96]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #80]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #64]             ; 16-byte Folded Reload
	add	sp, sp, #128
	ret
LBB3_27:
	bl	___stack_chk_fail
	.loh AdrpLdrGotLdr	Lloh15, Lloh16, Lloh17
	.loh AdrpLdr	Lloh18, Lloh19
	.loh AdrpLdrGotLdr	Lloh20, Lloh21, Lloh22
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function hchacha20
_hchacha20:                             ; @hchacha20
	.cfi_startproc
; %bb.0:
	stp	x20, x19, [sp, #-16]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 16
	.cfi_offset w19, -8
	.cfi_offset w20, -16
	mov	w9, #0                          ; =0x0
	ldp	w19, w11, [x0]
	ldp	w15, w16, [x0, #8]
	ldp	w6, w7, [x0, #16]
	ldp	w10, w12, [x0, #24]
	ldp	w14, w13, [x1]
	mov	w0, #25972                      ; =0x6574
	movk	w0, #27424, lsl #16
	mov	w3, #11570                      ; =0x2d32
	movk	w3, #31074, lsl #16
	mov	w4, #25710                      ; =0x646e
	movk	w4, #13088, lsl #16
	mov	w5, #30821                      ; =0x7865
	movk	w5, #24944, lsl #16
	ldp	w17, w8, [x1, #8]
LBB4_1:                                 ; =>This Inner Loop Header: Depth=1
	add	w1, w5, w19
	eor	w14, w14, w1
	ror	w14, w14, #16
	add	w5, w14, w6
	eor	w6, w5, w19
	ror	w19, w6, #20
	add	w20, w19, w1
	eor	w14, w20, w14
	ror	w14, w14, #24
	add	w6, w14, w5
	eor	w1, w6, w19
	ror	w1, w1, #25
	add	w4, w4, w11
	eor	w13, w13, w4
	ror	w13, w13, #16
	add	w5, w13, w7
	eor	w11, w5, w11
	ror	w11, w11, #20
	add	w4, w11, w4
	eor	w13, w4, w13
	ror	w13, w13, #24
	add	w7, w13, w5
	eor	w11, w7, w11
	ror	w11, w11, #25
	add	w3, w3, w15
	eor	w17, w17, w3
	ror	w17, w17, #16
	add	w10, w17, w10
	eor	w15, w10, w15
	ror	w15, w15, #20
	add	w3, w15, w3
	eor	w17, w3, w17
	ror	w17, w17, #24
	add	w10, w17, w10
	eor	w15, w10, w15
	ror	w15, w15, #25
	add	w0, w0, w16
	eor	w8, w8, w0
	ror	w8, w8, #16
	add	w12, w8, w12
	eor	w16, w12, w16
	ror	w16, w16, #20
	add	w0, w16, w0
	eor	w8, w0, w8
	ror	w8, w8, #24
	add	w12, w8, w12
	eor	w16, w12, w16
	ror	w16, w16, #25
	add	w5, w11, w20
	eor	w8, w8, w5
	ror	w8, w8, #16
	add	w10, w8, w10
	eor	w11, w10, w11
	ror	w11, w11, #20
	add	w5, w11, w5
	eor	w8, w5, w8
	ror	w8, w8, #24
	add	w10, w8, w10
	eor	w11, w10, w11
	ror	w11, w11, #25
	add	w4, w15, w4
	eor	w14, w4, w14
	ror	w14, w14, #16
	add	w12, w14, w12
	eor	w15, w12, w15
	ror	w15, w15, #20
	add	w4, w15, w4
	eor	w14, w4, w14
	ror	w14, w14, #24
	add	w12, w14, w12
	eor	w15, w12, w15
	ror	w15, w15, #25
	add	w3, w16, w3
	eor	w13, w3, w13
	ror	w13, w13, #16
	add	w6, w13, w6
	eor	w16, w6, w16
	ror	w16, w16, #20
	add	w3, w16, w3
	eor	w13, w3, w13
	ror	w13, w13, #24
	add	w6, w13, w6
	eor	w16, w6, w16
	ror	w16, w16, #25
	add	w0, w0, w1
	eor	w17, w0, w17
	ror	w17, w17, #16
	add	w7, w17, w7
	eor	w1, w7, w1
	ror	w1, w1, #20
	add	w0, w1, w0
	eor	w17, w0, w17
	ror	w17, w17, #24
	add	w7, w17, w7
	eor	w1, w7, w1
	ror	w19, w1, #25
	cmp	w9, #9
	add	w9, w9, #1
	b.lo	LBB4_1
; %bb.2:
	strb	w5, [x2]
	lsr	w9, w5, #8
	strb	w9, [x2, #1]
	lsr	w9, w5, #16
	strb	w9, [x2, #2]
	lsr	w9, w5, #24
	strb	w9, [x2, #3]
	strb	w4, [x2, #4]
	lsr	w9, w4, #8
	strb	w9, [x2, #5]
	lsr	w9, w4, #16
	strb	w9, [x2, #6]
	lsr	w9, w4, #24
	strb	w9, [x2, #7]
	strb	w3, [x2, #8]
	lsr	w9, w3, #8
	strb	w9, [x2, #9]
	lsr	w9, w3, #16
	strb	w9, [x2, #10]
	lsr	w9, w3, #24
	strb	w9, [x2, #11]
	strb	w0, [x2, #12]
	lsr	w9, w0, #8
	strb	w9, [x2, #13]
	lsr	w9, w0, #16
	strb	w9, [x2, #14]
	lsr	w9, w0, #24
	strb	w9, [x2, #15]
	strb	w14, [x2, #16]
	lsr	w9, w14, #8
	strb	w9, [x2, #17]
	lsr	w9, w14, #16
	strb	w9, [x2, #18]
	lsr	w9, w14, #24
	strb	w9, [x2, #19]
	strb	w13, [x2, #20]
	lsr	w9, w13, #8
	strb	w9, [x2, #21]
	lsr	w9, w13, #16
	strb	w9, [x2, #22]
	lsr	w9, w13, #24
	strb	w9, [x2, #23]
	strb	w17, [x2, #24]
	lsr	w9, w17, #8
	strb	w9, [x2, #25]
	lsr	w9, w17, #16
	strb	w9, [x2, #26]
	lsr	w9, w17, #24
	strb	w9, [x2, #27]
	strb	w8, [x2, #28]
	lsr	w9, w8, #8
	strb	w9, [x2, #29]
	lsr	w9, w8, #16
	strb	w9, [x2, #30]
	lsr	w8, w8, #24
	strb	w8, [x2, #31]
	ldp	x20, x19, [sp], #16             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
.zerofill __DATA,__bss,_g_aead_algo,4,2 ; @g_aead_algo
.zerofill __DATA,__bss,_g_last_nonce,24,0 ; @g_last_nonce
.zerofill __DATA,__bss,_g_aead_ready,1,0 ; @g_aead_ready
.subsections_via_symbols
```

## 5. Tests

```
137:static void test_aead_nonce_size_before_init(void) {
138:    TEST_ASSERT_EQUAL_UINT(0u, aead_nonce_size());
147:static void test_aead_init_rejects_bad_algo(void) {
148:    TEST_ASSERT_FALSE(aead_init((aead_algo_t)99));
157:static void test_aead_init_aes(void) {
158:    TEST_ASSERT_TRUE(aead_init(AEAD_ALGO_AES_256_GCM));
159:    TEST_ASSERT_EQUAL_UINT(AEAD_AES_NONCE_SIZE, aead_nonce_size());
168:static void test_aead_seal_bad_args(void) {
169:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_seal(NULL, g_nonce, g_pt, g_sealed));
170:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_seal(g_key, NULL, g_pt, g_sealed));
171:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_seal(g_key, g_nonce, NULL, g_sealed));
172:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_seal(g_key, g_nonce, g_pt, NULL));
181:static void test_aead_open_bad_args(void) {
182:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_open(NULL, g_nonce, g_sealed, g_pt));
183:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_open(g_key, NULL, g_sealed, g_pt));
184:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_open(g_key, g_nonce, NULL, g_pt));
185:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_open(g_key, g_nonce, g_sealed, NULL));
194:static void test_aead_seal_open_aes(void) {
196:    aead_init(AEAD_ALGO_AES_256_GCM);
199:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_SUCCESS, aead_seal(g_key, g_nonce, g_pt, g_sealed));
200:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_SUCCESS, aead_open(g_key, g_nonce, g_sealed, out));
210:static void test_aead_open_aes_bad_tag(void) {
212:    aead_init(AEAD_ALGO_AES_256_GCM);
214:    aead_seal(g_key, g_nonce, g_pt, g_sealed);
217:                          aead_open(g_key, g_nonce, g_sealed, out));
226:static void test_aead_nonce_reuse(void) {
227:    aead_init(AEAD_ALGO_AES_256_GCM);
229:    aead_seal(g_key, g_nonce, g_pt, g_sealed);
230:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_SUCCESS, aead_seal(g_key, g_nonce, g_pt, g_sealed));
239:static void test_aead_init_xchacha(void) {
240:    TEST_ASSERT_TRUE(aead_init(AEAD_ALGO_XCHACHA20_POLY1305));
241:    TEST_ASSERT_EQUAL_UINT(AEAD_XCHACHA_NONCE_SIZE, aead_nonce_size());
250:static void test_aead_seal_open_xchacha(void) {
252:    aead_init(AEAD_ALGO_XCHACHA20_POLY1305);
255:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_SUCCESS, aead_seal(g_key, g_nonce, g_pt, g_sealed));
256:    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_SUCCESS, aead_open(g_key, g_nonce, g_sealed, out));
266:static void test_aead_open_xchacha_bad_tag(void) {
268:    aead_init(AEAD_ALGO_XCHACHA20_POLY1305);
270:    aead_seal(g_key, g_nonce, g_pt, g_sealed);
273:                          aead_open(g_key, g_nonce, g_sealed, out));
```

## 6. Labs

1. Read the header, then the source; explain each function.
2. Match each C function to its assembly block.
3. Run the tests for this module: `python3 scripts/run_tests.py | grep aead`.
4. State one way this module could be misused and one way it is defended.
