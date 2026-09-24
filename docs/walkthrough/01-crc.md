# Walkthrough 01: `crc`

**every line of `src/crc.c`, explained**

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

## Why This Module First

`crc.c` is the module everything else depends on. The U-Boot environment validates with
one CRC, JFFS2 with another, and telemetry frames with a third. If `crc.c` is wrong,
every integrity check in the lab is wrong. So we read it line by line.

The file is tiny: three public functions and two private helpers.

```
crc32_byte      (private)  fold one byte into a reflected CRC-32
crc16_byte      (private)  fold one byte into a CRC-16/CCITT
crc32_le        (public)   the JFFS2 CRC
crc32_uboot     (public)   the U-Boot environment CRC
crc16_ccitt     (public)   the telemetry frame CRC
```

## 1. `crc32_byte` - the reflected CRC-32 core

```c
44  static uint32_t crc32_byte(uint32_t crc, uint8_t byte) {
45      uint8_t i;
46      crc ^= (uint32_t)byte;
47      for (i = 0u; i < 8u; ++i) {
48          crc = (crc >> 1u) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1u)));
49      }
50      return crc;
51  }
```

| line | code | what it does |
| ---- | ---- | ------------ |
| 44 | `static uint32_t crc32_byte(uint32_t crc, uint8_t byte)` | takes the running CRC and one byte; `static` = file-local |
| 45 | `uint8_t i;` | the bit counter, 0..7 |
| 46 | `crc ^= (uint32_t)byte;` | XOR the byte into the low 8 bits of the running CRC |
| 47 | `for (i = 0u; i < 8u; ++i)` | process **eight** bits, one per iteration |
| 48 | `crc = (crc >> 1u) ^ (0xEDB88320u & ...)` | shift right by one; if the bit that fell off was `1`, XOR the polynomial |
| 50 | `return crc;` | the residual after eight bits |

Line 48 is the whole algorithm. Break it apart:

- `crc & 1u` - the **low bit**, i.e. the bit falling off the right end.
- `-(int32_t)(crc & 1u)` - arithmetic negation, producing `0xFFFFFFFF` when the bit is
  `1` and `0` when it is `0`. This is a **branchless mask**.
- `0xEDB88320u & mask` - the polynomial, applied only when the bit was set.
- `crc >> 1u` - the right shift that makes this the **reflected** variant.

Why branchless? A data-dependent `if` is a **timing side-channel**. The mask turns the
choice into an arithmetic AND, so the loop runs the same instructions regardless of the
data. For integrity CRCs this is not security-critical, but it is the habit you want.

> The polynomial `0xEDB88320` is the reflected form of `0x04C11DB7` (the Ethereum/Ethernet
> polynomial). Reflected means bits are processed **LSB-first**: shift right, test the
> low bit.

### AArch64

```asm
crc32_le:                       ; the compiler inlines crc32_byte here
    cbz  x2, .LBB0_3            ; len == 0 -> return seed
    ...
    mov  w8, #33568             ; 0x8320
    movk w8, #60856, lsl #16    ; 0xEDB88320
```

The constant `0xEDB88320` in the listing is the proof of which CRC this is. See it and
you know: reflected CRC-32.

## 2. `crc16_byte` - the CRC-16 core

```c
60  static uint16_t crc16_byte(uint16_t crc, uint8_t byte) {
61      uint8_t i;
62      uint16_t inbit;
63      for (i = 0u; i < 8u; ++i) {
64          inbit = (uint16_t)((crc ^ ((uint16_t)byte << 8u)) & 0x8000u);
65          crc = (uint16_t)(crc << 1u);
66          if (inbit != 0u) crc ^= 0x1021u;
67          byte = (uint8_t)(byte << 1u);
68      }
69      return crc;
70  }
```

| line | code | what it does |
| ---- | ---- | ------------ |
| 60 | `static uint16_t crc16_byte(uint16_t crc, uint8_t byte)` | running CRC-16 + one byte |
| 64 | `inbit = (crc ^ (byte << 8)) & 0x8000` | take the **top bit** (the byte is placed in the high half) |
| 65 | `crc = crc << 1` | shift left - **non-reflected** (MSB-first) |
| 66 | `if (inbit) crc ^= 0x1021` | apply the CRC-16/CCITT polynomial |
| 67 | `byte = byte << 1` | advance the byte's bit window |

