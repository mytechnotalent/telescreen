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
// File:    crc.h
// Desc:    Declares the two integrity primitives the TELESCREEN images use: the
//          reflected crc32_le used by JFFS2 nodes, the standard CRC32 used by the
//          U-Boot environment, and the CRC-16/CCITT used by telemetry frames.
// Created: 2026

#ifndef CRC_H
#define CRC_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief CRC-16/CCITT seed value used for telemetry frame integrity.
 */
#define CRC16_INIT 0xFFFFu

/**
 * @brief Compute the reflected crc32_le checksum used by JFFS2.
 *
 * Uses the kernel crc32_le convention: reflected, caller-supplied seed, and no
 * final inversion. JFFS2 stores hdr_crc, node_crc, and data_crc this way.
 *
 * @param seed Initial checksum value.
 * @param buf Pointer to readable bytes.
 * @param len Number of bytes to checksum.
 * @return uint32_t Computed reflected CRC-32 residual.
 */
uint32_t crc32_le(uint32_t seed, const uint8_t *buf, size_t len);

/**
 * @brief Compute the standard CRC32 used by the U-Boot environment.
 *
 * Initializes with 0xFFFFFFFF and inverts the result, which is the convention
 * U-Boot applies to the environment blob.
 *
 * @param buf Pointer to readable bytes.
 * @param len Number of bytes to checksum.
 * @return uint32_t Computed standard CRC-32 value.
 */
uint32_t crc32_uboot(const uint8_t *buf, size_t len);

/**
 * @brief Compute the CRC-16/CCITT-FALSE checksum of a byte buffer.
 *
 * @param buf Pointer to readable bytes.
 * @param len Number of bytes to checksum.
 * @return uint16_t Computed CRC-16 residual value.
 */
uint16_t crc16_ccitt(const uint8_t *buf, size_t len);

#endif // CRC_H
