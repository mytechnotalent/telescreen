# Module 11: camera

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

See `README.md` for where `camera` sits in the lab. This volume is the whole module.

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
// File:    camera.h
// Desc:    Declares the UVC camera helpers for the TELESCREEN lab. A cheap USB
//          webcam enumerates as /dev/video0, so the pipeline is plain V4L2.
// Created: 2026

#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Default UVC capture device path.
 */
#define CAMERA_DEV_PATH "/dev/video0"

/**
 * @brief Default RTSP path served by the lab.
 */
#define CAMERA_RTSP_PATH "/stream"

/**
 * @brief Default MJPEG path served over HTTP for app-free viewing.
 */
#define CAMERA_MJPEG_PATH "/video.mjpg"

/**
 * @brief Default RTSP service port.
 */
#define CAMERA_RTSP_PORT 554u

/**
 * @brief Default HTTP service port.
 */
#define CAMERA_HTTP_PORT 80u

/**
 * @brief Build the RTSP URL a client (or VLC) connects to.
 *
 * @param ip NUL-terminated device IPv4 address.
 * @param port TCP port.
 * @param out Output URL buffer.
 * @param out_len Size of the output buffer.
 * @return bool true when the URL fits and is written.
 */
bool camera_rtsp_url(const char *ip, uint16_t port, char *out, size_t out_len);

/**
 * @brief Build the MJPEG URL a browser connects to.
 *
 * @param ip NUL-terminated device IPv4 address.
 * @param port TCP port.
 * @param out Output URL buffer.
 * @param out_len Size of the output buffer.
 * @return bool true when the URL fits and is written.
 */
bool camera_mjpeg_url(const char *ip, uint16_t port, char *out, size_t out_len);

#endif // CAMERA_H
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
// File:    camera.c
// Desc:    Implements the UVC camera URL helpers for the TELESCREEN lab.
// Created: 2026

#include "camera.h"
#include <stdio.h>

/**
 * @brief Format a URL and report whether it fit.
 *
 * @param out Output URL buffer.
 * @param out_len Size of the output buffer.
 * @param ip NUL-terminated device IPv4 address.
 * @param port TCP port.
 * @param scheme URL scheme ("rtsp" or "http").
 * @param path URL path.
 * @return bool true when the URL fits and is written.
 */
static bool camera_url(char *out, size_t out_len, const char *ip, uint16_t port,
                       const char *scheme, const char *path) {
    int n;
    if ((out == NULL) || (ip == NULL) || (out_len == 0u)) return false;
    n = snprintf(out, out_len, "%s://%s:%u%s", scheme, ip, (unsigned)port, path);
    return (n > 0) && ((size_t)n < out_len);
}

/**
 * @brief Build the RTSP URL a client (or VLC) connects to.
 *
 * @param ip NUL-terminated device IPv4 address.
 * @param port TCP port.
 * @param out Output URL buffer.
 * @param out_len Size of the output buffer.
 * @return bool true when the URL fits and is written.
 */
bool camera_rtsp_url(const char *ip, uint16_t port, char *out, size_t out_len) {
    return camera_url(out, out_len, ip, port, "rtsp", CAMERA_RTSP_PATH);
}

/**
 * @brief Build the MJPEG URL a browser connects to.
 *
 * @param ip NUL-terminated device IPv4 address.
 * @param port TCP port.
 * @param out Output URL buffer.
 * @param out_len Size of the output buffer.
 * @return bool true when the URL fits and is written.
 */
