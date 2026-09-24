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
// File:    beacon.h
// Desc:    Declares the TELESCREEN exfiltration beacon. The Ministry derives its
//          key from the public device UID, which is obfuscation rather than
//          encryption. The hardened path uses the AEAD API instead.
// Created: 2026

#ifndef BEACON_H
#define BEACON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Beacon key size in bytes.
 */
#define BEACON_KEY_SIZE 32u

/**
 * @brief Beacon sequence number size in bytes.
 */
#define BEACON_SEQ_SIZE 4u

/**
 * @brief Derive the Ministry "sealed" key from the public device UID.
 *
 * This is deliberately weak: the key is a deterministic function of the public
 * UID, so anyone who knows the UID can recompute it. It exists to demonstrate
 * the failure, not to protect anything.
 *
 * @param uid NUL-terminated public device identifier.
 * @param out Output 32-byte key buffer.
 * @return void
 */
void beacon_weak_key(const char *uid, uint8_t out[BEACON_KEY_SIZE]);

/**
 * @brief Build the beacon nonce from the sequence number.
 *
 * @param seq Monotonic sequence number.
 * @param out Output nonce buffer of AEAD nonce size.
 * @return void
 */
void beacon_nonce(uint32_t seq, uint8_t *out);

/**
 * @brief Seal one beacon payload with the hardened AEAD key.
 *
 * @param key Pointer to 32-byte session key.
 * @param seq Monotonic sequence number.
 * @param plaintext Pointer to AEAD payload bytes.
 * @param sealed_out Output AEAD sealed buffer.
 * @return bool true when sealing succeeds.
 */
bool beacon_seal(const uint8_t key[BEACON_KEY_SIZE], uint32_t seq,
                 const uint8_t *plaintext, uint8_t *sealed_out);

#endif // BEACON_H
