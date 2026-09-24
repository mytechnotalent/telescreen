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
// File:    identity.c
// Desc:    Implements the TELESCREEN Ed25519 device identity.
// Created: 2026

#include "identity.h"
#include <openssl/evp.h>

/**
 * @brief Build an Ed25519 private key object from a raw seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @return EVP_PKEY* Private key object, or NULL.
 */
static EVP_PKEY *identity_priv(const uint8_t priv[32]) {
    return EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL, priv, 32u);
}

/**
 * @brief Build an Ed25519 public key object from raw bytes.
 *
 * @param pub Pointer to 32-byte public key.
 * @return EVP_PKEY* Public key object, or NULL.
 */
static EVP_PKEY *identity_pub(const uint8_t pub[32]) {
    return EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, NULL, pub, 32u);
}

bool identity_pubkey(const uint8_t priv[IDENTITY_KEY_SIZE],
                     uint8_t pub[IDENTITY_KEY_SIZE]) {
    EVP_PKEY *k = identity_priv(priv);
    size_t n = 32u;
    bool ok = (k != NULL) && (EVP_PKEY_get_raw_public_key(k, pub, &n) == 1);
    EVP_PKEY_free(k);
    return ok && (n == 32u);
}

/**
 * @brief Initialize an Ed25519 signing context.
 *
 * @param c Pointer to the message digest context.
 * @param k Pointer to the private key.
 * @return bool true when initialization succeeds.
 */
static bool ed_sign_init(EVP_MD_CTX *c, EVP_PKEY *k) {
    return EVP_DigestSignInit(c, NULL, NULL, NULL, k) == 1;
}

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

/**
 * @brief Initialize an Ed25519 verification context.
 *
 * @param c Pointer to the message digest context.
 * @param k Pointer to the public key.
 * @return bool true when initialization succeeds.
 */
static bool ed_verify_init(EVP_MD_CTX *c, EVP_PKEY *k) {
    return EVP_DigestVerifyInit(c, NULL, NULL, NULL, k) == 1;
}

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
