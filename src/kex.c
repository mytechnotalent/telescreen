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
// File:    kex.c
// Desc:    Implements X25519 key agreement and HKDF-SHA256 key derivation for the
//          hardened TELESCREEN session key.
// Created: 2026

#include "kex.h"
#include <string.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

/**
 * @brief Compute one HMAC-SHA256 over a byte range.
 *
 * @param key Pointer to HMAC key bytes.
 * @param key_len Number of key bytes.
 * @param data Pointer to data bytes.
 * @param data_len Number of data bytes.
 * @param out Output 32-byte MAC buffer.
 * @return bool true when the MAC succeeds.
 */
static bool hkdf_hmac(const uint8_t *key, size_t key_len, const uint8_t *data,
                      size_t data_len, uint8_t out[32]) {
    unsigned int n = 0u;
    return HMAC(EVP_sha256(), key, (int)key_len, data, data_len, out, &n) != NULL;
}

/**
 * @brief Assemble one HKDF expand input block.
 *
 * @param buf Output assembly buffer.
 * @param tprev Pointer to the previous T block.
 * @param tlen Number of previous T bytes.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param counter One-based block counter byte.
 * @return size_t Assembled block length.
 */
static size_t hkdf_assemble(uint8_t *buf, const uint8_t *tprev, size_t tlen,
                            const uint8_t *info, size_t info_len, uint8_t counter) {
    memcpy(buf, tprev, tlen);
    memcpy(&buf[tlen], info, info_len);
    buf[tlen + info_len] = counter;
    return tlen + info_len + 1u;
}

/**
 * @brief Compute one HKDF expand T block.
 *
 * @param prk Pointer to the 32-byte pseudorandom key.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param tprev Pointer to the previous T block.
 * @param tlen Number of previous T bytes.
 * @param counter One-based block counter byte.
 * @param out Output 32-byte T block buffer.
 * @return size_t Number of bytes produced, or zero on failure.
 */
static size_t hkdf_block(const uint8_t prk[32], const uint8_t *info, size_t info_len,
                         const uint8_t *tprev, size_t tlen, uint8_t counter,
                         uint8_t out[32]) {
    uint8_t buf[97];
    size_t blen = hkdf_assemble(buf, tprev, tlen, info, info_len, counter);
    if (!hkdf_hmac(prk, 32u, buf, blen, out)) return 0u;
    return 32u;
}

/**
 * @brief Produce and copy one HKDF expand block into the output.
 *
 * @param prk Pointer to the 32-byte pseudorandom key.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param t Pointer to the mutable T block buffer.
 * @param tlen Pointer to the current T block length.
 * @param counter One-based block counter byte.
 * @param out Output key material buffer.
 * @param done Pointer to the number of output bytes written.
 * @param out_len Total number of output bytes.
 * @return bool true when the step succeeds.
 */
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

/**
 * @brief Run the HKDF expand phase.
 *
 * @param prk Pointer to the 32-byte pseudorandom key.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param out Output key material buffer.
 * @param out_len Number of output bytes.
 * @return bool true when expansion succeeds.
 */
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

bool kex_hkdf(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
              size_t salt_len, const uint8_t *info, size_t info_len,
              uint8_t *out, size_t out_len) {
    uint8_t prk[32];
    if (!hkdf_hmac(salt, salt_len, ikm, ikm_len, prk)) return false;
    return hkdf_expand(prk, info, info_len, out, out_len);
}

/**
 * @brief Drive an X25519 EVP context to a shared secret.
 *
 * @param c Pointer to the EVP key context.
 * @param b Pointer to the peer public key.
 * @param out Output shared-secret buffer.
 * @param n Pointer to the output length.
 * @return bool true when the derivation succeeds.
 */
static bool x25519_derive(EVP_PKEY_CTX *c, EVP_PKEY *b, uint8_t *out, size_t *n) {
    if (c == NULL) return false;
    if (EVP_PKEY_derive_init(c) != 1) return false;
    if (EVP_PKEY_derive_set_peer(c, b) != 1) return false;
    return EVP_PKEY_derive(c, out, n) == 1;
}

/**
 * @brief Release the X25519 EVP objects.
 *
 * @param c Pointer to the EVP key context.
 * @param a Pointer to the private key.
 * @param b Pointer to the public key.
 * @return void
 */
static void kex_free(EVP_PKEY_CTX *c, EVP_PKEY *a, EVP_PKEY *b) {
    EVP_PKEY_CTX_free(c);
    EVP_PKEY_free(a);
    EVP_PKEY_free(b);
}

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
