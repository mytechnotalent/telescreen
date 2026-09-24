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
// File:    collector.c
// Desc:    Implements the local lab sink that receives TELESCREEN beacons.
// Created: 2026

#include "collector.h"
#include "aead.h"

/**
 * @brief Collector accounting state.
 */
static collector_stat_t g_stats;

void collector_init(void) {
    g_stats.accepted = 0u;
    g_stats.rejected = 0u;
    g_stats.last_seq = 0u;
}

bool collector_accept(const uint8_t *sealed, size_t len, uint32_t seq) {
    if ((sealed == NULL) || (len != AEAD_SEALED_SIZE)) {
        g_stats.rejected += 1u;
        return false;
    }
    g_stats.accepted += 1u;
    g_stats.last_seq = seq;
    return true;
}

const collector_stat_t *collector_stats(void) {
    return &g_stats;
}
