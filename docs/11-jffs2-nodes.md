# Volume 11: JFFS2 Nodes

**the log-structured filesystem and `crc32_le`**

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

## What You'll Learn

- Why JFFS2 is **log-structured** (there is no fixed directory tree on disk)
- The node types and the header layout
- The exact CRC (`crc32_le`) and why zlib's CRC fails
- The real `jffs2.c` and its AArch64 disassembly

## 1. The Model

JFFS2 has **no fixed directory tree**. Nodes are **appended** to erase blocks and the
tree is rebuilt at mount by scanning. Every node is CRC-protected, and writes are
**wear-levelled**. A newer write is a **new node**; the old node becomes obsolete. That
single fact is what makes the in-place patch (`docs/12`) possible.

## 2. Node Header

**Common header (12 bytes, little-endian):**

```
u16 magic   = 0x1985
u16 nodetype
u32 totlen
u32 hdr_crc
```

The first node of a fresh region is a **CLEANMARKER**:

```
00000000: 85 19 03 20  0c 00 00 00  b1 b0 1e e4
          |     |       |           +-- hdr_crc
          |     |       +-- totlen = 12
          |     +-- nodetype 0x2003 = CLEANMARKER
          +-- magic 0x1985 (bytes 85 19)
```

## 3. Node Types

| type | name | purpose |
| ---- | ---- | ------- |
| `0x2003` | CLEANMARKER | marks an erased block |
| `0x2004` | PADDING | filler |
| `0xE001` | DIRENT | name -> inode |
| `0xE002` | INODE | metadata + a data chunk |

## 4. The CRCs (the trap)

JFFS2 uses the **kernel `crc32_le`**, not zlib's CRC:

```
hdr_crc  = crc32_le(0, node[0:8])
node_crc = crc32_le(0, node[0:60])     ; for INODE/DIRENT
data_crc = crc32_le(0, data)
```

This is the **reflected** CRC (polynomial `0xEDB88320`, no init/final inversion). Using
zlib's `crc32()` produces a different value and the node **will not mount**. This is the
single most common JFFS2 mistake.

## 5. The Real Source - `include/jffs2.h`

```c
#define JFFS2_MAGIC 0x1985u
#define JFFS2_HDR_SIZE 12u
#define JFFS2_TYPE_DIRENT 0xE001u
#define JFFS2_TYPE_INODE 0xE002u
#define JFFS2_TYPE_CLEANMARKER 0x2003u
#define JFFS2_TYPE_PADDING 0x2004u

uint16_t jffs2_magic(const uint8_t *node);
uint16_t jffs2_type(const uint8_t *node);
uint32_t jffs2_totlen(const uint8_t *node);
bool     jffs2_hdr_crc_valid(const uint8_t *node);
```

## 6. The Real Source - `src/jffs2.c`

```c
uint16_t jffs2_magic(const uint8_t *node) {
    return jffs2_read16(node);
}

uint16_t jffs2_type(const uint8_t *node) {
    return jffs2_read16(&node[2]);
}

uint32_t jffs2_totlen(const uint8_t *node) {
    return jffs2_read32(&node[4]);
}

bool jffs2_hdr_crc_valid(const uint8_t *node) {
    return crc32_le(0u, node, 8u) == jffs2_read32(&node[8]);
}
```

## 7. The AArch64 Disassembly

```asm
jffs2_hdr_crc_valid:
    stp  x29, x30, [sp, #-32]!
    str  x19, [sp, #16]
    mov  x19, x0
    mov  w0, wzr            ; seed = 0
    mov  w2, #8             ; len = 8
    mov  x1, x19
    bl   crc32_le           ; crc32_le(0, node, 8)
    ldr  w8, [x19, #8]      ; stored hdr_crc
    cmp  w0, w8
    cset w0, eq
    ret
```

## 8. Extract It

```bash
jefferson -d rootfs CTF-XX-rootfs.img
```

`jefferson` walks the nodes, decompresses each file, and restores modes and symlinks.
(The CTF rootfs is small; a production one is thousands of nodes.)

## Labs

1. Hex-dump the first 16 bytes of `CTF-XX-rootfs.img`; identify the CLEANMARKER.
2. Compute `crc32_le(0, node[0:8])` and confirm it equals the stored `hdr_crc`.
3. Compute the same bytes with zlib's CRC and show it does **not** match.
4. Extract the rootfs with `jefferson` and list the tree.

## Reference

- `src/jffs2.c`, `include/jffs2.h`, `src/crc.c`
- `docs/03` (CRC), `docs/12` (in-place patch)
