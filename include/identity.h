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
// File:    identity.h
// Desc:    Declares the TELESCREEN device identity: Ed25519 public-key derivation,
//          signing, and verification. This replaces RSA for the hardened build.
// Created: 2026

#ifndef IDENTITY_H
#define IDENTITY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Ed25519 private seed and public key size in bytes.
 */
#define IDENTITY_KEY_SIZE 32u

/**
 * @brief Ed25519 signature size in bytes.
 */
#define IDENTITY_SIG_SIZE 64u

/**
 * @brief Derive the Ed25519 public key from a private seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @param pub Output 32-byte public key.
 * @return bool true when derivation succeeds.
 */
bool identity_pubkey(const uint8_t priv[IDENTITY_KEY_SIZE],
                     uint8_t pub[IDENTITY_KEY_SIZE]);

/**
 * @brief Sign a message with an Ed25519 private seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @param msg Pointer to message bytes.
 * @param msg_len Number of message bytes.
 * @param sig Output 64-byte signature.
 * @return bool true when signing succeeds.
 */
bool identity_sign(const uint8_t priv[IDENTITY_KEY_SIZE], const uint8_t *msg,
                   size_t msg_len, uint8_t sig[IDENTITY_SIG_SIZE]);

/**
 * @brief Verify an Ed25519 signature.
 *
 * @param pub Pointer to 32-byte public key.
 * @param msg Pointer to message bytes.
 * @param msg_len Number of message bytes.
 * @param sig Pointer to 64-byte signature.
 * @return bool true when the signature verifies.
 */
bool identity_verify(const uint8_t pub[IDENTITY_KEY_SIZE], const uint8_t *msg,
                     size_t msg_len, const uint8_t sig[IDENTITY_SIG_SIZE]);

#endif // IDENTITY_H