bool camera_mjpeg_url(const char *ip, uint16_t port, char *out, size_t out_len) {
    return camera_url(out, out_len, ip, port, "http", CAMERA_MJPEG_PATH);
}
```

## 4. AArch64 Disassembly

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_camera_rtsp_url                ; -- Begin function camera_rtsp_url
	.p2align	2
_camera_rtsp_url:                       ; @camera_rtsp_url
	.cfi_startproc
; %bb.0:
                                        ; kill: def $w1 killed $w1 def $x1
	mov	w8, #0                          ; =0x0
	cbz	x3, LBB0_4
; %bb.1:
	cbz	x0, LBB0_4
; %bb.2:
	cbz	x2, LBB0_4
; %bb.3:
	sub	sp, sp, #64
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
Lloh0:
	adrp	x8, l_.str.1@PAGE
Lloh1:
	add	x8, x8, l_.str.1@PAGEOFF
	stp	x1, x8, [sp, #16]
Lloh2:
	adrp	x8, l_.str@PAGE
Lloh3:
	add	x8, x8, l_.str@PAGEOFF
	stp	x8, x0, [sp]
Lloh4:
	adrp	x8, l_.str.4@PAGE
Lloh5:
	add	x8, x8, l_.str.4@PAGEOFF
	mov	x0, x2
	mov	x1, x3
	mov	x2, x8
	mov	x19, x3
	bl	_snprintf
                                        ; kill: def $w0 killed $w0 def $x0
	cmp	w0, #0
	sxtw	x8, w0
	ccmp	x19, x8, #0, gt
	cset	w8, hi
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #64
LBB0_4:
	mov	x0, x8
	ret
	.loh AdrpAdd	Lloh4, Lloh5
	.loh AdrpAdd	Lloh2, Lloh3
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.globl	_camera_mjpeg_url               ; -- Begin function camera_mjpeg_url
	.p2align	2
_camera_mjpeg_url:                      ; @camera_mjpeg_url
	.cfi_startproc
; %bb.0:
                                        ; kill: def $w1 killed $w1 def $x1
	mov	w8, #0                          ; =0x0
	cbz	x3, LBB1_4
; %bb.1:
	cbz	x0, LBB1_4
; %bb.2:
	cbz	x2, LBB1_4
; %bb.3:
	sub	sp, sp, #64
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
Lloh6:
	adrp	x8, l_.str.3@PAGE
Lloh7:
	add	x8, x8, l_.str.3@PAGEOFF
	stp	x1, x8, [sp, #16]
Lloh8:
	adrp	x8, l_.str.2@PAGE
Lloh9:
	add	x8, x8, l_.str.2@PAGEOFF
	stp	x8, x0, [sp]
Lloh10:
	adrp	x8, l_.str.4@PAGE
Lloh11:
	add	x8, x8, l_.str.4@PAGEOFF
	mov	x0, x2
	mov	x1, x3
	mov	x2, x8
	mov	x19, x3
	bl	_snprintf
                                        ; kill: def $w0 killed $w0 def $x0
	cmp	w0, #0
	sxtw	x8, w0
	ccmp	x19, x8, #0, gt
	cset	w8, hi
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	add	sp, sp, #64
LBB1_4:
	mov	x0, x8
	ret
	.loh AdrpAdd	Lloh10, Lloh11
	.loh AdrpAdd	Lloh8, Lloh9
	.loh AdrpAdd	Lloh6, Lloh7
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"rtsp"

l_.str.1:                               ; @.str.1
	.asciz	"/stream"

l_.str.2:                               ; @.str.2
	.asciz	"http"

l_.str.3:                               ; @.str.3
	.asciz	"/video.mjpg"

l_.str.4:                               ; @.str.4
	.asciz	"%s://%s:%u%s"

.subsections_via_symbols
```

## 5. Tests

```
936:static void test_camera_rtsp_url(void) {
938:    TEST_ASSERT_TRUE(camera_rtsp_url("192.168.1.88", 554u, out, sizeof(out)));
948:static void test_camera_mjpeg_url(void) {
950:    TEST_ASSERT_TRUE(camera_mjpeg_url("192.168.1.88", 80u, out, sizeof(out)));
960:static void test_camera_url_bad(void) {
962:    TEST_ASSERT_FALSE(camera_rtsp_url("192.168.1.88", 554u, out, sizeof(out)));
963:    TEST_ASSERT_FALSE(camera_rtsp_url(NULL, 554u, out, sizeof(out)));
1134:static void run_camera_tests(void) {
1135:    RUN_TEST(test_camera_rtsp_url);
1136:    RUN_TEST(test_camera_mjpeg_url);
1137:    RUN_TEST(test_camera_url_bad);
1176:    run_camera_tests();
```

## 6. Labs

1. Read the header, then the source; explain each function.
2. Match each C function to its assembly block.
3. Run the tests for this module: `python3 scripts/run_tests.py | grep camera`.
4. State one way this module could be misused and one way it is defended.