The contrast with `crc32_byte` is the lesson: CRC-32 here is **reflected** (shift right,
test low bit, `0xEDB88320`), CRC-16 here is **non-reflected** (shift left, test high bit,
`0x1021`). Confusing the two directions is the classic CRC bug.

## 3. `crc32_le` - the JFFS2 CRC

```c
80  uint32_t crc32_le(uint32_t seed, const uint8_t *buf, size_t len) {
81      uint32_t crc = seed;
82      size_t i;
83      for (i = 0u; i < len; ++i) crc = crc32_byte(crc, buf[i]);
84      return crc;
85  }
```

| line | code | what it does |
| ---- | ---- | ------------ |
| 80 | `uint32_t crc32_le(uint32_t seed, const uint8_t *buf, size_t len)` | caller supplies the **seed** (JFFS2 uses 0) |
| 81 | `uint32_t crc = seed;` | initialise with the seed |
| 83 | `for (...) crc = crc32_byte(crc, buf[i]);` | fold every byte |

This is the **kernel `crc32_le`** exactly: reflected, caller-seeded, **no** final
inversion. JFFS2 computes `hdr_crc = crc32_le(0, node, 8)` with it (`docs/11`).

## 4. `crc32_uboot` - the environment CRC

```c
94  uint32_t crc32_uboot(const uint8_t *buf, size_t len) {
95      return crc32_le(0xFFFFFFFFu, buf, len) ^ 0xFFFFFFFFu;
96  }
```

| line | code | what it does |
| ---- | ---- | ------------ |
| 95 | `crc32_le(0xFFFFFFFF, buf, len) ^ 0xFFFFFFFF` | the standard CRC-32: init `0xFFFFFFFF`, final inversion |

The whole difference between JFFS2 and U-Boot is **this one line**. Same core, different
init/final. That is why you can compute both from one primitive.

## 5. `crc16_ccitt` - the telemetry CRC

```c
105 uint16_t crc16_ccitt(const uint8_t *buf, size_t len) {
106     uint16_t crc = CRC16_INIT;
107     size_t i;
108     for (i = 0u; i < len; ++i) crc = crc16_byte(crc, buf[i]);
109     return crc;
110 }
```

`CRC16_INIT` is `0xFFFF` (`include/crc.h`).

## 6. The Two Check Values

```bash
$ python3 scripts/run_tests.py | grep crc
:test_crc32_uboot_check:PASS
:test_crc32_le_empty:PASS
:test_crc16_check:PASS
```

| function | input | expected | why |
| -------- | ----- | -------- | --- |
| `crc32_uboot` | `"123456789"` | `0xCBF43926` | IEEE 802.3 published check value |
| `crc16_ccitt` | `"123456789"` | `0x29B1` | CRC-16/CCITT-FALSE check value |
| `crc32_le` | any, len 0 | the seed | no input -> no change |

If your CRC does not match the published check value, it is wrong - do not proceed.

## 7. Line Map (summary)

| lines | entity |
| ----- | ------ |
| 32 | `#include "crc.h"` |
| 44-51 | `crc32_byte` |
| 60-70 | `crc16_byte` |
| 80-85 | `crc32_le` |
| 94-96 | `crc32_uboot` |
| 105-110 | `crc16_ccitt` |

## Exercises

1. Re-derive `0xEDB88320` from `0x04C11DB7` by reversing the bit order.
2. Trace `crc32_byte(0, 0x31)` for the byte `'1'` (`0x31`) by hand for eight iterations.
3. Explain why `crc32_uboot` and `crc32_le` differ by a single line.
4. Prove that `crc32_le(seed, buf, 0) == seed` from the loop structure.
5. Run the two check values and record them.

## Reference

- `src/crc.c`, `include/crc.h`
- `docs/03` (the image store), `docs/07` (environment), `docs/11` (JFFS2)
