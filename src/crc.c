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
// File:    crc.c
// Desc:    Implements the crc32_le, standard CRC32, and CRC-16/CCITT integrity
//          primitives used by the JFFS2 rootfs, the U-Boot environment, and
//          telemetry frames.
// Created: 2026

#include "crc.h"

/**
 * @brief Fold one byte into a reflected CRC-32 residual.
 *
 * The reflected representation uses the 0xEDB88320 polynomial, shifting right
 * and XORing the polynomial when the low bit is set.
 *
 * @param crc Current running checksum.
 * @param byte Byte to fold in.
 * @return uint32_t Updated reflected checksum.
 */
static uint32_t crc32_byte(uint32_t crc, uint8_t byte) {
    uint8_t i;
    crc ^= (uint32_t)byte;
    for (i = 0u; i < 8u; ++i) {
        crc = (crc >> 1u) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1u)));
    }
    return crc;
}

/**
 * @brief Fold one byte into a CRC-16/CCITT-FALSE residual.
 *
 * @param crc Current running checksum.
 * @param byte Byte to fold in.
 * @return uint16_t Updated checksum.
 */
static uint16_t crc16_byte(uint16_t crc, uint8_t byte) {
    uint8_t i;
    uint16_t inbit;
    for (i = 0u; i < 8u; ++i) {
        inbit = (uint16_t)((crc ^ ((uint16_t)byte << 8u)) & 0x8000u);
        crc = (uint16_t)(crc << 1u);
        if (inbit != 0u) crc ^= 0x1021u;
        byte = (uint8_t)(byte << 1u);
    }
    return crc;
}

uint32_t crc32_le(uint32_t seed, const uint8_t *buf, size_t len) {
    uint32_t crc = seed;
    size_t i;
    for (i = 0u; i < len; ++i) crc = crc32_byte(crc, buf[i]);
    return crc;
}

uint32_t crc32_uboot(const uint8_t *buf, size_t len) {
    return crc32_le(0xFFFFFFFFu, buf, len) ^ 0xFFFFFFFFu;
}

uint16_t crc16_ccitt(const uint8_t *buf, size_t len) {
    uint16_t crc = CRC16_INIT;
    size_t i;
    for (i = 0u; i < len; ++i) crc = crc16_byte(crc, buf[i]);
    return crc;
}
