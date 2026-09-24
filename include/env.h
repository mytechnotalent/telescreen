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
// File:    env.h
// Desc:    Declares the U-Boot environment helpers. The environment partition
//          begins with a little-endian CRC32 of the remaining bytes, followed by
//          a NUL-separated key=value blob.
// Created: 2026

#ifndef ENV_H
#define ENV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Size of the leading environment CRC field in bytes.
 */
#define ENV_CRC_SIZE 4u

/**
 * @brief Compute the environment CRC32 over the key/value blob.
 *
 * The blob begins after the four-byte CRC field and runs to the end of the
 * supplied buffer.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @return uint32_t Computed standard CRC-32 over the blob.
 */
uint32_t env_compute_crc(const uint8_t *env, size_t len);

/**
 * @brief Verify the leading environment CRC32.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @return bool true when the stored CRC matches the computed value.
 */
bool env_crc_valid(const uint8_t *env, size_t len);

/**
 * @brief Find a key in the environment blob and return its value.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @param key NUL-terminated key to locate.
 * @return const char* Pointer to the value, or NULL when absent.
 */
const char *env_find(const uint8_t *env, size_t len, const char *key);

#endif // ENV_H
