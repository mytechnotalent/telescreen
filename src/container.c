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
// File:    container.c
// Desc:    Implements the vendor kernel container helpers used to identify and
//          validate the kernel partition before inflation.
// Created: 2026

#include "container.h"
#include <string.h>

/**
 * @brief Read a 32-bit little-endian word from bytes.
 *
 * @param src Pointer to four readable bytes.
 * @return uint32_t Parsed 32-bit word.
 */
static uint32_t cont_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}

uint32_t container_magic(const uint8_t *buf) {
    return cont_read32(buf);
}

uint32_t container_length(const uint8_t *buf) {
    return cont_read32(&buf[4]);
}

bool container_tag_ok(const uint8_t *buf) {
    return memcmp(&buf[8], "gziphead", 8u) == 0;
}

bool container_valid(const uint8_t *buf, size_t len) {
    if ((buf == NULL) || (len < CONTAINER_HDR_SIZE)) return false;
    return (container_magic(buf) == CONTAINER_MAGIC) && container_tag_ok(buf);
}
