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
