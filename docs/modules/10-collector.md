# Module 10: collector

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

See `README.md` for where `collector` sits in the lab. This volume is the whole module.

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
// File:    collector.h
// Desc:    Declares the local lab sink that receives TELESCREEN beacons. It is
//          local-only by design and never bridges to a production network.
// Created: 2026

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Collector accounting snapshot.
 */
typedef struct collector_stat {
    /**
     * @brief Number of accepted beacons.
     */
    uint32_t accepted;
    /**
     * @brief Number of rejected beacons.
     */
    uint32_t rejected;
    /**
     * @brief Sequence number of the most recent accepted beacon.
     */
    uint32_t last_seq;
} collector_stat_t;

/**
 * @brief Reset the collector accounting state.
 *
 * @param void No parameters.
 * @return void
 */
void collector_init(void);

/**
 * @brief Accept or reject one beacon frame.
 *
 * @param sealed Pointer to the sealed frame bytes.
 * @param len Number of frame bytes.
 * @param seq Sequence number carried by the frame.
 * @return bool true when the frame is accepted.
 */
bool collector_accept(const uint8_t *sealed, size_t len, uint32_t seq);

/**
 * @brief Return a pointer to the collector accounting snapshot.
 *
 * @param void No parameters.
 * @return const collector_stat_t* Pointer to the static snapshot.
 */
const collector_stat_t *collector_stats(void);

#endif // COLLECTOR_H
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
// File:    collector.c
// Desc:    Implements the local lab sink that receives TELESCREEN beacons.
// Created: 2026

#include "collector.h"
#include "aead.h"

/**
 * @brief Collector accounting state.
 */
static collector_stat_t g_stats;

/**
 * @brief Reset the collector accounting state.
 *
 * @param void No parameters.
 * @return void
 */
void collector_init(void) {
    g_stats.accepted = 0u;
    g_stats.rejected = 0u;
    g_stats.last_seq = 0u;
}

/**
 * @brief Accept or reject one beacon frame.
 *
 * @param sealed Pointer to the sealed frame bytes.
 * @param len Number of frame bytes.
 * @param seq Sequence number carried by the frame.
 * @return bool true when the frame is accepted.
 */
bool collector_accept(const uint8_t *sealed, size_t len, uint32_t seq) {
    if ((sealed == NULL) || (len != AEAD_SEALED_SIZE)) {
        g_stats.rejected += 1u;
        return false;
    }
    g_stats.accepted += 1u;
    g_stats.last_seq = seq;
    return true;
}

/**
 * @brief Return a pointer to the collector accounting snapshot.
 *
 * @param void No parameters.
 * @return const collector_stat_t* Pointer to the static snapshot.
 */
const collector_stat_t *collector_stats(void) {
    return &g_stats;
}
```

## 4. AArch64 Disassembly

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_collector_init                 ; -- Begin function collector_init
	.p2align	2
_collector_init:                        ; @collector_init
	.cfi_startproc
; %bb.0:
Lloh0:
	adrp	x8, _g_stats@PAGE
Lloh1:
	add	x8, x8, _g_stats@PAGEOFF
	str	xzr, [x8]
	str	wzr, [x8, #8]
	ret
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_collector_accept               ; -- Begin function collector_accept
	.p2align	2
_collector_accept:                      ; @collector_accept
	.cfi_startproc
; %bb.0:
	cmp	x0, #0
	mov	w8, #64                         ; =0x40
	ccmp	x1, x8, #0, ne
	cset	w0, eq
	b.ne	LBB1_2
; %bb.1:
Lloh2:
	adrp	x8, _g_stats@PAGE
Lloh3:
	add	x8, x8, _g_stats@PAGEOFF
	ldr	w9, [x8]
	add	w9, w9, #1
	str	w9, [x8]
	str	w2, [x8, #8]
	ret
LBB1_2:
	adrp	x8, _g_stats@PAGE+4
	ldr	w9, [x8, _g_stats@PAGEOFF+4]
	add	w9, w9, #1
	str	w9, [x8, _g_stats@PAGEOFF+4]
	ret
	.loh AdrpAdd	Lloh2, Lloh3
	.cfi_endproc
                                        ; -- End function
	.globl	_collector_stats                ; -- Begin function collector_stats
	.p2align	2
_collector_stats:                       ; @collector_stats
	.cfi_startproc
; %bb.0:
Lloh4:
	adrp	x0, _g_stats@PAGE
Lloh5:
	add	x0, x0, _g_stats@PAGEOFF
	ret
	.loh AdrpAdd	Lloh4, Lloh5
	.cfi_endproc
                                        ; -- End function
.zerofill __DATA,__bss,_g_stats,12,3    ; @g_stats
.subsections_via_symbols
```

## 5. Tests

```
877:static void test_collector_accept(void) {
879:    collector_init();
880:    TEST_ASSERT_TRUE(collector_accept(sealed, sizeof(sealed), 5u));
881:    TEST_ASSERT_EQUAL_UINT(1u, collector_stats()->accepted);
882:    TEST_ASSERT_EQUAL_UINT(5u, collector_stats()->last_seq);
891:static void test_collector_reject(void) {
893:    collector_init();
894:    TEST_ASSERT_FALSE(collector_accept(NULL, sizeof(sealed), 0u));
895:    TEST_ASSERT_FALSE(collector_accept(sealed, 1u, 0u));
896:    TEST_ASSERT_EQUAL_UINT(2u, collector_stats()->rejected);
1112:static void run_collector_tests(void) {
1113:    RUN_TEST(test_collector_accept);
1114:    RUN_TEST(test_collector_reject);
1174:    run_collector_tests();
```

## 6. Labs

1. Read the header, then the source; explain each function.
2. Match each C function to its assembly block.
3. Run the tests for this module: `python3 scripts/run_tests.py | grep collector`.
4. State one way this module could be misused and one way it is defended.
