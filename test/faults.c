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
// GitHub:  https://github.com/mytechnotalent/telescreen
// File:    faults.c
// Desc:    Link-companion to test/faults.h. When the build renames OpenSSL
//          calls to fault_* (see scripts/run_tests.py), these wrappers forward
//          to the real functions, or fail on demand to exercise error paths.
// Created: 2026

#undef EVP_CIPHER_CTX_new
#undef EVP_MD_CTX_new
#undef EVP_EncryptInit_ex
#undef EVP_EncryptUpdate
#undef EVP_EncryptFinal_ex
#undef EVP_DecryptInit_ex
#undef EVP_DecryptUpdate
#undef EVP_DecryptFinal_ex
#undef EVP_CIPHER_CTX_ctrl
#undef HMAC
#undef EVP_PKEY_get_raw_public_key
#undef EVP_DigestSignInit
#undef EVP_DigestSign
#undef EVP_DigestVerifyInit
#undef EVP_DigestVerify
#undef EVP_PKEY_derive_init
#undef EVP_PKEY_derive_set_peer
#undef EVP_PKEY_derive

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include "faults.h"

int g_fault = FAULT_NONE;

EVP_CIPHER_CTX *fault_EVP_CIPHER_CTX_new(void) {
    if (g_fault == FAULT_CTX_NEW) return NULL;
    return EVP_CIPHER_CTX_new();
}

EVP_MD_CTX *fault_EVP_MD_CTX_new(void) {
    if (g_fault == FAULT_MD_CTX_NEW) return NULL;
    return EVP_MD_CTX_new();
}

int fault_EVP_EncryptInit_ex(EVP_CIPHER_CTX *c, const EVP_CIPHER *t, ENGINE *e,
                             const unsigned char *k, const unsigned char *iv) {
    if (g_fault == FAULT_ENC_INIT) return 0;
    return EVP_EncryptInit_ex(c, t, e, k, iv);
}

int fault_EVP_EncryptUpdate(EVP_CIPHER_CTX *c, unsigned char *out, int *outl,
                            const unsigned char *in, int inl) {
    if (g_fault == FAULT_ENC_UPDATE) return 0;
    return EVP_EncryptUpdate(c, out, outl, in, inl);
}

int fault_EVP_EncryptFinal_ex(EVP_CIPHER_CTX *c, unsigned char *out, int *outl) {
    if (g_fault == FAULT_ENC_FINAL) return 0;
    return EVP_EncryptFinal_ex(c, out, outl);
}

int fault_EVP_DecryptInit_ex(EVP_CIPHER_CTX *c, const EVP_CIPHER *t, ENGINE *e,
                             const unsigned char *k, const unsigned char *iv) {
    if (g_fault == FAULT_DEC_INIT) return 0;
    return EVP_DecryptInit_ex(c, t, e, k, iv);
}

int fault_EVP_DecryptUpdate(EVP_CIPHER_CTX *c, unsigned char *out, int *outl,
                            const unsigned char *in, int inl) {
    if (g_fault == FAULT_DEC_UPDATE) return 0;
    return EVP_DecryptUpdate(c, out, outl, in, inl);
}

int fault_EVP_DecryptFinal_ex(EVP_CIPHER_CTX *c, unsigned char *out, int *outl) {
    if (g_fault == FAULT_DEC_FINAL) return 0;
    return EVP_DecryptFinal_ex(c, out, outl);
}

int fault_EVP_CIPHER_CTX_ctrl(EVP_CIPHER_CTX *c, int type, int arg, void *ptr) {
    if (g_fault == FAULT_CTRL) return 0;
    return EVP_CIPHER_CTX_ctrl(c, type, arg, ptr);
}

unsigned char *fault_HMAC(const EVP_MD *md, const void *key, int key_len,
                          const unsigned char *d, size_t n, unsigned char *md_out,
                          unsigned int *md_len) {
    if (g_fault == FAULT_HMAC) return NULL;
    return HMAC(md, key, key_len, d, n, md_out, md_len);
}

int fault_EVP_PKEY_get_raw_public_key(const EVP_PKEY *p, unsigned char *pub, size_t *len) {
    int r;
    if (g_fault == FAULT_GET_PUB) return 0;
    r = EVP_PKEY_get_raw_public_key(p, pub, len);
    if (g_fault == FAULT_GET_PUB_LEN) *len = 31u;
    return r;
}

int fault_EVP_DigestSignInit(EVP_MD_CTX *c, EVP_PKEY_CTX **pctx,
                             const EVP_MD *type, ENGINE *e, EVP_PKEY *pkey) {
    if (g_fault == FAULT_SIGN_INIT) return 0;
    return EVP_DigestSignInit(c, pctx, type, e, pkey);
}

int fault_EVP_DigestSign(EVP_MD_CTX *c, unsigned char *sig, size_t *siglen,
                         const unsigned char *tbs, size_t tbslen) {
    int r;
    if (g_fault == FAULT_SIGN) return 0;
    r = EVP_DigestSign(c, sig, siglen, tbs, tbslen);
    if (g_fault == FAULT_SIGN_LEN) *siglen = 63u;
    return r;
}

int fault_EVP_DigestVerifyInit(EVP_MD_CTX *c, EVP_PKEY_CTX **pctx,
                               const EVP_MD *type, ENGINE *e, EVP_PKEY *pkey) {
    if (g_fault == FAULT_VERIFY_INIT) return 0;
    return EVP_DigestVerifyInit(c, pctx, type, e, pkey);
}

int fault_EVP_DigestVerify(EVP_MD_CTX *c, const unsigned char *sig, size_t siglen,
                           const unsigned char *tbs, size_t tbslen) {
    if (g_fault == FAULT_VERIFY) return 0;
    return EVP_DigestVerify(c, sig, siglen, tbs, tbslen);
}

int fault_EVP_PKEY_derive_init(EVP_PKEY_CTX *c) {
    if (g_fault == FAULT_DERIVE_INIT) return 0;
    return EVP_PKEY_derive_init(c);
}

int fault_EVP_PKEY_derive_set_peer(EVP_PKEY_CTX *c, EVP_PKEY *peer) {
    if (g_fault == FAULT_DERIVE_PEER) return 0;
    return EVP_PKEY_derive_set_peer(c, peer);
}

int fault_EVP_PKEY_derive(EVP_PKEY_CTX *c, unsigned char *key, size_t *keylen) {
    int r;
    if (g_fault == FAULT_DERIVE) return 0;
    r = EVP_PKEY_derive(c, key, keylen);
    if (g_fault == FAULT_DERIVE_LEN) *keylen = 31u;
    return r;
}
