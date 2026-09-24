# Module 01: crc

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

See `README.md` for where `crc` sits in the lab. This volume is the whole module.

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

/**
 * @brief Compute the reflected crc32_le checksum used by JFFS2.
 *
 * @param seed Initial checksum value.
 * @param buf Pointer to readable bytes.
 * @param len Number of bytes to checksum.
 * @return uint32_t Computed reflected CRC-32 residual.
 */
uint32_t crc32_le(uint32_t seed, const uint8_t *buf, size_t len) {
    uint32_t crc = seed;
    size_t i;
    for (i = 0u; i < len; ++i) crc = crc32_byte(crc, buf[i]);
    return crc;
}

/**
 * @brief Compute the standard CRC32 used by the U-Boot environment.
 *
 * @param buf Pointer to readable bytes.
 * @param len Number of bytes to checksum.
 * @return uint32_t Computed standard CRC-32 value.
 */
uint32_t crc32_uboot(const uint8_t *buf, size_t len) {
    return crc32_le(0xFFFFFFFFu, buf, len) ^ 0xFFFFFFFFu;
}

/**
 * @brief Compute the CRC-16/CCITT-FALSE checksum of a byte buffer.
 *
 * @param buf Pointer to readable bytes.
 * @param len Number of bytes to checksum.
 * @return uint16_t Computed CRC-16 residual value.
 */
