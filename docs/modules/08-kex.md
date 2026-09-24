# Module 08: kex

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

See `README.md` for where `kex` sits in the lab. This volume is the whole module.

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
// File:    kex.h
// Desc:    Declares the hardened key agreement: X25519 for the shared secret and
//          HKDF-SHA256 to derive the AEAD session key.
// Created: 2026

#ifndef KEX_H
#define KEX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief X25519 key and shared-secret size in bytes.
 */
#define KEX_KEY_SIZE 32u

/**
 * @brief HKDF output key size in bytes.
 */
#define KEX_OKM_SIZE 32u

/**
 * @brief Compute the X25519 shared secret.
 *
 * @param priv Pointer to 32-byte private scalar.
 * @param peer Pointer to 32-byte peer public key.
 * @param out Output 32-byte shared secret.
 * @return bool true when the derivation succeeds.
 */
bool kex_x25519(const uint8_t priv[KEX_KEY_SIZE], const uint8_t peer[KEX_KEY_SIZE],
                uint8_t out[KEX_KEY_SIZE]);

/**
 * @brief Derive key material with HKDF-SHA256.
 *
 * @param ikm Pointer to input key material.
 * @param ikm_len Number of input key material bytes.
 * @param salt Pointer to salt bytes.
 * @param salt_len Number of salt bytes.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param out Output key material buffer.
 * @param out_len Number of output bytes.
 * @return bool true when derivation succeeds.
 */
bool kex_hkdf(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
              size_t salt_len, const uint8_t *info, size_t info_len,
              uint8_t *out, size_t out_len);

#endif // KEX_H
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
// File:    kex.c
// Desc:    Implements X25519 key agreement and HKDF-SHA256 key derivation for the
//          hardened TELESCREEN session key.
// Created: 2026

#include "kex.h"
#include <string.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

/**
 * @brief Compute one HMAC-SHA256 over a byte range.
 *
 * @param key Pointer to HMAC key bytes.
 * @param key_len Number of key bytes.
 * @param data Pointer to data bytes.
 * @param data_len Number of data bytes.
 * @param out Output 32-byte MAC buffer.
 * @return bool true when the MAC succeeds.
 */
static bool hkdf_hmac(const uint8_t *key, size_t key_len, const uint8_t *data,
                      size_t data_len, uint8_t out[32]) {
    unsigned int n = 0u;
    return HMAC(EVP_sha256(), key, (int)key_len, data, data_len, out, &n) != NULL;
}

/**
 * @brief Assemble one HKDF expand input block.
 *
 * @param buf Output assembly buffer.
 * @param tprev Pointer to the previous T block.
 * @param tlen Number of previous T bytes.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param counter One-based block counter byte.
 * @return size_t Assembled block length.
 */
static size_t hkdf_assemble(uint8_t *buf, const uint8_t *tprev, size_t tlen,
                            const uint8_t *info, size_t info_len, uint8_t counter) {
    memcpy(buf, tprev, tlen);
    memcpy(&buf[tlen], info, info_len);
    buf[tlen + info_len] = counter;
    return tlen + info_len + 1u;
}

/**
 * @brief Compute one HKDF expand T block.
 *
 * @param prk Pointer to the 32-byte pseudorandom key.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param tprev Pointer to the previous T block.
 * @param tlen Number of previous T bytes.
 * @param counter One-based block counter byte.
 * @param out Output 32-byte T block buffer.
 * @return size_t Number of bytes produced, or zero on failure.
 */
static size_t hkdf_block(const uint8_t prk[32], const uint8_t *info, size_t info_len,
                         const uint8_t *tprev, size_t tlen, uint8_t counter,
                         uint8_t out[32]) {
    uint8_t buf[97];
    size_t blen = hkdf_assemble(buf, tprev, tlen, info, info_len, counter);
    if (!hkdf_hmac(prk, 32u, buf, blen, out)) return 0u;
    return 32u;
}

