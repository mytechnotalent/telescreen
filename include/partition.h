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
// GitHub:  https://github.com/mytechnotalent
// File:    partition.h
// Desc:    Declares the TELESCREEN four-partition model. The captured camera and
//          the RP5 replica share the same layout: boot, bootargs, kernel, rootfs,
//          with identical offsets, sizes, and magic bytes.
// Created: 2026

#ifndef PARTITION_H
#define PARTITION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Whole-flash image size in bytes (16 MiB).
 */
#define PART_IMAGE_SIZE 0x1000000u

/**
 * @brief Boot partition offset (first stage + U-Boot).
 */
#define PART_BOOT_OFF 0x000000u

/**
 * @brief Boot partition size in bytes (128 KiB).
 */
#define PART_BOOT_SIZE 0x020000u

/**
 * @brief Environment partition offset (U-Boot env).
 */
#define PART_ENV_OFF 0x020000u

/**
 * @brief Environment partition size in bytes (64 KiB).
 */
#define PART_ENV_SIZE 0x010000u

/**
 * @brief Kernel partition offset (vendor container).
 */
#define PART_KERNEL_OFF 0x030000u

/**
 * @brief Kernel partition size in bytes (1792 KiB).
 */
#define PART_KERNEL_SIZE 0x1C0000u

/**
 * @brief Rootfs partition offset (JFFS2).
 */
#define PART_ROOTFS_OFF 0x1F0000u

/**
 * @brief Rootfs partition size in bytes (14400 KiB).
 */
#define PART_ROOTFS_SIZE 0xE10000u

/**
 * @brief Boot partition magic: the ARM reset-vector word at offset 0.
 */
#define PART_BOOT_MAGIC 0xEA000515u

/**
 * @brief Vendor kernel container magic.
 */
#define PART_CONTAINER_MAGIC 0x001B8421u

/**
 * @brief JFFS2 little-endian magic as stored in the first two bytes.
 */
#define PART_JFFS2_MAGIC 0x1985u

/**
 * @brief Partition identity result codes.
 */
typedef enum part_result {
    PART_RESULT_OK = 0,
    PART_RESULT_BAD_ARGUMENT = 1,
    PART_RESULT_TOO_SMALL = 2,
    PART_RESULT_UNKNOWN = 3,
} part_result_t;

/**
 * @brief Identified partition kind.
 */
typedef enum part_kind {
    PART_KIND_UNKNOWN = 0,
    PART_KIND_BOOT = 1,
    PART_KIND_ENV = 2,
    PART_KIND_KERNEL = 3,
    PART_KIND_ROOTFS = 4,
} part_kind_t;

/**
 * @brief A partition descriptor (offset, size, name).
 */
typedef struct part_desc {
    part_kind_t kind;
    uint32_t offset;
    uint32_t size;
    const char *name;
} part_desc_t;

/**
 * @brief Return the descriptor for a partition kind.
 *
 * @param kind Partition kind.
 * @return const part_desc_t* Pointer to the static descriptor, or NULL.
 */
const part_desc_t *part_desc_for(part_kind_t kind);

/**
 * @brief Identify a partition by its magic bytes.
 *
 * Inspects the first bytes of a buffer and returns the partition kind. This is
 * how the layout is proven from the artifact rather than assumed.
 *
 * @param buf Pointer to the partition bytes.
 * @param len Number of readable bytes.
 * @return part_kind_t Identified kind, or PART_KIND_UNKNOWN.
 */
part_kind_t part_identify(const uint8_t *buf, size_t len);

/**
 * @brief Verify that a whole-flash image is large enough and internally consistent.
 *
 * Checks the image length against PART_IMAGE_SIZE and confirms each partition's
 * offset plus size stays within the image.
 *
 * @param image_size Size of the whole-flash image in bytes.
 * @return part_result_t Detailed verification outcome.
 */
part_result_t part_verify_layout(size_t image_size);

/**
 * @brief Copy one partition out of a whole-flash image.
 *
 * @param image Pointer to the whole-flash image.
 * @param image_size Size of the whole-flash image in bytes.
 * @param kind Partition kind to extract.
 * @param out Output buffer of at least the partition size.
 * @param out_size Size of the output buffer in bytes.
 * @return part_result_t Detailed extraction outcome.
 */
part_result_t part_carve(const uint8_t *image,
                         size_t image_size,
                         part_kind_t kind,
                         uint8_t *out,
                         size_t out_size);

#endif // PARTITION_H
