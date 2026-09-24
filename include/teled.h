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
// File:    teled.h
// Desc:    Declares the TELESCREEN daemon. It owns the device identity, the
//          hardened session key, and the beacon sequence, and hands sealed
//          beacons to the local collector.
// Created: 2026

#ifndef TELED_H
#define TELED_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Maximum device UID length stored by the daemon.
 */
#define TELED_UID_MAX 32u

/**
 * @brief TELESCREEN daemon state.
 */
typedef struct teled_state {
    /**
     * @brief NUL-terminated device UID.
     */
    char uid[TELED_UID_MAX];
    /**
     * @brief Hardened session key.
     */
    uint8_t key[32];
    /**
     * @brief Monotonic beacon sequence number.
     */
    uint32_t seq;
    /**
     * @brief Number of beacons sealed by the daemon.
     */
    uint32_t beacons;
} teled_state_t;

/**
 * @brief Initialize the daemon with a device UID and session key.
 *
 * @param uid NUL-terminated public device identifier.
 * @param key Pointer to 32-byte hardened session key.
 * @return void
 */
void teled_init(const char *uid, const uint8_t key[32]);

/**
 * @brief Seal and deliver one beacon to the local collector.
 *
 * @param payload Pointer to AEAD payload bytes.
 * @return bool true when the beacon is sealed and accepted.
 */
bool teled_beacon(const uint8_t *payload);

/**
 * @brief Return a pointer to the daemon state snapshot.
 *
 * @param void No parameters.
 * @return const teled_state_t* Pointer to the static state.
 */
const teled_state_t *teled_state(void);

#endif // TELED_H