/**
 * @brief Produce and copy one HKDF expand block into the output.
 *
 * @param prk Pointer to the 32-byte pseudorandom key.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param t Pointer to the mutable T block buffer.
 * @param tlen Pointer to the current T block length.
 * @param counter One-based block counter byte.
 * @param out Output key material buffer.
 * @param done Pointer to the number of output bytes written.
 * @param out_len Total number of output bytes.
 * @return bool true when the step succeeds.
 */
static bool hkdf_step(const uint8_t prk[32], const uint8_t *info, size_t info_len,
                      uint8_t *t, size_t *tlen, uint8_t counter, uint8_t *out,
                      size_t *done, size_t out_len) {
    size_t n = hkdf_block(prk, info, info_len, t, *tlen, counter, t);
    size_t copy;
    if (n == 0u) return false;
    *tlen = n;
    copy = (out_len - *done < n) ? (out_len - *done) : n;
    memcpy(&out[*done], t, copy);
    *done += copy;
    return true;
}

/**
 * @brief Run the HKDF expand phase.
 *
 * @param prk Pointer to the 32-byte pseudorandom key.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param out Output key material buffer.
 * @param out_len Number of output bytes.
 * @return bool true when expansion succeeds.
 */
static bool hkdf_expand(const uint8_t prk[32], const uint8_t *info, size_t info_len,
                        uint8_t *out, size_t out_len) {
    uint8_t t[32];
    size_t done = 0u;
    size_t tlen = 0u;
    uint8_t counter = 1u;
    while (done < out_len) {
        if (!hkdf_step(prk, info, info_len, t, &tlen, counter, out, &done, out_len)) return false;
        counter += 1u;
    }
    return true;
}

/**
 * @brief Derive key material with HKDF-SHA256.
 *
 * @param ikm Pointer to input key material.
 * @param ikm_len Number of input key material bytes.
 * @param salt Pointer to salt bytes.
 * @param salt_len Number of salt bytes.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param out Output key material buffer.
 * @param out_len Number of output bytes.
 * @return bool true when derivation succeeds.
 */
bool kex_hkdf(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
              size_t salt_len, const uint8_t *info, size_t info_len,
              uint8_t *out, size_t out_len) {
    uint8_t prk[32];
    if (!hkdf_hmac(salt, salt_len, ikm, ikm_len, prk)) return false;
    return hkdf_expand(prk, info, info_len, out, out_len);
}

/**
 * @brief Drive an X25519 EVP context to a shared secret.
 *
 * @param c Pointer to the EVP key context.
 * @param b Pointer to the peer public key.
 * @param out Output shared-secret buffer.
 * @param n Pointer to the output length.
 * @return bool true when the derivation succeeds.
 */
static bool x25519_derive(EVP_PKEY_CTX *c, EVP_PKEY *b, uint8_t *out, size_t *n) {
    if (c == NULL) return false;
    if (EVP_PKEY_derive_init(c) != 1) return false;
    if (EVP_PKEY_derive_set_peer(c, b) != 1) return false;
    return EVP_PKEY_derive(c, out, n) == 1;
}

/**
 * @brief Release the X25519 EVP objects.
 *
 * @param c Pointer to the EVP key context.
 * @param a Pointer to the private key.
 * @param b Pointer to the public key.
 * @return void
 */
static void kex_free(EVP_PKEY_CTX *c, EVP_PKEY *a, EVP_PKEY *b) {
    EVP_PKEY_CTX_free(c);
    EVP_PKEY_free(a);
    EVP_PKEY_free(b);
}

/**
 * @brief Compute the X25519 shared secret.
 *
 * @param priv Pointer to 32-byte private scalar.
 * @param peer Pointer to 32-byte peer public key.
 * @param out Output 32-byte shared secret.
 * @return bool true when the derivation succeeds.
 */
