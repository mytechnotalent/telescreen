# Module 09: identity

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

See `README.md` for where `identity` sits in the lab. This volume is the whole module.

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
// File:    identity.h
// Desc:    Declares the TELESCREEN device identity: Ed25519 public-key derivation,
//          signing, and verification. This replaces RSA for the hardened build.
// Created: 2026

#ifndef IDENTITY_H
#define IDENTITY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Ed25519 private seed and public key size in bytes.
 */
#define IDENTITY_KEY_SIZE 32u

/**
 * @brief Ed25519 signature size in bytes.
 */
#define IDENTITY_SIG_SIZE 64u

/**
 * @brief Derive the Ed25519 public key from a private seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @param pub Output 32-byte public key.
 * @return bool true when derivation succeeds.
 */
bool identity_pubkey(const uint8_t priv[IDENTITY_KEY_SIZE],
                     uint8_t pub[IDENTITY_KEY_SIZE]);

/**
 * @brief Sign a message with an Ed25519 private seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @param msg Pointer to message bytes.
 * @param msg_len Number of message bytes.
 * @param sig Output 64-byte signature.
 * @return bool true when signing succeeds.
 */
bool identity_sign(const uint8_t priv[IDENTITY_KEY_SIZE], const uint8_t *msg,
                   size_t msg_len, uint8_t sig[IDENTITY_SIG_SIZE]);

/**
 * @brief Verify an Ed25519 signature.
 *
 * @param pub Pointer to 32-byte public key.
 * @param msg Pointer to message bytes.
 * @param msg_len Number of message bytes.
 * @param sig Pointer to 64-byte signature.
 * @return bool true when the signature verifies.
 */
bool identity_verify(const uint8_t pub[IDENTITY_KEY_SIZE], const uint8_t *msg,
                     size_t msg_len, const uint8_t sig[IDENTITY_SIG_SIZE]);

#endif // IDENTITY_H
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
// File:    identity.c
// Desc:    Implements the TELESCREEN Ed25519 device identity.
// Created: 2026

#include "identity.h"
#include <openssl/evp.h>

/**
 * @brief Build an Ed25519 private key object from a raw seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @return EVP_PKEY* Private key object, or NULL.
 */
static EVP_PKEY *identity_priv(const uint8_t priv[32]) {
    return EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL, priv, 32u);
}

/**
 * @brief Build an Ed25519 public key object from raw bytes.
 *
 * @param pub Pointer to 32-byte public key.
 * @return EVP_PKEY* Public key object, or NULL.
 */
static EVP_PKEY *identity_pub(const uint8_t pub[32]) {
    return EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, NULL, pub, 32u);
}

/**
 * @brief Derive the Ed25519 public key from a private seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @param pub Output 32-byte public key.
 * @return bool true when derivation succeeds.
 */
bool identity_pubkey(const uint8_t priv[IDENTITY_KEY_SIZE],
                     uint8_t pub[IDENTITY_KEY_SIZE]) {
    EVP_PKEY *k = identity_priv(priv);
    size_t n = 32u;
    bool ok = (k != NULL) && (EVP_PKEY_get_raw_public_key(k, pub, &n) == 1);
    EVP_PKEY_free(k);
    return ok && (n == 32u);
}

/**
 * @brief Initialize an Ed25519 signing context.
 *
 * @param c Pointer to the message digest context.
 * @param k Pointer to the private key.
 * @return bool true when initialization succeeds.
 */
static bool ed_sign_init(EVP_MD_CTX *c, EVP_PKEY *k) {
    return EVP_DigestSignInit(c, NULL, NULL, NULL, k) == 1;
}

/**
 * @brief Sign a message with an Ed25519 private seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @param msg Pointer to message bytes.
 * @param msg_len Number of message bytes.
 * @param sig Output 64-byte signature.
 * @return bool true when signing succeeds.
 */
bool identity_sign(const uint8_t priv[IDENTITY_KEY_SIZE], const uint8_t *msg,
                   size_t msg_len, uint8_t sig[IDENTITY_SIG_SIZE]) {
    EVP_PKEY *k = identity_priv(priv);
    EVP_MD_CTX *c = EVP_MD_CTX_new();
    size_t n = 64u;
    bool ok = (k != NULL) && (c != NULL) && ed_sign_init(c, k) &&
              (EVP_DigestSign(c, sig, &n, msg, msg_len) == 1);
    EVP_MD_CTX_free(c);
    EVP_PKEY_free(k);
    return ok && (n == 64u);
}

/**
 * @brief Initialize an Ed25519 verification context.
 *
 * @param c Pointer to the message digest context.
 * @param k Pointer to the public key.
 * @return bool true when initialization succeeds.
 */
static bool ed_verify_init(EVP_MD_CTX *c, EVP_PKEY *k) {
    return EVP_DigestVerifyInit(c, NULL, NULL, NULL, k) == 1;
}

/**
 * @brief Verify an Ed25519 signature.
 *
 * @param pub Pointer to 32-byte public key.
 * @param msg Pointer to message bytes.
 * @param msg_len Number of message bytes.
 * @param sig Pointer to 64-byte signature.
 * @return bool true when the signature verifies.
 */
bool identity_verify(const uint8_t pub[IDENTITY_KEY_SIZE], const uint8_t *msg,
                     size_t msg_len, const uint8_t sig[IDENTITY_SIG_SIZE]) {
    EVP_PKEY *k = identity_pub(pub);
    EVP_MD_CTX *c = EVP_MD_CTX_new();
    bool ok = (k != NULL) && (c != NULL) && ed_verify_init(c, k) &&
              (EVP_DigestVerify(c, sig, 64u, msg, msg_len) == 1);
    EVP_MD_CTX_free(c);
    EVP_PKEY_free(k);
    return ok;
}
```

## 4. AArch64 Disassembly

```asm
	.build_version macos, 26, 0	sdk_version 27, 0
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_identity_pubkey                ; -- Begin function identity_pubkey
	.p2align	2
