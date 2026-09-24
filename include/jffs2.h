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
// File:    jffs2.h
// Desc:    Declares the JFFS2 node header helpers for the TELESCREEN rootfs:
//          magic, node type, total length, and the crc32_le-protected header CRC.
// Created: 2026

#ifndef JFFS2_H
#define JFFS2_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief JFFS2 little-endian magic value.
 */
#define JFFS2_MAGIC 0x1985u

/**
 * @brief Common JFFS2 node header size in bytes.
 */
#define JFFS2_HDR_SIZE 12u

/**
 * @brief JFFS2 DIRENT node type.
 */
#define JFFS2_TYPE_DIRENT 0xE001u

/**
 * @brief JFFS2 INODE node type.
 */
#define JFFS2_TYPE_INODE 0xE002u

/**
 * @brief JFFS2 CLEANMARKER node type.
 */
#define JFFS2_TYPE_CLEANMARKER 0x2003u

/**
 * @brief JFFS2 PADDING node type.
 */
#define JFFS2_TYPE_PADDING 0x2004u

/**
 * @brief Read the little-endian magic from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint16_t Parsed magic value.
 */
uint16_t jffs2_magic(const uint8_t *node);

/**
 * @brief Read the little-endian node type from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint16_t Parsed node type.
 */
uint16_t jffs2_type(const uint8_t *node);

/**
 * @brief Read the little-endian total length from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint32_t Parsed total length in bytes.
 */
uint32_t jffs2_totlen(const uint8_t *node);

/**
 * @brief Verify the crc32_le header CRC of a node.
 *
 * The stored header CRC covers the first eight bytes of the node.
 *
 * @param node Pointer to the node header bytes.
 * @return bool true when the stored header CRC matches the computed value.
 */
bool jffs2_hdr_crc_valid(const uint8_t *node);

#endif // JFFS2_H
