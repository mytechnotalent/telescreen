# Module 05: jffs2

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

See `README.md` for where `jffs2` sits in the lab. This volume is the whole module.

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
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    jffs2.h
// Desc:    Declares the JFFS2 node header helpers for the TELESCREEN rootfs:
//          magic, node type, total length, and the crc32_le-protected header CRC.
// Created: 2026

#ifndef JFFS2_H
#define JFFS2_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief JFFS2 little-endian magic value.
 */
#define JFFS2_MAGIC 0x1985u

/**
 * @brief Common JFFS2 node header size in bytes.
 */
#define JFFS2_HDR_SIZE 12u

/**
 * @brief JFFS2 DIRENT node type.
 */
#define JFFS2_TYPE_DIRENT 0xE001u

/**
 * @brief JFFS2 INODE node type.
 */
#define JFFS2_TYPE_INODE 0xE002u

/**
 * @brief JFFS2 CLEANMARKER node type.
 */
#define JFFS2_TYPE_CLEANMARKER 0x2003u

/**
 * @brief JFFS2 PADDING node type.
 */
#define JFFS2_TYPE_PADDING 0x2004u

/**
 * @brief Read the little-endian magic from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint16_t Parsed magic value.
 */
uint16_t jffs2_magic(const uint8_t *node);

/**
 * @brief Read the little-endian node type from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint16_t Parsed node type.
 */
uint16_t jffs2_type(const uint8_t *node);

/**
 * @brief Read the little-endian total length from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint32_t Parsed total length in bytes.
 */
uint32_t jffs2_totlen(const uint8_t *node);

/**
 * @brief Verify the crc32_le header CRC of a node.
 *
 * The stored header CRC covers the first eight bytes of the node.
 *
 * @param node Pointer to the node header bytes.
 * @return bool true when the stored header CRC matches the computed value.
 */
bool jffs2_hdr_crc_valid(const uint8_t *node);

#endif // JFFS2_H
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
// File:    jffs2.c
// Desc:    Implements the JFFS2 node header helpers for the TELESCREEN rootfs.
// Created: 2026

#include "jffs2.h"
#include "crc.h"

/**
 * @brief Read a 16-bit little-endian value from bytes.
 *
 * @param src Pointer to two readable bytes.
 * @return uint16_t Parsed 16-bit value.
 */
static uint16_t jffs2_read16(const uint8_t *src) {
    return (uint16_t)((uint16_t)src[0] | ((uint16_t)src[1] << 8u));
}

/**
 * @brief Read a 32-bit little-endian word from bytes.
 *
 * @param src Pointer to four readable bytes.
 * @return uint32_t Parsed 32-bit word.
 */
static uint32_t jffs2_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}

/**
 * @brief Read the little-endian magic from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint16_t Parsed magic value.
 */
uint16_t jffs2_magic(const uint8_t *node) {
    return jffs2_read16(node);
}

/**
 * @brief Read the little-endian node type from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint16_t Parsed node type.
 */
uint16_t jffs2_type(const uint8_t *node) {
    return jffs2_read16(&node[2]);
}

/**
 * @brief Read the little-endian total length from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint32_t Parsed total length in bytes.
 */
uint32_t jffs2_totlen(const uint8_t *node) {
    return jffs2_read32(&node[4]);
}

/**
 * @brief Verify the crc32_le header CRC of a node.
 *
 * @param node Pointer to the node header bytes.
 * @return bool true when the stored header CRC matches the computed value.
 */
bool jffs2_hdr_crc_valid(const uint8_t *node) {
    return crc32_le(0u, node, 8u) == jffs2_read32(&node[8]);
}
```

## 4. AArch64 Disassembly

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_jffs2_magic                    ; -- Begin function jffs2_magic
	.p2align	2
_jffs2_magic:                           ; @jffs2_magic
	.cfi_startproc
; %bb.0:
	ldrh	w0, [x0]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_jffs2_type                     ; -- Begin function jffs2_type
	.p2align	2
_jffs2_type:                            ; @jffs2_type
	.cfi_startproc
; %bb.0:
	ldrh	w0, [x0, #2]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_jffs2_totlen                   ; -- Begin function jffs2_totlen
	.p2align	2
_jffs2_totlen:                          ; @jffs2_totlen
	.cfi_startproc
; %bb.0:
	ldr	w0, [x0, #4]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_jffs2_hdr_crc_valid            ; -- Begin function jffs2_hdr_crc_valid
	.p2align	2
_jffs2_hdr_crc_valid:                   ; @jffs2_hdr_crc_valid
	.cfi_startproc
; %bb.0:
	stp	x20, x19, [sp, #-32]!           ; 16-byte Folded Spill
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x0
	mov	w0, #0                          ; =0x0
	mov	x1, x19
	mov	w2, #8                          ; =0x8
	bl	_crc32_le
	ldr	w8, [x19, #8]
	cmp	w0, w8
	cset	w0, eq
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp], #32             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```

## 5. Tests

```
558:static void test_jffs2_header(void) {
560:    TEST_ASSERT_EQUAL_HEX16(JFFS2_MAGIC, jffs2_magic(g_node));
561:    TEST_ASSERT_EQUAL_HEX16(JFFS2_TYPE_CLEANMARKER, jffs2_type(g_node));
562:    TEST_ASSERT_EQUAL_UINT(12u, jffs2_totlen(g_node));
571:static void test_jffs2_crc(void) {
573:    TEST_ASSERT_TRUE(jffs2_hdr_crc_valid(g_node));
575:    TEST_ASSERT_FALSE(jffs2_hdr_crc_valid(g_node));
1055:static void run_jffs2_tests(void) {
1056:    RUN_TEST(test_jffs2_header);
1057:    RUN_TEST(test_jffs2_crc);
1160:    run_jffs2_tests();
```

## 6. Labs

1. Read the header, then the source; explain each function.
2. Match each C function to its assembly block.
3. Run the tests for this module: `python3 scripts/run_tests.py | grep jffs2`.
4. State one way this module could be misused and one way it is defended.
