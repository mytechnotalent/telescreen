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
// File:    collector.h
// Desc:    Declares the local lab sink that receives TELESCREEN beacons. It is
//          local-only by design and never bridges to a production network.
// Created: 2026

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Collector accounting snapshot.
 */
typedef struct collector_stat {
    /**
     * @brief Number of accepted beacons.
     */
    uint32_t accepted;
    /**
     * @brief Number of rejected beacons.
     */
    uint32_t rejected;
    /**
     * @brief Sequence number of the most recent accepted beacon.
     */
    uint32_t last_seq;
} collector_stat_t;

/**
 * @brief Reset the collector accounting state.
 *
 * @param void No parameters.
 * @return void
 */
void collector_init(void);

/**
 * @brief Accept or reject one beacon frame.
 *
 * @param sealed Pointer to the sealed frame bytes.
 * @param len Number of frame bytes.
 * @param seq Sequence number carried by the frame.
 * @return bool true when the frame is accepted.
 */
bool collector_accept(const uint8_t *sealed, size_t len, uint32_t seq);

/**
 * @brief Return a pointer to the collector accounting snapshot.
 *
 * @param void No parameters.
 * @return const collector_stat_t* Pointer to the static snapshot.
 */
const collector_stat_t *collector_stats(void);

#endif // COLLECTOR_H
