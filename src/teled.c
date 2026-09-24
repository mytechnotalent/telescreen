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
// File:    teled.c
// Desc:    Implements the TELESCREEN daemon: identity, session key, beacon
//          sequence, and delivery to the local collector.
// Created: 2026

#include "teled.h"
#include "aead.h"
#include "beacon.h"
#include "collector.h"
#include <string.h>

/**
 * @brief TELESCREEN daemon state.
 */
static teled_state_t g_teled;

void teled_init(const char *uid, const uint8_t key[32]) {
    size_t n;
    memset(&g_teled, 0, sizeof(g_teled));
    n = strlen(uid);
    if (n >= (size_t)TELED_UID_MAX) n = (size_t)TELED_UID_MAX - 1u;
    memcpy(g_teled.uid, uid, n);
    memcpy(g_teled.key, key, 32u);
    collector_init();
}

bool teled_beacon(const uint8_t *payload) {
    uint8_t sealed[AEAD_SEALED_SIZE];
    g_teled.seq += 1u;
    if (!beacon_seal(g_teled.key, g_teled.seq, payload, sealed)) return false;
    g_teled.beacons += 1u;
    return collector_accept(sealed, sizeof(sealed), g_teled.seq);
}

const teled_state_t *teled_state(void) {
    return &g_teled;
}
