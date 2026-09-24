# Module 02: partition

**purpose, full source, AArch64 disassembly, and tests**

***
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. 

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:

1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws regarding cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**

***

## 1. Purpose

See `README.md` for where `partition` sits in the lab. This volume is the whole module.

## 2. Header

```c
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
```

## 3. Source

```c
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

/**
 * @brief Return the end offset of partition slot i.
 *
 * @param i Partition table index.
 * @return size_t End offset in bytes.
 */
static size_t part_end(size_t i) {
    return (size_t)g_parts[i].offset + (size_t)g_parts[i].size;
}

/**
 * @brief Return the descriptor for a partition kind.
 *
 * @param kind Partition kind.
 * @return const part_desc_t* Pointer to the static descriptor, or NULL.
 */
const part_desc_t *part_desc_for(part_kind_t kind) {
    size_t i;
    for (i = 0u; i < 4u; ++i) {
        if (g_parts[i].kind == kind) return &g_parts[i];
    }
    return NULL;
}

/**
 * @brief Identify a partition by its magic bytes.
 *
 * @param buf Pointer to the partition bytes.
 * @param len Number of readable bytes.
 * @return part_kind_t Identified kind, or PART_KIND_UNKNOWN.
 */
part_kind_t part_identify(const uint8_t *buf, size_t len) {
    if ((buf == NULL) || (len < 4u)) return PART_KIND_UNKNOWN;
    if (part_read32(buf) == PART_BOOT_MAGIC) return PART_KIND_BOOT;
    if (part_read16(buf) == PART_JFFS2_MAGIC) return PART_KIND_ROOTFS;
    if (part_read32(buf) == PART_CONTAINER_MAGIC) return PART_KIND_KERNEL;
    return PART_KIND_ENV;
}

/**
 * @brief Verify that a whole-flash image is large enough and consistent.
 *
 * @param image_size Size of the whole-flash image in bytes.
 * @return part_result_t Detailed verification outcome.
 */
part_result_t part_verify_layout(size_t image_size) {
    size_t i;
    if (image_size < PART_IMAGE_SIZE) return PART_RESULT_TOO_SMALL;
    for (i = 0u; i < 4u; ++i) {
        if (part_end(i) > PART_IMAGE_SIZE) return PART_RESULT_UNKNOWN;
    }
    return PART_RESULT_OK;
}

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
```

## 4. AArch64 Disassembly

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_part_desc_for                  ; -- Begin function part_desc_for
	.p2align	2
_part_desc_for:                         ; @part_desc_for
	.cfi_startproc
; %bb.0:
	sub	w8, w0, #1
	cmp	w8, #3
	b.hi	LBB0_2
; %bb.1:
Lloh0:
	adrp	x9, l_switch.table.part_desc_for@PAGE
