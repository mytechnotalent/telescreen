# Module 07: beacon

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

See `README.md` for where `beacon` sits in the lab. This volume is the whole module.

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
// File:    beacon.h
// Desc:    Declares the TELESCREEN exfiltration beacon. The Ministry derives its
//          key from the public device UID, which is obfuscation rather than
//          encryption. The hardened path uses the AEAD API instead.
// Created: 2026

#ifndef BEACON_H
#define BEACON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Beacon key size in bytes.
 */
#define BEACON_KEY_SIZE 32u

/**
 * @brief Beacon sequence number size in bytes.
 */
#define BEACON_SEQ_SIZE 4u

/**
 * @brief Derive the Ministry "sealed" key from the public device UID.
 *
 * This is deliberately weak: the key is a deterministic function of the public
 * UID, so anyone who knows the UID can recompute it. It exists to demonstrate
 * the failure, not to protect anything.
 *
 * @param uid NUL-terminated public device identifier.
 * @param out Output 32-byte key buffer.
 * @return void
 */
void beacon_weak_key(const char *uid, uint8_t out[BEACON_KEY_SIZE]);

/**
 * @brief Build the beacon nonce from the sequence number.
 *
 * @param seq Monotonic sequence number.
 * @param out Output nonce buffer of AEAD nonce size.
 * @return void
 */
void beacon_nonce(uint32_t seq, uint8_t *out);

/**
 * @brief Seal one beacon payload with the hardened AEAD key.
 *
 * @param key Pointer to 32-byte session key.
 * @param seq Monotonic sequence number.
 * @param plaintext Pointer to AEAD payload bytes.
 * @param sealed_out Output AEAD sealed buffer.
 * @return bool true when sealing succeeds.
 */
bool beacon_seal(const uint8_t key[BEACON_KEY_SIZE], uint32_t seq,
                 const uint8_t *plaintext, uint8_t *sealed_out);

#endif // BEACON_H
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
// File:    beacon.c
// Desc:    Implements the TELESCREEN exfiltration beacon: the deliberately weak
//          UID-derived key, the sequence nonce, and the hardened AEAD seal.
// Created: 2026

#include "beacon.h"
#include "aead.h"
#include "crc.h"
#include <string.h>

/**
 * @brief Derive the Ministry "sealed" key from the public device UID.
 *
 * @param uid NUL-terminated public device identifier.
 * @param out Output 32-byte key buffer.
 * @return void
 */
void beacon_weak_key(const char *uid, uint8_t out[BEACON_KEY_SIZE]) {
    size_t ulen = strlen(uid);
    uint32_t seed = crc32_le(0u, (const uint8_t *)uid, ulen);
    size_t i;
    for (i = 0u; i < BEACON_KEY_SIZE; ++i) {
        seed = crc32_le(seed, (const uint8_t *)uid, ulen);
        out[i] = (uint8_t)(seed & 0xFFu);
    }
}

/**
 * @brief Build the beacon nonce from the sequence number.
 *
 * @param seq Monotonic sequence number.
 * @param out Output nonce buffer of AEAD nonce size.
 * @return void
 */
void beacon_nonce(uint32_t seq, uint8_t *out) {
    out[0] = (uint8_t)(seq & 0xFFu);
    out[1] = (uint8_t)((seq >> 8u) & 0xFFu);
    out[2] = (uint8_t)((seq >> 16u) & 0xFFu);
    out[3] = (uint8_t)((seq >> 24u) & 0xFFu);
    memset(&out[4], 0, 20u);
}

/**
 * @brief Seal one beacon payload with the hardened AEAD key.
 *
 * @param key Pointer to 32-byte session key.
 * @param seq Monotonic sequence number.
 * @param plaintext Pointer to AEAD payload bytes.
 * @param sealed_out Output AEAD sealed buffer.
 * @return bool true when sealing succeeds.
 */
