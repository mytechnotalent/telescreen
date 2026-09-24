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
