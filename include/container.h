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
// File:    container.h
// Desc:    Declares the vendor kernel container helpers. The kernel partition is
//          not a zImage or uImage; it is a vendor container of the form
//          [magic 0x001B8421][length]["gziphead"][gzip stream named "Image"].
// Created: 2026

#ifndef CONTAINER_H
#define CONTAINER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Vendor kernel container magic value.
 */
#define CONTAINER_MAGIC 0x001B8421u

/**
 * @brief Vendor kernel container header size in bytes.
 */
#define CONTAINER_HDR_SIZE 16u

/**
 * @brief Read the container magic from the header.
 *
 * @param buf Pointer to the container header bytes.
 * @return uint32_t Parsed magic value.
 */
uint32_t container_magic(const uint8_t *buf);

/**
 * @brief Read the container length field from the header.
 *
 * @param buf Pointer to the container header bytes.
 * @return uint32_t Parsed length value.
 */
uint32_t container_length(const uint8_t *buf);

/**
 * @brief Verify the "gziphead" tag in the container header.
 *
 * @param buf Pointer to the container header bytes.
 * @return bool true when the tag is present.
 */
bool container_tag_ok(const uint8_t *buf);

/**
 * @brief Validate a vendor kernel container header.
 *
 * @param buf Pointer to the container bytes.
 * @param len Number of readable bytes.
 * @return bool true when the magic and tag both validate.
 */
bool container_valid(const uint8_t *buf, size_t len);

#endif // CONTAINER_H
