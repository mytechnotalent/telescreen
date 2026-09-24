# Volume 02: The Four Partitions

**the model, the source, the disassembly, and the bytes**

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

- The four-partition model and why embedded images are laid out this way
- The exact layout the TELESCREEN uses
- The real C implementation (`partition.c`) and its AArch64 disassembly
- How to identify each partition by **magic bytes**, not by filename

## 1. The Model

An embedded image is almost always a **small, fixed set of regions**: a bootloader, a
bootloader environment, a kernel, and a root filesystem. The bootloader declares them
in a single string, `mtdparts`, and every tool downstream depends on it.

```
mtdparts=sfc:128K(boot),64K(bootargs),1792K(kernel),14400K(rootfs)
```

Decoding that string:

| mtd | name       | offset     | size        | end        |
| --- | ---------- | ---------- | ----------- | ---------- |
| 0   | `boot`     | `0x000000` | `0x020000`  | `0x020000` |
| 1   | `bootargs` | `0x020000` | `0x010000`  | `0x030000` |
| 2   | `kernel`   | `0x030000` | `0x1C0000`  | `0x1F0000` |
| 3   | `rootfs`   | `0x1F0000` | `0xE10000`  | `0x1000000`|

Coverage: `0x1F0000 + 0xE10000 = 0x1000000` - the whole 16 MiB image. Notice the
**1984 KiB pad** between the end of `kernel` and the start of `rootfs`
(`0x1F0000 - 0x030000 - 0x1C0000`). It is deliberate; do not "fix" it.

## 2. The Source - `include/partition.h`

The layout is data, not magic numbers scattered through the code:

```c
#define PART_IMAGE_SIZE 0x1000000u
#define PART_BOOT_OFF 0x000000u
#define PART_BOOT_SIZE 0x020000u
#define PART_ENV_OFF 0x020000u
#define PART_ENV_SIZE 0x010000u
#define PART_KERNEL_OFF 0x030000u
#define PART_KERNEL_SIZE 0x1C0000u
#define PART_ROOTFS_OFF 0x1F0000u
#define PART_ROOTFS_SIZE 0xE10000u
#define PART_BOOT_MAGIC 0xEA000515u
#define PART_CONTAINER_MAGIC 0x001B8421u
#define PART_JFFS2_MAGIC 0x1985u
```

## 3. The Source - `src/partition.c` (identification)

The finding function is short and total: read the first bytes, match a magic, return a
kind. There is **no guess** based on filename or size.

```c
part_kind_t part_identify(const uint8_t *buf, size_t len) {
    if ((buf == NULL) || (len < 4u)) return PART_KIND_UNKNOWN;
    if (part_read32(buf) == PART_BOOT_MAGIC) return PART_KIND_BOOT;
    if (part_read16(buf) == PART_JFFS2_MAGIC) return PART_KIND_ROOTFS;
    if (part_read32(buf) == PART_CONTAINER_MAGIC) return PART_KIND_KERNEL;
    return PART_KIND_ENV;
}
```

And carving is a bounds-checked copy:

```c
part_result_t part_carve(const uint8_t *image, size_t image_size, part_kind_t kind,
                         uint8_t *out, size_t out_size) {
    const part_desc_t *desc = part_desc_for(kind);
    if ((image == NULL) || (out == NULL)) return PART_RESULT_BAD_ARGUMENT;
    if (image_size < PART_IMAGE_SIZE) return PART_RESULT_TOO_SMALL;
    if (desc == NULL) return PART_RESULT_BAD_ARGUMENT;
    if (out_size < desc->size) return PART_RESULT_TOO_SMALL;
    memcpy(out, &image[desc->offset], desc->size);
    return PART_RESULT_OK;
}
```

## 4. The AArch64 Disassembly

This is the whole point of the project: the same logic, on real silicon, as the RP5
executes it. Compile the freestanding CRC module for the RP5 and read the assembly:

```bash
clang --target=aarch64-unknown-linux-gnu -O2 -S src/jffs2.c -Iinclude -o jffs2.s
```

The header-CRC verification lowers to a single `bl crc32_le`, a load, and a compare:

```asm
jffs2_hdr_crc_valid:
    stp  x29, x30, [sp, #-32]!
    str  x19, [sp, #16]
    mov  x29, sp
    mov  x19, x0            ; save node pointer
    mov  w0, wzr            ; seed = 0
    mov  w2, #8             ; len = 8
    mov  x1, x19            ; &node[0]
    bl   crc32_le           ; crc32_le(0, node, 8)
    ldr  w8, [x19, #8]      ; load stored hdr_crc
    cmp  w0, w8
    cset w0, eq             ; return (computed == stored)
    ldp  x29, x30, [sp], #32
    ret
```

And `crc32_le` itself is vectorized by the compiler (the `dup v2.4s` / `and v2.16b`):

```asm
crc32_le:
    cbz  x2, .LBB0_3
    adrp x8, .LCPI0_0
    adrp x9, .LCPI0_1
    ldr  q0, [x8, :lo12:.LCPI0_0]
    ldr  q1, [x9, :lo12:.LCPI0_1]
    mov  w8, #33568
    movk w8, #60856, lsl #16     ; 0xEDB88320 polynomial, low half
    ...
```

**Read this carefully**: the polynomial `0xEDB88320` is the *reflected* CRC-32 used by
JFFS2. Seeing it in the assembly is how you confirm which CRC a function computes.

## 5. Command Output (real)

```bash
$ python3 scripts/carve.py --image CTF-XX-full.img --out carved/
boot      off=0x000000 size=0x020000 wrote 131072 bytes
bootargs  off=0x020000 size=0x010000 wrote 65536 bytes
kernel    off=0x030000 size=0x1C0000 wrote 1835008 bytes
rootfs    off=0x1F0000 size=0xE10000 wrote 14745600 bytes
```

```bash
$ python3 scripts/verify_telescreen.py --image CTF-XX-full.img
boot      magic ok
kernel    magic ok
rootfs    magic ok
```

## 6. Identification Table

| partition | first bytes | meaning |
| --------- | ----------- | ------- |
| `boot` | `15 05 00 ea` | ARM branch at offset 0 (reset vector) |
| `bootargs` | CRC32 (4 B LE) then `k=v\0` | U-Boot environment |
| `kernel` | `21 84 1b 00` then `gziphead` | vendor container |
| `rootfs` | `85 19 03 20` | JFFS2 little-endian + CLEANMARKER |

## Labs

1. Carve the four partitions and `xxd` the first 16 bytes of each. Match the table.
2. Hash each partition with SHA-256 and record the values.
3. Compile `src/jffs2.c` for AArch64 and confirm the `0xEDB88320` constant appears.
4. Deliberately flip a byte in `boot.img`'s magic and show `part_identify` fails.

## Reference

- `src/partition.c`, `include/partition.h`
- `scripts/carve.py`, `scripts/verify_telescreen.py`
- `docs/03` (the image store), `docs/04` (carve and verify)
