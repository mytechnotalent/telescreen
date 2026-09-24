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
// File:    env.c
// Desc:    Implements the U-Boot environment helpers: CRC computation, CRC
//          verification, and key lookup.
// Created: 2026

#include "env.h"
#include "crc.h"
#include <string.h>

/**
 * @brief Read a 32-bit little-endian word from bytes.
 *
 * @param src Pointer to four readable bytes.
 * @return uint32_t Parsed 32-bit word.
 */
static uint32_t env_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}

uint32_t env_compute_crc(const uint8_t *env, size_t len) {
    if ((env == NULL) || (len < ENV_CRC_SIZE)) return 0u;
    return crc32_uboot(&env[ENV_CRC_SIZE], len - ENV_CRC_SIZE);
}

bool env_crc_valid(const uint8_t *env, size_t len) {
    if ((env == NULL) || (len < ENV_CRC_SIZE)) return false;
    return env_read32(env) == env_compute_crc(env, len);
}

const char *env_find(const uint8_t *env, size_t len, const char *key) {
    size_t klen;
    size_t i;
    if ((env == NULL) || (key == NULL) || (len < ENV_CRC_SIZE)) return NULL;
    klen = strlen(key);
    for (i = ENV_CRC_SIZE; (i + klen + 1u) < len; ++i) {
        if ((env[i + klen] == '=') && (memcmp(&env[i], key, klen) == 0)) {
            return (const char *)&env[i + klen + 1u];
        }
    }
    return NULL;
}
