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

size_t aead_nonce_size(void) {
    if (!g_aead_ready) return 0u;
    if (g_aead_algo == AEAD_ALGO_AES_256_GCM) return AEAD_AES_NONCE_SIZE;
    return AEAD_XCHACHA_NONCE_SIZE;
}

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
