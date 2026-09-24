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
// File:    partition.c
// Desc:    Implements the TELESCREEN four-partition model: descriptors, magic
//          identification, layout verification, and carving.
// Created: 2026

#include "partition.h"
#include <string.h>

/**
 * @brief The four partitions, in offset order.
 *
 * Exactly as the captured device declares them in mtdparts. The RP5 replica
 * uses the same table.
 */
static const part_desc_t g_parts[4] = {
    { PART_KIND_BOOT,   PART_BOOT_OFF,   PART_BOOT_SIZE,   "boot" },
    { PART_KIND_ENV,    PART_ENV_OFF,    PART_ENV_SIZE,    "bootargs" },
    { PART_KIND_KERNEL, PART_KERNEL_OFF, PART_KERNEL_SIZE, "kernel" },
    { PART_KIND_ROOTFS, PART_ROOTFS_OFF, PART_ROOTFS_SIZE, "rootfs" },
};

/**
 * @brief Read a 32-bit little-endian word from bytes.
 *
 * @param src Pointer to four readable bytes.
 * @return uint32_t Parsed 32-bit word.
 */
static uint32_t part_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}

/**
 * @brief Read a 16-bit little-endian value from bytes.
 *
 * @param src Pointer to two readable bytes.
 * @return uint16_t Parsed 16-bit value.
 */
static uint16_t part_read16(const uint8_t *src) {
    return (uint16_t)((uint16_t)src[0] | ((uint16_t)src[1] << 8u));
}

const part_desc_t *part_desc_for(part_kind_t kind) {
    size_t i;
    for (i = 0u; i < 4u; ++i) {
        if (g_parts[i].kind == kind) return &g_parts[i];
    }
    return NULL;
}

part_kind_t part_identify(const uint8_t *buf, size_t len) {
    if ((buf == NULL) || (len < 4u)) return PART_KIND_UNKNOWN;
    if (part_read32(buf) == PART_BOOT_MAGIC) return PART_KIND_BOOT;
    if (part_read16(buf) == PART_JFFS2_MAGIC) return PART_KIND_ROOTFS;
    if (part_read32(buf) == PART_CONTAINER_MAGIC) return PART_KIND_KERNEL;
    return PART_KIND_ENV;
}

part_result_t part_verify_layout(size_t image_size) {
    if (image_size < PART_IMAGE_SIZE) return PART_RESULT_TOO_SMALL;
    return PART_RESULT_OK;
}

part_result_t part_carve(const uint8_t *image, size_t image_size, part_kind_t kind,
                         uint8_t *out, size_t out_size) {
    const part_desc_t *desc = part_desc_for(kind);
    if ((image == NULL) || (out == NULL)) return PART_RESULT_BAD_ARGUMENT;
    if (image_size < PART_IMAGE_SIZE) return PART_RESULT_TOO_SMALL;
    if (desc == NULL) return PART_RESULT_BAD_ARGUMENT;
    if (out_size < desc->size) return PART_RESULT_TOO_SMALL;
    memcpy(out, &image[desc->offset], desc->size);
    return PART_RESULT_OK;
}
