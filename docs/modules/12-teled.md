# Module 12: teled

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

See `README.md` for where `teled` sits in the lab. This volume is the whole module.

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
// File:    teled.h
// Desc:    Declares the TELESCREEN daemon. It owns the device identity, the
//          hardened session key, and the beacon sequence, and hands sealed
//          beacons to the local collector.
// Created: 2026

#ifndef TELED_H
#define TELED_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Maximum device UID length stored by the daemon.
 */
#define TELED_UID_MAX 32u

/**
 * @brief TELESCREEN daemon state.
 */
typedef struct teled_state {
    /**
     * @brief NUL-terminated device UID.
     */
    char uid[TELED_UID_MAX];
    /**
     * @brief Hardened session key.
     */
    uint8_t key[32];
    /**
     * @brief Monotonic beacon sequence number.
     */
    uint32_t seq;
    /**
     * @brief Number of beacons sealed by the daemon.
     */
    uint32_t beacons;
} teled_state_t;

/**
 * @brief Initialize the daemon with a device UID and session key.
 *
 * @param uid NUL-terminated public device identifier.
 * @param key Pointer to 32-byte hardened session key.
 * @return void
 */
void teled_init(const char *uid, const uint8_t key[32]);

/**
 * @brief Seal and deliver one beacon to the local collector.
 *
 * @param payload Pointer to AEAD payload bytes.
 * @return bool true when the beacon is sealed and accepted.
 */
bool teled_beacon(const uint8_t *payload);

/**
 * @brief Return a pointer to the daemon state snapshot.
 *
 * @param void No parameters.
 * @return const teled_state_t* Pointer to the static state.
 */
const teled_state_t *teled_state(void);

#endif // TELED_H
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
// File:    teled.c
// Desc:    Implements the TELESCREEN daemon: identity, session key, beacon
//          sequence, and delivery to the local collector.
// Created: 2026

#include "teled.h"
#include "aead.h"
#include "beacon.h"
#include "collector.h"
#include <string.h>

/**
 * @brief TELESCREEN daemon state.
 */
static teled_state_t g_teled;

/**
 * @brief Initialize the daemon with a device UID and session key.
 *
 * @param uid NUL-terminated public device identifier.
 * @param key Pointer to 32-byte hardened session key.
 * @return void
 */
void teled_init(const char *uid, const uint8_t key[32]) {
    memset(&g_teled, 0, sizeof(g_teled));
    strncpy(g_teled.uid, uid, sizeof(g_teled.uid) - 1u);
    memcpy(g_teled.key, key, 32u);
    collector_init();
}

/**
 * @brief Seal and deliver one beacon to the local collector.
 *
 * @param payload Pointer to AEAD payload bytes.
 * @return bool true when the beacon is sealed and accepted.
 */
bool teled_beacon(const uint8_t *payload) {
    uint8_t sealed[AEAD_SEALED_SIZE];
    g_teled.seq += 1u;
    if (!beacon_seal(g_teled.key, g_teled.seq, payload, sealed)) return false;
    g_teled.beacons += 1u;
    return collector_accept(sealed, sizeof(sealed), g_teled.seq);
}

/**
 * @brief Return a pointer to the daemon state snapshot.
 *
 * @param void No parameters.
 * @return const teled_state_t* Pointer to the static state.
 */
const teled_state_t *teled_state(void) {
    return &g_teled;
}
```

## 4. AArch64 Disassembly

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_teled_init                     ; -- Begin function teled_init
	.p2align	2
_teled_init:                            ; @teled_init
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
	mov	x19, x1
	mov	x1, x0
Lloh0:
	adrp	x0, _g_teled@PAGE
Lloh1:
	add	x0, x0, _g_teled@PAGEOFF
	str	xzr, [x0, #64]
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [x0, #32]
	stp	q0, q0, [x0]
	mov	w2, #31                         ; =0x1f
	bl	_strncpy
	ldp	q0, q1, [x19]
	stp	q0, q1, [x0, #32]
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp], #32             ; 16-byte Folded Reload
	b	_collector_init
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_teled_beacon                   ; -- Begin function teled_beacon
	.p2align	2
_teled_beacon:                          ; @teled_beacon
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #96
	stp	x29, x30, [sp, #80]             ; 16-byte Folded Spill
	add	x29, sp, #80
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	mov	x2, x0
Lloh2:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh3:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh4:
	ldr	x8, [x8]
	stur	x8, [x29, #-8]
Lloh5:
	adrp	x0, _g_teled@PAGE+32
Lloh6:
	add	x0, x0, _g_teled@PAGEOFF+32
	ldr	w8, [x0, #32]
	add	w1, w8, #1
	str	w1, [x0, #32]
	add	x3, sp, #8
	bl	_beacon_seal
	cbz	w0, LBB1_2
; %bb.1:
Lloh7:
	adrp	x8, _g_teled@PAGE+64
Lloh8:
	add	x8, x8, _g_teled@PAGEOFF+64
	ldp	w2, w9, [x8]
	add	w9, w9, #1
	str	w9, [x8, #4]
	add	x0, sp, #8
	mov	w1, #64                         ; =0x40
	bl	_collector_accept
LBB1_2:
	ldur	x8, [x29, #-8]
Lloh9:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh10:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh11:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB1_4
; %bb.3:
	ldp	x29, x30, [sp, #80]             ; 16-byte Folded Reload
	add	sp, sp, #96
	ret
LBB1_4:
	bl	___stack_chk_fail
	.loh AdrpAdd	Lloh5, Lloh6
	.loh AdrpLdrGotLdr	Lloh2, Lloh3, Lloh4
	.loh AdrpAdd	Lloh7, Lloh8
	.loh AdrpLdrGotLdr	Lloh9, Lloh10, Lloh11
	.cfi_endproc
                                        ; -- End function
	.globl	_teled_state                    ; -- Begin function teled_state
	.p2align	2
_teled_state:                           ; @teled_state
	.cfi_startproc
; %bb.0:
Lloh12:
	adrp	x0, _g_teled@PAGE
Lloh13:
	add	x0, x0, _g_teled@PAGEOFF
	ret
	.loh AdrpAdd	Lloh12, Lloh13
	.cfi_endproc
                                        ; -- End function
.zerofill __DATA,__bss,_g_teled,72,2    ; @g_teled
.subsections_via_symbols
```

## 5. Tests

```
906:static void test_teled_beacon(void) {
910:    teled_init("SSAT-468547-FEEBD", key);
911:    TEST_ASSERT_TRUE(teled_beacon(g_pt));
912:    TEST_ASSERT_EQUAL_UINT(1u, teled_state()->beacons);
913:    TEST_ASSERT_EQUAL_STRING("SSAT-468547-FEEBD", teled_state()->uid);
922:static void test_teled_beacon_fail(void) {
925:    teled_init("uid", key);
926:    TEST_ASSERT_FALSE(teled_beacon(NULL));
1123:static void run_teled_tests(void) {
1124:    RUN_TEST(test_teled_beacon);
1125:    RUN_TEST(test_teled_beacon_fail);
1175:    run_teled_tests();
```

## 6. Labs

1. Read the header, then the source; explain each function.
2. Match each C function to its assembly block.
3. Run the tests for this module: `python3 scripts/run_tests.py | grep teled`.
4. State one way this module could be misused and one way it is defended.
