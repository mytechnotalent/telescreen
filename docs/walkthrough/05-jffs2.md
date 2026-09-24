# Walkthrough 05: `jffs2`

**every line of `src/jffs2.c`, explained**

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

## Why This Module

`jffs2.c` reads a JFFS2 **node header** (`docs/11`). It is the smallest module in the lab
and the best place to see the `crc32_le` trap up close.

```
jffs2_read16 / jffs2_read32   (private)   little-endian loads
jffs2_magic   (public)   bytes 0..1
jffs2_type    (public)   bytes 2..3
jffs2_totlen  (public)   bytes 4..7
jffs2_hdr_crc_valid (public)  compare crc32_le(0, node, 8) with bytes 8..11
```

## 1. The Header

```
u16 magic   0x1985
u16 nodetype
u32 totlen
u32 hdr_crc
```

12 bytes total (`JFFS2_HDR_SIZE`). Little-endian.

## 2. The Loads

```c
static uint16_t jffs2_read16(const uint8_t *src) {
    return (uint16_t)((uint16_t)src[0] | ((uint16_t)src[1] << 8u));
}

static uint32_t jffs2_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}
```

Two sizes, same pattern. The `(uint16_t)` casts keep the 16-bit path from being promoted
to `int` and accidentally sign-extending.

## 3. The Three Readers

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
```

| function | offset | field |
| -------- | ------ | ----- |
| `jffs2_magic` | 0 | magic (must be `0x1985`) |
| `jffs2_type` | 2 | node type (`0x2003` CLEANMARKER, `0xE001` DIRENT, `0xE002` INODE) |
| `jffs2_totlen` | 4 | total node length, used to step to the next node |

`jffs2_totlen` is what a scanner uses to **advance**: read the length, add it to the
offset, repeat. Get it wrong and the scan walks off into garbage.

## 4. The Header CRC

```c
bool jffs2_hdr_crc_valid(const uint8_t *node) {
    return crc32_le(0u, node, 8u) == jffs2_read32(&node[8]);
}
```

| part | meaning |
| ---- | ------- |
| `crc32_le(0u, node, 8u)` | CRC over the first **8** bytes (magic, type, totlen) |
| `jffs2_read32(&node[8])` | the **stored** header CRC |
| `==` | valid only if they match |

Two facts to burn in:

1. The seed is **0** (`0u`) - JFFS2 uses the raw `crc32_le`, not the U-Boot variant
   (`docs/walkthrough/01`).
2. It covers **8** bytes, not 12. The CRC field itself is not part of its own input.

## 5. The AArch64 Disassembly

```asm
jffs2_hdr_crc_valid:
    stp  x29, x30, [sp, #-32]!
    str  x19, [sp, #16]
    mov  x19, x0            ; save node
    mov  w0, wzr            ; seed = 0
    mov  w2, #8             ; len = 8
    mov  x1, x19            ; &node[0]
    bl   crc32_le           ; crc32_le(0, node, 8)
    ldr  w8, [x19, #8]      ; load stored crc
    cmp  w0, w8
    cset w0, eq             ; w0 = (computed == stored)
    ldp  x29, x30, [sp], #32
    ret
```

Line by line, the assembly **is** the C:

- `mov w0, wzr` - the seed `0u`.
- `mov w2, #8` - the length `8u`.
- `bl crc32_le` - the call.
- `ldr w8, [x19, #8]` - read the stored CRC at offset 8.
- `cset w0, eq` - convert the comparison to a 0/1 boolean.

This is the module's whole lesson: **the C and the assembly are the same program.**
Once you can read one, you can read the other.

## 6. The Trap

If you computed the header CRC with zlib's `crc32()` instead of `crc32_le(0, ...)`, you
would get a different value and the node would be rejected at mount. The two differ only
by init/final and reflection; the check value table in `docs/walkthrough/01` is how you
catch it.

## 7. Tests

```bash
$ python3 scripts/run_tests.py | grep jffs2
:test_jffs2_header:PASS
:test_jffs2_crc:PASS
```

- `header` builds a CLEANMARKER and checks magic, type, and `totlen == 12`.
- `crc` checks a valid node verifies, then flips a byte and checks it fails.

## Exercises

1. Build a CLEANMARKER node and verify its header CRC by hand.
2. Change `totlen` and show the header CRC fails.
3. Explain why the CRC covers 8 bytes, not 12.
4. Compute the same bytes with zlib and show the mismatch.

## Reference

- `src/jffs2.c`, `include/jffs2.h`, `src/crc.c`
- `docs/11` (JFFS2 nodes), `docs/12` (in-place patch)