bool beacon_seal(const uint8_t key[BEACON_KEY_SIZE], uint32_t seq,
                 const uint8_t *plaintext, uint8_t *sealed_out) {
    uint8_t nonce[AEAD_XCHACHA_NONCE_SIZE];
    beacon_nonce(seq, nonce);
    return aead_seal(key, nonce, plaintext, sealed_out) == AEAD_RESULT_SUCCESS;
}
```

## 4. AArch64 Disassembly

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_beacon_weak_key                ; -- Begin function beacon_weak_key
	.p2align	2
_beacon_weak_key:                       ; @beacon_weak_key
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
	mov	x19, x1
	mov	x20, x0
	bl	_strlen
	mov	x21, x0
	mov	w0, #0                          ; =0x0
	mov	x1, x20
	mov	x2, x21
	bl	_crc32_le
	mov	x22, #0                         ; =0x0
LBB0_1:                                 ; =>This Inner Loop Header: Depth=1
	mov	x1, x20
	mov	x2, x21
	bl	_crc32_le
	strb	w0, [x19, x22]
	add	x22, x22, #1
	cmp	x22, #32
	b.ne	LBB0_1
; %bb.2:
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp], #48             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_beacon_nonce                   ; -- Begin function beacon_nonce
	.p2align	2
_beacon_nonce:                          ; @beacon_nonce
	.cfi_startproc
; %bb.0:
	strb	w0, [x1]
	lsr	w8, w0, #8
	strb	w8, [x1, #1]
	lsr	w8, w0, #16
	strb	w8, [x1, #2]
	lsr	w8, w0, #24
	strb	w8, [x1, #3]
	stur	xzr, [x1, #12]
	stur	xzr, [x1, #4]
	str	wzr, [x1, #20]
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_beacon_seal                    ; -- Begin function beacon_seal
	.p2align	2
_beacon_seal:                           ; @beacon_seal
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
Lloh0:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh1:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh2:
	ldr	x8, [x8]
	stur	x8, [x29, #-8]
	strb	w1, [sp]
	lsr	w8, w1, #8
	strb	w8, [sp, #1]
	lsr	w8, w1, #16
	strb	w8, [sp, #2]
	lsr	w8, w1, #24
	strb	w8, [sp, #3]
	stur	xzr, [sp, #12]
	stur	xzr, [sp, #4]
	str	wzr, [sp, #20]
	mov	x1, sp
	bl	_aead_seal
	cmp	w0, #0
	cset	w0, eq
	ldur	x8, [x29, #-8]
Lloh3:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh4:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh5:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB2_2
; %bb.1:
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
LBB2_2:
	bl	___stack_chk_fail
	.loh AdrpLdrGotLdr	Lloh3, Lloh4, Lloh5
	.loh AdrpLdrGotLdr	Lloh0, Lloh1, Lloh2
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```

## 5. Tests

```
635:static void test_beacon_weak_key(void) {
638:    beacon_weak_key("SSAT-468547-FEEBD", k1);
639:    beacon_weak_key("SSAT-468547-FEEBD", k2);
649:static void test_beacon_weak_key_differs(void) {
652:    beacon_weak_key("A", k1);
653:    beacon_weak_key("B", k2);
663:static void test_beacon_nonce(void) {
665:    beacon_nonce(0x11223344u, n);
677:static void test_beacon_seal(void) {
681:    beacon_weak_key("uid", key);
683:    TEST_ASSERT_TRUE(beacon_seal(key, 1u, g_pt, sealed));
922:static void test_teled_beacon_fail(void) {
1077:static void run_beacon_tests(void) {
1078:    RUN_TEST(test_beacon_weak_key);
1079:    RUN_TEST(test_beacon_weak_key_differs);
1080:    RUN_TEST(test_beacon_nonce);
1081:    RUN_TEST(test_beacon_seal);
1125:    RUN_TEST(test_teled_beacon_fail);
1162:    run_beacon_tests();
```

## 6. Labs

1. Read the header, then the source; explain each function.
2. Match each C function to its assembly block.
3. Run the tests for this module: `python3 scripts/run_tests.py | grep beacon`.
4. State one way this module could be misused and one way it is defended.
