# Module 03: env

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

See `README.md` for where `env` sits in the lab. This volume is the whole module.

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
// File:    env.h
// Desc:    Declares the U-Boot environment helpers. The environment partition
//          begins with a little-endian CRC32 of the remaining bytes, followed by
//          a NUL-separated key=value blob.
// Created: 2026

#ifndef ENV_H
#define ENV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Size of the leading environment CRC field in bytes.
 */
#define ENV_CRC_SIZE 4u

/**
 * @brief Compute the environment CRC32 over the key/value blob.
 *
 * The blob begins after the four-byte CRC field and runs to the end of the
 * supplied buffer.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @return uint32_t Computed standard CRC-32 over the blob.
 */
uint32_t env_compute_crc(const uint8_t *env, size_t len);

/**
 * @brief Verify the leading environment CRC32.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @return bool true when the stored CRC matches the computed value.
 */
bool env_crc_valid(const uint8_t *env, size_t len);

/**
 * @brief Find a key in the environment blob and return its value.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @param key NUL-terminated key to locate.
 * @return const char* Pointer to the value, or NULL when absent.
 */
const char *env_find(const uint8_t *env, size_t len, const char *key);

#endif // ENV_H
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
// File:    env.c
// Desc:    Implements the U-Boot environment helpers: CRC computation, CRC
//          verification, and key lookup.
// Created: 2026

#include "env.h"
#include "crc.h"
#include <string.h>

/**
 * @brief Read a 32-bit little-endian word from bytes.
 *
 * @param src Pointer to four readable bytes.
 * @return uint32_t Parsed 32-bit word.
 */
static uint32_t env_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}

/**
 * @brief Compute the environment CRC32 over the key/value blob.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @return uint32_t Computed standard CRC-32 over the blob.
 */
uint32_t env_compute_crc(const uint8_t *env, size_t len) {
    if ((env == NULL) || (len < ENV_CRC_SIZE)) return 0u;
    return crc32_uboot(&env[ENV_CRC_SIZE], len - ENV_CRC_SIZE);
}

/**
 * @brief Verify the leading environment CRC32.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @return bool true when the stored CRC matches the computed value.
 */
bool env_crc_valid(const uint8_t *env, size_t len) {
    if ((env == NULL) || (len < ENV_CRC_SIZE)) return false;
    return env_read32(env) == env_compute_crc(env, len);
}

/**
 * @brief Find a key in the environment blob and return its value.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @param key NUL-terminated key to locate.
 * @return const char* Pointer to the value, or NULL when absent.
 */
const char *env_find(const uint8_t *env, size_t len, const char *key) {
    size_t klen;
    size_t i;
    if ((env == NULL) || (key == NULL) || (len < ENV_CRC_SIZE)) return NULL;
    klen = strlen(key);
    for (i = ENV_CRC_SIZE; (i + klen + 1u) < len; ++i) {
        if ((env[i + klen] == '=') && (memcmp(&env[i], key, klen) == 0)) {
            return (const char *)&env[i + klen + 1u];
        }
    }
    return NULL;
}
```

## 4. AArch64 Disassembly

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_env_compute_crc                ; -- Begin function env_compute_crc
	.p2align	2
_env_compute_crc:                       ; @env_compute_crc
	.cfi_startproc
; %bb.0:
	cbz	x0, LBB0_3
; %bb.1:
	subs	x1, x1, #4
	b.lo	LBB0_3
; %bb.2:
	add	x0, x0, #4
	b	_crc32_uboot
LBB0_3:
	mov	w0, #0                          ; =0x0
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_env_crc_valid                  ; -- Begin function env_crc_valid
	.p2align	2
_env_crc_valid:                         ; @env_crc_valid
	.cfi_startproc
; %bb.0:
	mov	x8, x0
	mov	w0, #0                          ; =0x0
	cbz	x8, LBB1_3
; %bb.1:
	subs	x1, x1, #4
	b.lo	LBB1_3
; %bb.2:
	stp	x20, x19, [sp, #-32]!           ; 16-byte Folded Spill
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	ldr	w19, [x8], #4
	mov	x0, x8
	bl	_crc32_uboot
	cmp	w19, w0
	cset	w0, eq
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp], #32             ; 16-byte Folded Reload
LBB1_3:
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_env_find                       ; -- Begin function env_find
	.p2align	2
_env_find:                              ; @env_find
	.cfi_startproc
; %bb.0:
	stp	x22, x21, [sp, #-48]!           ; 16-byte Folded Spill
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	x20, x0
	mov	x0, #0                          ; =0x0
	cmp	x1, #4
	b.lo	LBB2_10
; %bb.1:
	cbz	x20, LBB2_10
; %bb.2:
	mov	x19, x2
	cbz	x2, LBB2_10
; %bb.3:
	mov	x22, x1
	mov	x0, x19
	bl	_strlen
	add	x8, x0, #5
	cmp	x8, x22
	b.hs	LBB2_9
; %bb.4:
	mov	x21, x0
	sub	x22, x22, #5
	b	LBB2_6
LBB2_5:                                 ;   in Loop: Header=BB2_6 Depth=1
	add	x20, x20, #1
	sub	x22, x22, #1
	cmp	x21, x22
	b.eq	LBB2_9
LBB2_6:                                 ; =>This Inner Loop Header: Depth=1
	add	x8, x20, x21
	ldrb	w8, [x8, #4]
	cmp	w8, #61
	b.ne	LBB2_5
; %bb.7:                                ;   in Loop: Header=BB2_6 Depth=1
	add	x0, x20, #4
	mov	x1, x19
	mov	x2, x21
	bl	_memcmp
	cbnz	w0, LBB2_5
; %bb.8:
	add	x8, x20, x21
	add	x0, x8, #5
	b	LBB2_10
LBB2_9:
	mov	x0, #0                          ; =0x0
LBB2_10:
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp], #48             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```

## 5. Tests

```
451:static size_t env_build(const char *blob) {
464:static void env_seal(size_t n) {
521:static void test_env_crc_valid(void) {
522:    size_t n = env_build("bootargs=console=ttyAMA0");
523:    env_seal(n);
524:    TEST_ASSERT_TRUE(env_crc_valid(g_env, n));
525:    TEST_ASSERT_EQUAL_STRING("console=ttyAMA0", env_find(g_env, n, "bootargs"));
534:static void test_env_crc_invalid(void) {
535:    size_t n = env_build("bootdelay=1");
536:    TEST_ASSERT_FALSE(env_crc_valid(g_env, n));
537:    TEST_ASSERT_TRUE(env_find(g_env, n, "missing") == NULL);
546:static void test_env_short(void) {
547:    TEST_ASSERT_EQUAL_UINT(0u, env_compute_crc(g_image, 2u));
548:    TEST_ASSERT_FALSE(env_crc_valid(g_image, 2u));
549:    TEST_ASSERT_TRUE(env_find(NULL, 8u, "x") == NULL);
1043:static void run_env_tests(void) {
1044:    RUN_TEST(test_env_crc_valid);
1045:    RUN_TEST(test_env_crc_invalid);
1046:    RUN_TEST(test_env_short);
1150:    run_env_tests();
```

## 6. Labs

1. Read the header, then the source; explain each function.
2. Match each C function to its assembly block.
3. Run the tests for this module: `python3 scripts/run_tests.py | grep env`.
4. State one way this module could be misused and one way it is defended.
