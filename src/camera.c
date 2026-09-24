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
 * @brief printf-style formatter signature used to build a URL.
 */
typedef int (*camera_fmt_t)(char *, size_t, const char *, ...);

/**
 * @brief Indirection to the C library formatter.
 */
static const camera_fmt_t g_camera_fmt = snprintf;

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
    n = g_camera_fmt(out, out_len, "%s://%s:%u%s", scheme, ip, (unsigned)port, path);
    return (size_t)n < out_len;
}

bool camera_rtsp_url(const char *ip, uint16_t port, char *out, size_t out_len) {
    return camera_url(out, out_len, ip, port, "rtsp", CAMERA_RTSP_PATH);
}

bool camera_mjpeg_url(const char *ip, uint16_t port, char *out, size_t out_len) {
    return camera_url(out, out_len, ip, port, "http", CAMERA_MJPEG_PATH);
}