_identity_pubkey:                       ; @identity_pubkey
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x1
	mov	x2, x0
	mov	w20, #32                        ; =0x20
	mov	w0, #1087                       ; =0x43f
	mov	x1, #0                          ; =0x0
	mov	w3, #32                         ; =0x20
	bl	_EVP_PKEY_new_raw_private_key
	str	x20, [sp, #8]
	cbz	x0, LBB0_2
; %bb.1:
	add	x2, sp, #8
	mov	x20, x0
	mov	x1, x19
	bl	_EVP_PKEY_get_raw_public_key
	mov	x8, x0
	mov	x0, x20
	cmp	w8, #1
	cset	w19, eq
	b	LBB0_3
LBB0_2:
	mov	w19, #0                         ; =0x0
LBB0_3:
	bl	_EVP_PKEY_free
	ldr	x8, [sp, #8]
	cmp	x8, #32
	csel	w0, wzr, w19, ne
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_identity_sign                  ; -- Begin function identity_sign
	.p2align	2
_identity_sign:                         ; @identity_sign
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #80
	stp	x24, x23, [sp, #16]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #32]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #48]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #64]             ; 16-byte Folded Spill
	add	x29, sp, #64
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x21, x3
	mov	x20, x2
	mov	x22, x1
	mov	x2, x0
	mov	w0, #1087                       ; =0x43f
	mov	x1, #0                          ; =0x0
	mov	w3, #32                         ; =0x20
	bl	_EVP_PKEY_new_raw_private_key
	mov	x19, x0
	bl	_EVP_MD_CTX_new
	mov	w23, #0                         ; =0x0
	mov	w8, #64                         ; =0x40
	str	x8, [sp, #8]
	cbz	x19, LBB1_5
; %bb.1:
	cbz	x0, LBB1_5
; %bb.2:
	mov	x24, x0
	mov	x1, #0                          ; =0x0
	mov	x2, #0                          ; =0x0
	mov	x3, #0                          ; =0x0
	mov	x4, x19
	bl	_EVP_DigestSignInit
	cmp	w0, #1
	b.ne	LBB1_4
; %bb.3:
	add	x2, sp, #8
	mov	x0, x24
	mov	x1, x21
	mov	x3, x22
	mov	x4, x20
	bl	_EVP_DigestSign
	mov	x8, x0
	mov	x0, x24
	cmp	w8, #1
	cset	w23, eq
	b	LBB1_5
LBB1_4:
	mov	w23, #0                         ; =0x0
	mov	x0, x24
LBB1_5:
	bl	_EVP_MD_CTX_free
	mov	x0, x19
	bl	_EVP_PKEY_free
	ldr	x8, [sp, #8]
	cmp	x8, #64
	csel	w0, wzr, w23, ne
	ldp	x29, x30, [sp, #64]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #80
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_identity_verify                ; -- Begin function identity_verify
	.p2align	2
_identity_verify:                       ; @identity_verify
	.cfi_startproc
; %bb.0:
	stp	x24, x23, [sp, #-64]!           ; 16-byte Folded Spill
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
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x22, x3
	mov	x20, x2
	mov	x21, x1
	mov	x2, x0
	mov	w0, #1087                       ; =0x43f
	mov	x1, #0                          ; =0x0
	mov	w3, #32                         ; =0x20
	bl	_EVP_PKEY_new_raw_public_key
	mov	x19, x0
	bl	_EVP_MD_CTX_new
	mov	w23, #0                         ; =0x0
	cbz	x19, LBB2_5
; %bb.1:
	cbz	x0, LBB2_5
; %bb.2:
	mov	x24, x0
	mov	x1, #0                          ; =0x0
	mov	x2, #0                          ; =0x0
	mov	x3, #0                          ; =0x0
	mov	x4, x19
	bl	_EVP_DigestVerifyInit
	cmp	w0, #1
	b.ne	LBB2_4
; %bb.3:
	mov	x0, x24
	mov	x1, x22
	mov	w2, #64                         ; =0x40
	mov	x3, x21
	mov	x4, x20
	bl	_EVP_DigestVerify
	mov	x8, x0
	mov	x0, x24
	cmp	w8, #1
	cset	w23, eq
	b	LBB2_5
LBB2_4:
	mov	w23, #0                         ; =0x0
	mov	x0, x24
LBB2_5:
	bl	_EVP_MD_CTX_free
	mov	x0, x19
	bl	_EVP_PKEY_free
	mov	x0, x23
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp], #64             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
```

## 5. Tests

```
847:static void test_identity_rfc8032(void) {
850:    TEST_ASSERT_TRUE(identity_pubkey(s_ed_priv, pub));
852:    TEST_ASSERT_TRUE(identity_sign(s_ed_priv, s_ed_msg, 0u, sig));
854:    TEST_ASSERT_TRUE(identity_verify(s_ed_pub, s_ed_msg, 0u, sig));
863:static void test_identity_verify_bad(void) {
867:    TEST_ASSERT_FALSE(identity_verify(s_ed_pub, s_ed_msg, 0u, sig));
1101:static void run_identity_tests(void) {
1102:    RUN_TEST(test_identity_rfc8032);
1103:    RUN_TEST(test_identity_verify_bad);
1173:    run_identity_tests();
```

## 6. Labs

1. Read the header, then the source; explain each function.
2. Match each C function to its assembly block.
3. Run the tests for this module: `python3 scripts/run_tests.py | grep identity`.
4. State one way this module could be misused and one way it is defended.