bool kex_x25519(const uint8_t priv[KEX_KEY_SIZE], const uint8_t peer[KEX_KEY_SIZE],
                uint8_t out[KEX_KEY_SIZE]) {
    EVP_PKEY *a = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, priv, 32u);
    EVP_PKEY *b = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, NULL, peer, 32u);
    EVP_PKEY_CTX *c = ((a != NULL) && (b != NULL)) ? EVP_PKEY_CTX_new(a, NULL) : NULL;
    size_t n = 32u;
    bool ok = x25519_derive(c, b, out, &n);
    kex_free(c, a, b);
    return ok && (n == 32u);
}
```

## 4. AArch64 Disassembly

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_kex_hkdf                       ; -- Begin function kex_hkdf
	.p2align	2
_kex_hkdf:                              ; @kex_hkdf
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #304
	stp	x28, x27, [sp, #208]            ; 16-byte Folded Spill
	stp	x26, x25, [sp, #224]            ; 16-byte Folded Spill
	stp	x24, x23, [sp, #240]            ; 16-byte Folded Spill
	stp	x22, x21, [sp, #256]            ; 16-byte Folded Spill
	stp	x20, x19, [sp, #272]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #288]            ; 16-byte Folded Spill
	add	x29, sp, #288
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	.cfi_offset w25, -72
	.cfi_offset w26, -80
	.cfi_offset w27, -88
	.cfi_offset w28, -96
	mov	x19, x7
	mov	x20, x6
	mov	x21, x5
	mov	x22, x4
	mov	x23, x3
	mov	x24, x2
	mov	x25, x1
	mov	x26, x0
Lloh0:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh1:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh2:
	ldr	x8, [x8]
	stur	x8, [x29, #-96]
	str	wzr, [sp, #80]
	bl	_EVP_sha256
	add	x5, sp, #16
	add	x6, sp, #80
	mov	x1, x24
	mov	x2, x23
	mov	x3, x26
	mov	x4, x25
	bl	_HMAC
	cbz	x0, LBB0_7
; %bb.1:
	mov	w25, #1                         ; =0x1
	cbz	x19, LBB0_8
; %bb.2:
	add	x23, sp, #80
	add	x0, sp, #80
	mov	x1, x22
	mov	x2, x21
	mov	w3, #97                         ; =0x61
	bl	___memcpy_chk
	strb	w25, [x23, x21]
	str	wzr, [sp, #12]
	bl	_EVP_sha256
	add	x1, sp, #16
	add	x3, sp, #80
	add	x4, x21, #1
	add	x5, sp, #48
	add	x6, sp, #12
	mov	w2, #32                         ; =0x20
	bl	_HMAC
	cbz	x0, LBB0_7
; %bb.3:
	mov	w8, #32                         ; =0x20
	cmp	x19, #32
	csel	x23, x19, x8, lo
	add	x1, sp, #48
	mov	x0, x20
	mov	x2, x23
	bl	_memcpy
	cmp	x19, #33
	b.lo	LBB0_8
; %bb.4:
	add	x8, sp, #80
	add	x24, x8, #32
	mov	w27, #2                         ; =0x2
	mov	w28, #32                        ; =0x20
LBB0_5:                                 ; =>This Inner Loop Header: Depth=1
	ldp	q0, q1, [sp, #48]
	stp	q0, q1, [sp, #80]
	mov	x0, x24
	mov	x1, x22
	mov	x2, x21
	mov	w3, #65                         ; =0x41
	bl	___memcpy_chk
	strb	w27, [x24, x21]
	str	wzr, [sp, #12]
	bl	_EVP_sha256
	add	x1, sp, #16
	add	x3, sp, #80
	add	x4, x21, #33
	add	x5, sp, #48
	add	x6, sp, #12
	mov	w2, #32                         ; =0x20
	bl	_HMAC
	cmp	x0, #0
	cset	w25, ne
	cbz	x0, LBB0_8
; %bb.6:                                ;   in Loop: Header=BB0_5 Depth=1
	sub	x8, x19, x23
	cmp	x8, #32
	csel	x26, x8, x28, lo
	add	x0, x20, x23
	add	x1, sp, #48
	mov	x2, x26
	bl	_memcpy
	add	w27, w27, #1
	add	x23, x26, x23
	cmp	x23, x19
	b.lo	LBB0_5
	b	LBB0_8
LBB0_7:
	mov	w25, #0                         ; =0x0
LBB0_8:
	ldur	x8, [x29, #-96]
Lloh3:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh4:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh5:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB0_10
; %bb.9:
	mov	x0, x25
	ldp	x29, x30, [sp, #288]            ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #272]            ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #256]            ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #240]            ; 16-byte Folded Reload
	ldp	x26, x25, [sp, #224]            ; 16-byte Folded Reload
	ldp	x28, x27, [sp, #208]            ; 16-byte Folded Reload
	add	sp, sp, #304
	ret
LBB0_10:
	bl	___stack_chk_fail
	.loh AdrpLdrGotLdr	Lloh0, Lloh1, Lloh2
	.loh AdrpLdrGotLdr	Lloh3, Lloh4, Lloh5
	.cfi_endproc
                                        ; -- End function
	.globl	_kex_x25519                     ; -- Begin function kex_x25519
	.p2align	2
_kex_x25519:                            ; @kex_x25519
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #64
	stp	x22, x21, [sp, #16]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	x21, x2
	mov	x20, x1
	mov	x2, x0
	mov	w0, #1034                       ; =0x40a
	mov	x1, #0                          ; =0x0
	mov	w3, #32                         ; =0x20
	bl	_EVP_PKEY_new_raw_private_key
	mov	x19, x0
	mov	w0, #1034                       ; =0x40a
	mov	x1, #0                          ; =0x0
	mov	x2, x20
	mov	w3, #32                         ; =0x20
	bl	_EVP_PKEY_new_raw_public_key
	mov	x20, x0
	cbz	x19, LBB1_6
; %bb.1:
	cbz	x20, LBB1_6
; %bb.2:
	mov	x0, x19
	mov	x1, #0                          ; =0x0
	bl	_EVP_PKEY_CTX_new
	mov	w8, #32                         ; =0x20
	str	x8, [sp, #8]
	cbz	x0, LBB1_8
; %bb.3:
	mov	x22, x0
	bl	_EVP_PKEY_derive_init
	cmp	w0, #1
	b.ne	LBB1_7
; %bb.4:
	mov	x0, x22
	mov	x1, x20
	bl	_EVP_PKEY_derive_set_peer
	cmp	w0, #1
	b.ne	LBB1_7
; %bb.5:
	add	x2, sp, #8
	mov	x0, x22
	mov	x1, x21
	bl	_EVP_PKEY_derive
	mov	x8, x0
	mov	x0, x22
	cmp	w8, #1
	cset	w21, eq
	b	LBB1_9
LBB1_6:
	mov	x0, #0                          ; =0x0
	mov	w21, #0                         ; =0x0
	mov	w8, #32                         ; =0x20
	str	x8, [sp, #8]
	b	LBB1_9
LBB1_7:
	mov	w21, #0                         ; =0x0
	mov	x0, x22
	b	LBB1_9
LBB1_8:
	mov	w21, #0                         ; =0x0
LBB1_9:
	bl	_EVP_PKEY_CTX_free
	mov	x0, x19
	bl	_EVP_PKEY_free
	mov	x0, x20
	bl	_EVP_PKEY_free
	ldr	x8, [sp, #8]
	cmp	x8, #32
	csel	w0, wzr, w21, ne
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #64
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```

## 5. Tests

```
779:static void test_kex_x25519_rfc7748(void) {
782:    TEST_ASSERT_TRUE(kex_x25519(s_alice_priv, s_bob_pub, shared_a));
783:    TEST_ASSERT_TRUE(kex_x25519(s_bob_priv, s_alice_pub, shared_b));
794:static void test_kex_hkdf_rfc5869(void) {
796:    TEST_ASSERT_TRUE(kex_hkdf(s_ikm, sizeof(s_ikm), s_salt, sizeof(s_salt),
1090:static void run_kex_tests(void) {
1091:    RUN_TEST(test_kex_x25519_rfc7748);
1092:    RUN_TEST(test_kex_hkdf_rfc5869);
1163:    run_kex_tests();
```

## 6. Labs

1. Read the header, then the source; explain each function.
2. Match each C function to its assembly block.
3. Run the tests for this module: `python3 scripts/run_tests.py | grep kex`.
4. State one way this module could be misused and one way it is defended.
