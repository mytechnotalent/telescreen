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
// File:    beacon.c
// Desc:    Implements the TELESCREEN exfiltration beacon: the deliberately weak
//          UID-derived key, the sequence nonce, and the hardened AEAD seal.
// Created: 2026

#include "beacon.h"
#include "aead.h"
#include "crc.h"
#include <string.h>

void beacon_weak_key(const char *uid, uint8_t out[BEACON_KEY_SIZE]) {
    size_t ulen = strlen(uid);
    uint32_t seed = crc32_le(0u, (const uint8_t *)uid, ulen);
    size_t i;
    for (i = 0u; i < BEACON_KEY_SIZE; ++i) {
        seed = crc32_le(seed, (const uint8_t *)uid, ulen);
        out[i] = (uint8_t)(seed & 0xFFu);
    }
}

void beacon_nonce(uint32_t seq, uint8_t *out) {
    out[0] = (uint8_t)(seq & 0xFFu);
    out[1] = (uint8_t)((seq >> 8u) & 0xFFu);
    out[2] = (uint8_t)((seq >> 16u) & 0xFFu);
    out[3] = (uint8_t)((seq >> 24u) & 0xFFu);
    memset(&out[4], 0, 20u);
}

bool beacon_seal(const uint8_t key[BEACON_KEY_SIZE], uint32_t seq,
                 const uint8_t *plaintext, uint8_t *sealed_out) {
    uint8_t nonce[AEAD_XCHACHA_NONCE_SIZE];
    beacon_nonce(seq, nonce);
    return aead_seal(key, nonce, plaintext, sealed_out) == AEAD_RESULT_SUCCESS;
}
