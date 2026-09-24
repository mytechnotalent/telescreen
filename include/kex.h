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
// File:    kex.h
// Desc:    Declares the hardened key agreement: X25519 for the shared secret and
//          HKDF-SHA256 to derive the AEAD session key.
// Created: 2026

#ifndef KEX_H
#define KEX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief X25519 key and shared-secret size in bytes.
 */
#define KEX_KEY_SIZE 32u

/**
 * @brief HKDF output key size in bytes.
 */
#define KEX_OKM_SIZE 32u

/**
 * @brief Compute the X25519 shared secret.
 *
 * @param priv Pointer to 32-byte private scalar.
 * @param peer Pointer to 32-byte peer public key.
 * @param out Output 32-byte shared secret.
 * @return bool true when the derivation succeeds.
 */
bool kex_x25519(const uint8_t priv[KEX_KEY_SIZE], const uint8_t peer[KEX_KEY_SIZE],
                uint8_t out[KEX_KEY_SIZE]);

/**
 * @brief Derive key material with HKDF-SHA256.
 *
 * @param ikm Pointer to input key material.
 * @param ikm_len Number of input key material bytes.
 * @param salt Pointer to salt bytes.
 * @param salt_len Number of salt bytes.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param out Output key material buffer.
 * @param out_len Number of output bytes.
 * @return bool true when derivation succeeds.
 */
bool kex_hkdf(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
              size_t salt_len, const uint8_t *info, size_t info_len,
              uint8_t *out, size_t out_len);

#endif // KEX_H
