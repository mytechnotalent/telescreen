# Module 04: container

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

See `README.md` for where `container` sits in the lab. This volume is the whole module.

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

/**
 * @brief Read the container magic from the header.
 *
 * @param buf Pointer to the container header bytes.
 * @return uint32_t Parsed magic value.
 */
uint32_t container_magic(const uint8_t *buf) {
    return cont_read32(buf);
}

/**
 * @brief Read the container length field from the header.
 *
 * @param buf Pointer to the container header bytes.
 * @return uint32_t Parsed length value.
 */
uint32_t container_length(const uint8_t *buf) {
    return cont_read32(&buf[4]);
}

/**
 * @brief Verify the "gziphead" tag in the container header.
 *
 * @param buf Pointer to the container header bytes.
 * @return bool true when the tag is present.
 */
bool container_tag_ok(const uint8_t *buf) {
    return memcmp(&buf[8], "gziphead", 8u) == 0;
}

/**
 * @brief Validate a vendor kernel container header.
 *
 * @param buf Pointer to the container bytes.
 * @param len Number of readable bytes.
 * @return bool true when the magic and tag both validate.
 */
bool container_valid(const uint8_t *buf, size_t len) {
    if ((buf == NULL) || (len < CONTAINER_HDR_SIZE)) return false;
    return (container_magic(buf) == CONTAINER_MAGIC) && container_tag_ok(buf);
}
```

## 4. AArch64 Disassembly

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_container_magic                ; -- Begin function container_magic
	.p2align	2
_container_magic:                       ; @container_magic
	.cfi_startproc
; %bb.0:
	ldr	w0, [x0]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_container_length               ; -- Begin function container_length
	.p2align	2
_container_length:                      ; @container_length
	.cfi_startproc
; %bb.0:
	ldr	w0, [x0, #4]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_container_tag_ok               ; -- Begin function container_tag_ok
	.p2align	2
_container_tag_ok:                      ; @container_tag_ok
	.cfi_startproc
; %bb.0:
	ldr	x8, [x0, #8]
	mov	x9, #31335                      ; =0x7a67
	movk	x9, #28777, lsl #16
	movk	x9, #25960, lsl #32
	movk	x9, #25697, lsl #48
	cmp	x8, x9
	cset	w0, eq
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_container_valid                ; -- Begin function container_valid
	.p2align	2
_container_valid:                       ; @container_valid
	.cfi_startproc
; %bb.0:
	mov	x8, x0
	mov	w0, #0                          ; =0x0
	cbz	x8, LBB3_4
; %bb.1:
	cmp	x1, #16
	b.lo	LBB3_4
; %bb.2:
	ldr	w9, [x8]
	sub	w9, w9, #440, lsl #12           ; =1802240
	cmp	w9, #1057
	b.ne	LBB3_5
; %bb.3:
	ldr	x8, [x8, #8]
	mov	x9, #31335                      ; =0x7a67
	movk	x9, #28777, lsl #16
	movk	x9, #25960, lsl #32
	movk	x9, #25697, lsl #48
	cmp	x8, x9
	cset	w0, eq
LBB3_4:
	ret
LBB3_5:
	mov	w0, #0                          ; =0x0
	ret
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"gziphead"

.subsections_via_symbols
```

## 5. Tests

```
607:static void test_container_fields(void) {
609:    TEST_ASSERT_EQUAL_UINT(CONTAINER_MAGIC, container_magic(g_cont));
610:    TEST_ASSERT_EQUAL_UINT(0x002CF000u, container_length(g_cont));
611:    TEST_ASSERT_TRUE(container_tag_ok(g_cont));
612:    TEST_ASSERT_TRUE(container_valid(g_cont, sizeof(g_cont)));
621:static void test_container_invalid(void) {
622:    TEST_ASSERT_FALSE(container_valid(NULL, CONTAINER_HDR_SIZE));
623:    TEST_ASSERT_FALSE(container_valid(g_cont, 8u));
626:    TEST_ASSERT_FALSE(container_valid(g_cont, sizeof(g_cont)));
1066:static void run_container_tests(void) {
1067:    RUN_TEST(test_container_fields);
1068:    RUN_TEST(test_container_invalid);
1161:    run_container_tests();
```

## 6. Labs

1. Read the header, then the source; explain each function.
2. Match each C function to its assembly block.
3. Run the tests for this module: `python3 scripts/run_tests.py | grep container`.
4. State one way this module could be misused and one way it is defended.
