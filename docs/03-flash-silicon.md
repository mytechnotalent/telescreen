# Volume 03: The Image Store

**four fixed regions and integrity**

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

- Why an RP5 has no SPI-NOR and what replaces it
- The four fixed regions, laid out identically to a camera's flash
- The two CRCs in play and why confusing them is the classic failure
- The real `crc.c` and its AArch64 code

## 1. From Silicon to Image Store

A camera stores firmware in **SPI-NOR flash** (a 16 MiB chip). An RP5 has **no
SPI-NOR**: it boots from a **microSD card or NVMe**. So the TELESCREEN keeps the *same
four-region layout* but stores it as an **image file / raw regions** on the RP5's
image store.

| region    | offset     | size        | contents |
| --------- | ---------- | ----------- | -------- |
| `boot`    | `0x000000` | 128 KiB     | U-Boot |
| `bootargs`| `0x020000` | 64 KiB      | U-Boot environment (CRC32) |
| `kernel`  | `0x030000` | 1792 KiB    | kernel container |
| `rootfs`  | `0x1F0000` | 14400 KiB   | JFFS2 |

This is the same table as `docs/02`. The lesson is the **layout and integrity
discipline**, not the specific silicon.

## 2. The Two CRCs

Two different CRC algorithms appear, and mixing them up is the single most common bug
in firmware work:

| layer | algorithm | coverage |
| ----- | --------- | -------- |
| U-Boot environment | standard CRC32 (init `0xFFFFFFFF`, final xor) | the `key=value\0` blob |
| JFFS2 node | **`crc32_le`** (reflected, **no** init/final inversion) | `hdr_crc`, `node_crc`, `data_crc` |

A CRC is **integrity, not authenticity**. It detects accidental corruption. It does
**not** stop an attacker - that is a separate lesson (`docs/18`, AEAD).

## 3. The Real Source - `include/crc.h`

```c
#define CRC16_INIT 0xFFFFu

uint32_t crc32_le(uint32_t seed, const uint8_t *buf, size_t len);
uint32_t crc32_uboot(const uint8_t *buf, size_t len);
uint16_t crc16_ccitt(const uint8_t *buf, size_t len);
```

`crc32_uboot` is simply the reflected core with the standard init/final:

```c
uint32_t crc32_uboot(const uint8_t *buf, size_t len) {
    return crc32_le(0xFFFFFFFFu, buf, len) ^ 0xFFFFFFFFu;
}
```

## 4. The Real Source - `src/crc.c`

The reflected byte fold:

```c
static uint32_t crc32_byte(uint32_t crc, uint8_t byte) {
    uint8_t i;
    crc ^= (uint32_t)byte;
    for (i = 0u; i < 8u; ++i) {
        crc = (crc >> 1u) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1u)));
    }
    return crc;
}
```

The polynomial is `0xEDB88320`, the reflected form of `0x04C11DB7`.

## 5. The AArch64 Disassembly

Compiled for the RP5, `crc32_le` is vectorized. The polynomial shows up as two
`movk`-built constants:

```asm
crc32_le:
    cbz  x2, .LBB0_3
    adrp x8, .LCPI0_0
    adrp x9, .LCPI0_1
    ldr  q0, [x8, :lo12:.LCPI0_0]
    ldr  q1, [x9, :lo12:.LCPI0_1]
    mov  w8, #33568                 ; 0x8320
    mov  w9, #16784                 ; 0x4190
    movk w8, #60856, lsl #16        ; 0xEDB88320
    movk w9, #30428, lsl #16
.LBB0_2:
    ldrb w10, [x1], #1
    subs x2, x2, #1
    eor  w10, w0, w10
    dup  v2.4s, w10
    ...
```

Reading `0xEDB88320` in the disassembly is how you *prove* which CRC a function
implements, without source.

## 6. Verify the Check Values

```bash
$ python3 scripts/run_tests.py | grep crc
:test_crc32_uboot_check:PASS
:test_crc16_check:PASS
```

- `crc32_uboot("123456789")` = `0xCBF43926` (the IEEE 802.3 check value).
- `crc16_ccitt("123456789")` = `0x29B1` (the CRC-16/CCITT-FALSE check value).

If either is wrong, every downstream integrity check is wrong. This is why the test
suite pins them to published check values.

## 7. Why "Three Identical Reads"

The same discipline that finds CRCs finds a **bad read**. On a camera you read the
flash three times and require byte-identical results; on the RP5 you hash the image and
re-read. A single read is never trustworthy.

## Labs

1. Compute `crc32_uboot` and `crc16_ccitt` of `"123456789"`; confirm the check values.
2. Compile `src/crc.c` for AArch64 and find `0xEDB88320` in the assembly.
3. Explain, in one paragraph, why a CRC does not stop an attacker.

## Reference

- `src/crc.c`, `include/crc.h`
- `docs/07` (U-Boot environment), `docs/11` (JFFS2 `crc32_le`)