Lloh1:
	add	x9, x9, l_switch.table.part_desc_for@PAGEOFF
	ldr	x0, [x9, w8, uxtw #3]
	ret
LBB0_2:
	mov	x0, #0                          ; =0x0
	ret
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_part_identify                  ; -- Begin function part_identify
	.p2align	2
_part_identify:                         ; @part_identify
	.cfi_startproc
; %bb.0:
	mov	x8, x0
	mov	w0, #0                          ; =0x0
	cbz	x8, LBB1_3
; %bb.1:
	cmp	x1, #4
	b.lo	LBB1_3
; %bb.2:
	ldr	w8, [x8]
	mov	w9, #1                          ; =0x1
	mov	w10, #1301                      ; =0x515
	movk	w10, #59904, lsl #16
	mov	w11, #4                         ; =0x4
	mov	w12, #6533                      ; =0x1985
	sub	w13, w8, #440, lsl #12          ; =1802240
	cmp	w13, #1057
	mov	w13, #2                         ; =0x2
	cinc	w13, w13, eq
	cmp	w12, w8, uxth
	csel	w11, w11, w13, eq
	cmp	w8, w10
	csel	w0, w9, w11, eq
LBB1_3:
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_part_verify_layout             ; -- Begin function part_verify_layout
	.p2align	2
_part_verify_layout:                    ; @part_verify_layout
	.cfi_startproc
; %bb.0:
	lsr	x8, x0, #24
	cmp	x8, #0
	mov	w8, #2                          ; =0x2
	csel	w0, w8, wzr, eq
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_part_carve                     ; -- Begin function part_carve
	.p2align	2
_part_carve:                            ; @part_carve
	.cfi_startproc
; %bb.0:
	cmp	w2, #2
	b.gt	LBB3_4
; %bb.1:
	cmp	w2, #1
	b.eq	LBB3_8
; %bb.2:
	cmp	w2, #2
	b.ne	LBB3_7
; %bb.3:
	mov	w10, #0                         ; =0x0
Lloh2:
	adrp	x9, _g_parts@PAGE+24
Lloh3:
	add	x9, x9, _g_parts@PAGEOFF+24
	mov	w8, #1                          ; =0x1
	cbnz	x0, LBB3_11
	b	LBB3_9
LBB3_4:
	cmp	w2, #3
	b.eq	LBB3_10
; %bb.5:
	cmp	w2, #4
	b.ne	LBB3_7
; %bb.6:
	mov	w10, #0                         ; =0x0
Lloh4:
	adrp	x9, _g_parts@PAGE+72
Lloh5:
	add	x9, x9, _g_parts@PAGEOFF+72
	mov	w8, #1                          ; =0x1
	cbnz	x0, LBB3_11
	b	LBB3_9
LBB3_7:
	mov	x9, #0                          ; =0x0
	mov	w10, #1                         ; =0x1
	mov	w8, #1                          ; =0x1
	cbnz	x0, LBB3_11
	b	LBB3_9
LBB3_8:
	mov	w10, #0                         ; =0x0
Lloh6:
	adrp	x9, _g_parts@PAGE
Lloh7:
	add	x9, x9, _g_parts@PAGEOFF
	mov	w8, #1                          ; =0x1
	cbnz	x0, LBB3_11
LBB3_9:
	mov	x0, x8
	ret
LBB3_10:
	mov	w10, #0                         ; =0x0
Lloh8:
	adrp	x9, _g_parts@PAGE+48
Lloh9:
	add	x9, x9, _g_parts@PAGEOFF+48
	mov	w8, #1                          ; =0x1
	cbz	x0, LBB3_9
LBB3_11:
	cbz	x3, LBB3_9
; %bb.12:
	lsr	x8, x1, #24
	cmp	x8, #0
	csinc	w10, w10, wzr, ne
	mov	w8, #1                          ; =0x1
	cinc	w8, w8, eq
	tbnz	w10, #0, LBB3_9
; %bb.13:
	ldr	w2, [x9, #8]
	cmp	x4, x2
	b.hs	LBB3_15
; %bb.14:
	mov	w0, #2                          ; =0x2
	ret
LBB3_15:
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	mov	x29, sp
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	ldr	w8, [x9, #4]
	add	x1, x0, x8
	mov	x0, x3
	bl	_memcpy
	mov	w8, #0                          ; =0x0
	ldp	x29, x30, [sp], #16             ; 16-byte Folded Reload
	mov	x0, x8
	ret
	.loh AdrpAdd	Lloh2, Lloh3
	.loh AdrpAdd	Lloh4, Lloh5
	.loh AdrpAdd	Lloh6, Lloh7
	.loh AdrpAdd	Lloh8, Lloh9
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"boot"

l_.str.1:                               ; @.str.1
	.asciz	"bootargs"

l_.str.2:                               ; @.str.2
	.asciz	"kernel"

l_.str.3:                               ; @.str.3
	.asciz	"rootfs"

	.section	__DATA,__const
	.p2align	3, 0x0                          ; @g_parts
_g_parts:
	.long	1                               ; 0x1
	.long	0                               ; 0x0
	.long	131072                          ; 0x20000
	.space	4
	.quad	l_.str
	.long	2                               ; 0x2
	.long	131072                          ; 0x20000
	.long	65536                           ; 0x10000
	.space	4
	.quad	l_.str.1
	.long	3                               ; 0x3
	.long	196608                          ; 0x30000
	.long	1835008                         ; 0x1c0000
	.space	4
	.quad	l_.str.2
	.long	4                               ; 0x4
	.long	2031616                         ; 0x1f0000
	.long	14745600                        ; 0xe10000
	.space	4
	.quad	l_.str.3

	.p2align	3, 0x0                          ; @switch.table.part_desc_for
l_switch.table.part_desc_for:
	.quad	_g_parts
	.quad	_g_parts+24
	.quad	_g_parts+48
	.quad	_g_parts+72

.subsections_via_symbols
```

## 5. Tests

```
1001:static void run_partition_tests(void) {
1018:static void run_partition_carve_tests(void) {
1177:    run_partition_tests();
1178:    run_partition_carve_tests();
```

## 6. Labs

1. Read the header, then the source; explain each function.
2. Match each C function to its assembly block.
3. Run the tests for this module: `python3 scripts/run_tests.py | grep partition`.
4. State one way this module could be misused and one way it is defended.