uint16_t crc16_ccitt(const uint8_t *buf, size_t len) {
    uint16_t crc = CRC16_INIT;
    size_t i;
    for (i = 0u; i < len; ++i) crc = crc16_byte(crc, buf[i]);
    return crc;
}
```

## 4. AArch64 Disassembly

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__literal16,16byte_literals
	.p2align	4, 0x0                          ; -- Begin function crc32_le
lCPI0_0:
	.long	8                               ; 0x8
	.long	4                               ; 0x4
	.long	16                              ; 0x10
	.long	32                              ; 0x20
lCPI0_1:
	.long	249268274                       ; 0xedb8832
	.long	124634137                       ; 0x76dc419
	.long	498536548                       ; 0x1db71064
	.long	997073096                       ; 0x3b6e20c8
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_crc32_le
	.p2align	2
_crc32_le:                              ; @crc32_le
	.cfi_startproc
; %bb.0:
	cbz	x2, LBB0_3
; %bb.1:
	mov	w8, #33568                      ; =0x8320
	movk	w8, #60856, lsl #16
Lloh0:
	adrp	x9, lCPI0_0@PAGE
Lloh1:
	ldr	q0, [x9, lCPI0_0@PAGEOFF]
Lloh2:
	adrp	x9, lCPI0_1@PAGE
Lloh3:
	ldr	q1, [x9, lCPI0_1@PAGEOFF]
	mov	w9, #16784                      ; =0x4190
	movk	w9, #30428, lsl #16
LBB0_2:                                 ; =>This Inner Loop Header: Depth=1
	ldrb	w10, [x1], #1
	eor	w10, w0, w10
	sbfx	w11, w10, #0, #1
	and	w11, w11, w8
	eor	w11, w11, w10, lsr #1
	lsl	w12, w10, #30
	and	w12, w8, w12, asr #31
	eor	w12, w12, w11, lsr #1
	dup.4s	v2, w10
	and.16b	v2, v2, v0
	cmeq.4s	v2, v2, #0
	bic.16b	v2, v1, v2
	lsl	w10, w11, #26
	and	w10, w9, w10, asr #31
	lsl	w11, w12, #26
	and	w11, w8, w11, asr #31
	ext.16b	v3, v2, v2, #8
	eor.8b	v2, v2, v3
	fmov	x13, d2
	lsr	x14, x13, #32
	eor	w10, w13, w10
	eor	w10, w10, w14
	eor	w11, w11, w12, lsr #6
	eor	w0, w10, w11
	subs	x2, x2, #1
	b.ne	LBB0_2
LBB0_3:
	ret
	.loh AdrpLdr	Lloh2, Lloh3
	.loh AdrpAdrp	Lloh0, Lloh2
	.loh AdrpLdr	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__literal16,16byte_literals
	.p2align	4, 0x0                          ; -- Begin function crc32_uboot
lCPI1_0:
	.long	8                               ; 0x8
	.long	4                               ; 0x4
	.long	16                              ; 0x10
	.long	32                              ; 0x20
lCPI1_1:
	.long	249268274                       ; 0xedb8832
	.long	124634137                       ; 0x76dc419
	.long	498536548                       ; 0x1db71064
	.long	997073096                       ; 0x3b6e20c8
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_crc32_uboot
	.p2align	2
_crc32_uboot:                           ; @crc32_uboot
	.cfi_startproc
; %bb.0:
	cbz	x1, LBB1_4
; %bb.1:
	mov	w10, #-1                        ; =0xffffffff
	mov	w8, #33568                      ; =0x8320
	movk	w8, #60856, lsl #16
Lloh4:
	adrp	x9, lCPI1_0@PAGE
Lloh5:
	ldr	q0, [x9, lCPI1_0@PAGEOFF]
Lloh6:
	adrp	x9, lCPI1_1@PAGE
Lloh7:
	ldr	q1, [x9, lCPI1_1@PAGEOFF]
	mov	w9, #16784                      ; =0x4190
	movk	w9, #30428, lsl #16
LBB1_2:                                 ; =>This Inner Loop Header: Depth=1
	ldrb	w11, [x0], #1
	eor	w10, w10, w11
	sbfx	w11, w10, #0, #1
	and	w11, w11, w8
	eor	w11, w11, w10, lsr #1
	lsl	w12, w10, #30
	and	w12, w8, w12, asr #31
	eor	w12, w12, w11, lsr #1
	dup.4s	v2, w10
	and.16b	v2, v2, v0
	cmeq.4s	v2, v2, #0
	bic.16b	v2, v1, v2
	lsl	w10, w11, #26
	and	w10, w9, w10, asr #31
	lsl	w11, w12, #26
	and	w11, w8, w11, asr #31
	ext.16b	v3, v2, v2, #8
	eor.8b	v2, v2, v3
	fmov	x13, d2
	lsr	x14, x13, #32
	eor	w10, w13, w10
	eor	w10, w10, w14
	eor	w11, w11, w12, lsr #6
	eor	w10, w10, w11
	subs	x1, x1, #1
	b.ne	LBB1_2
; %bb.3:
	mvn	w0, w10
	ret
LBB1_4:
	mov	w0, #0                          ; =0x0
	ret
	.loh AdrpLdr	Lloh6, Lloh7
	.loh AdrpAdrp	Lloh4, Lloh6
	.loh AdrpLdr	Lloh4, Lloh5
	.cfi_endproc
                                        ; -- End function
	.globl	_crc16_ccitt                    ; -- Begin function crc16_ccitt
	.p2align	2
_crc16_ccitt:                           ; @crc16_ccitt
	.cfi_startproc
; %bb.0:
	mov	w9, #65535                      ; =0xffff
	cbz	x1, LBB2_3
; %bb.1:
	mov	w8, #4129                       ; =0x1021
LBB2_2:                                 ; =>This Inner Loop Header: Depth=1
	ldrb	w10, [x0], #1
	eor	w11, w9, w10, lsl #8
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w11, w9, w10, lsl #9
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w11, w9, w10, lsl #10
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w11, w9, w10, lsl #11
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w11, w9, w10, lsl #12
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w11, w9, w10, lsl #13
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w11, w9, w10, lsl #14
	sxth	w11, w11
	lsl	w9, w9, #1
	eor	w12, w9, w8
	cmp	w11, #0
	csel	w9, w12, w9, lt
	eor	w10, w9, w10, lsl #15
	sxth	w10, w10
	lsl	w9, w9, #1
	eor	w11, w9, w8
	cmp	w10, #0
	csel	w9, w11, w9, lt
	subs	x1, x1, #1
	b.ne	LBB2_2
LBB2_3:
	and	w0, w9, #0xffff
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```

## 5. Tests

```
489:static void test_crc32_uboot_check(void) {
500:static void test_crc32_le_empty(void) {
510:static void test_crc16_check(void) {
521:static void test_env_crc_valid(void) {
524:    TEST_ASSERT_TRUE(env_crc_valid(g_env, n));
534:static void test_env_crc_invalid(void) {
536:    TEST_ASSERT_FALSE(env_crc_valid(g_env, n));
548:    TEST_ASSERT_FALSE(env_crc_valid(g_image, 2u));
573:    TEST_ASSERT_TRUE(jffs2_hdr_crc_valid(g_node));
575:    TEST_ASSERT_FALSE(jffs2_hdr_crc_valid(g_node));
1031:static void run_crc_tests(void) {
1032:    RUN_TEST(test_crc32_uboot_check);
1033:    RUN_TEST(test_crc32_le_empty);
1034:    RUN_TEST(test_crc16_check);
1044:    RUN_TEST(test_env_crc_valid);
1045:    RUN_TEST(test_env_crc_invalid);
1149:    run_crc_tests();
```

## 6. Labs

1. Read the header, then the source; explain each function.
2. Match each C function to its assembly block.
3. Run the tests for this module: `python3 scripts/run_tests.py | grep crc`.
4. State one way this module could be misused and one way it is defended.
