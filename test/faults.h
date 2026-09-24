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
// File:    faults.h
// Desc:    Fault-injection selector for the native test suite. A test sets
//          g_fault to make the next matching wrapped OpenSSL call fail, which
//          exercises the error paths that no normal input can reach.
// Created: 2026

#ifndef TELESCREEN_FAULTS_H
#define TELESCREEN_FAULTS_H

/**
 * @brief Selectable fault-injection targets (0 = normal operation).
 */
enum {
    FAULT_NONE = 0,
    FAULT_CTX_NEW,
    FAULT_MD_CTX_NEW,
    FAULT_ENC_INIT,
    FAULT_ENC_UPDATE,
    FAULT_ENC_FINAL,
    FAULT_DEC_INIT,
    FAULT_DEC_UPDATE,
    FAULT_DEC_FINAL,
    FAULT_CTRL,
    FAULT_HMAC,
    FAULT_GET_PUB,
    FAULT_SIGN_INIT,
    FAULT_SIGN,
    FAULT_VERIFY_INIT,
    FAULT_VERIFY,
    FAULT_DERIVE_INIT,
    FAULT_DERIVE_PEER,
    FAULT_DERIVE,
    FAULT_GET_PUB_LEN,
    FAULT_SIGN_LEN,
    FAULT_DERIVE_LEN
};

/**
 * @brief The fault to inject on the next matching call.
 */
extern int g_fault;

#endif
