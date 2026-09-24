# Walkthrough 02: `partition`

**every line of `src/partition.c`, explained**

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

`partition.c` turns the `mtdparts` idea into code: it knows the four regions, identifies
a buffer by its magic, verifies a whole-image layout, and carves one region out. Every
carve and verify lab calls it.

```
part_read32 / part_read16   (private)   little-endian loads
part_end                    (private)   end offset of a slot
part_desc_for               (public)    slot -> descriptor
part_identify               (public)    bytes -> partition kind
part_verify_layout          (public)    whole-image sanity
part_carve                  (public)    copy one region out
```

## 1. The Table

```c
static const part_desc_t g_parts[4] = {
    { PART_KIND_BOOT,   PART_BOOT_OFF,   PART_BOOT_SIZE,   "boot" },
    { PART_KIND_ENV,    PART_ENV_OFF,    PART_ENV_SIZE,    "bootargs" },
    { PART_KIND_KERNEL, PART_KERNEL_OFF, PART_KERNEL_SIZE, "kernel" },
    { PART_KIND_ROOTFS, PART_ROOTFS_OFF, PART_ROOTFS_SIZE, "rootfs" },
};
```

The layout is **data**. The offsets and sizes come from `include/partition.h`
(`docs/02`). A `part_desc_t` is `{kind, offset, size, name}`.

## 2. `part_read32` / `part_read16`

```c
static uint32_t part_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}
```

| part | why |
| ---- | --- |
| `src[0]` | lowest byte, no shift |
| `<< 8`, `<< 16`, `<< 24` | build the little-endian word |

Reading bytes explicitly (not `*(uint32_t *)src`) avoids **alignment faults** on ARM:
an unaligned word load can trap. Byte-wise is safe and portable.

## 3. `part_end`

```c
static size_t part_end(size_t i) {
    return (size_t)g_parts[i].offset + (size_t)g_parts[i].size;
}
```

Used by `part_verify_layout` to check each region fits in the image. Casting to
`size_t` avoids 32-bit overflow on the sum.

## 4. `part_desc_for`

```c
const part_desc_t *part_desc_for(part_kind_t kind) {
    size_t i;
    for (i = 0u; i < 4u; ++i) {
        if (g_parts[i].kind == kind) return &g_parts[i];
    }
    return NULL;
}
```

A linear scan of four entries. Returns `NULL` for an unknown kind, so callers must check
(`part_carve` does). Returning a `const` pointer means callers cannot corrupt the table.

## 5. `part_identify` - the heart

```c
part_kind_t part_identify(const uint8_t *buf, size_t len) {
    if ((buf == NULL) || (len < 4u)) return PART_KIND_UNKNOWN;
    if (part_read32(buf) == PART_BOOT_MAGIC) return PART_KIND_BOOT;
    if (part_read16(buf) == PART_JFFS2_MAGIC) return PART_KIND_ROOTFS;
    if (part_read32(buf) == PART_CONTAINER_MAGIC) return PART_KIND_KERNEL;
    return PART_KIND_ENV;
}
```

| line | condition | result |
| ---- | --------- | ------ |
| 1 | null or short | `PART_KIND_UNKNOWN` |
| 2 | `0xEA000515` | `PART_KIND_BOOT` |
| 3 | `0x1985` | `PART_KIND_ROOTFS` |
| 4 | `0x001B8421` | `PART_KIND_KERNEL` |
| 5 | anything else | `PART_KIND_ENV` (the fallback) |

The ENV case is the **default**: a U-Boot environment has no fixed magic (it is CRC +
text), so it is "everything that is not the other three". That is a deliberate choice;
it means identification is total (always returns a kind).

**Order matters.** Boot is checked first because its magic is a full word; JFFS2 next
because it is a 16-bit magic that could collide with the low half of some word; kernel
third. Reorder carelessly and you misidentify.

## 6. `part_verify_layout`

```c
part_result_t part_verify_layout(size_t image_size) {
    size_t i;
    if (image_size < PART_IMAGE_SIZE) return PART_RESULT_TOO_SMALL;
    for (i = 0u; i < 4u; ++i) {
        if (part_end(i) > PART_IMAGE_SIZE) return PART_RESULT_UNKNOWN;
    }
    return PART_RESULT_OK;
}
```

Two checks:

1. the image is at least 16 MiB;
2. no region runs past the end.

The second is redundant if the table is correct, but it is the check that catches a
**future table edit** that breaks the invariant. Defensive programming on data.

## 7. `part_carve`

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

Every failure mode is a distinct return code:

| condition | code |
| --------- | ---- |
| null image or out | `BAD_ARGUMENT` |
| image too small | `TOO_SMALL` |
| unknown kind (`desc == NULL`) | `BAD_ARGUMENT` |
| output buffer too small | `TOO_SMALL` |
| success | `OK` |

The `out_size < desc->size` check is the **caller-side overflow guard**: without it,
`memcpy` would write past `out`. This is exactly the class of bug that ships in real
firmware.

## 8. AArch64

```asm
part_identify:
    cbz  x1, .Lunknown        ; len == 0
    cmp  x1, #4
    b.lo .Lunknown            ; len < 4
    ldr  w8, [x0]             ; first word
    mov  w9, #...             ; 0xEA000515
    cmp  w8, w9
    b.eq .Lboot
    ...
```

The `ldr w8, [x0]` is a **word load** because `part_identify` reads through a pointer the
caller guarantees is aligned (the image base). The byte-wise `part_read32` is used where
alignment is not guaranteed.

## 9. Tests

```bash
$ python3 scripts/run_tests.py | grep part
:test_part_desc_for_boot:PASS
:test_part_desc_for_unknown:PASS
:test_part_identify_boot:PASS
:test_part_identify_kernel:PASS
:test_part_identify_rootfs:PASS
:test_part_identify_env:PASS
:test_part_identify_unknown:PASS
:test_part_verify_layout:PASS
:test_part_carve_ok:PASS
:test_part_carve_bad_args:PASS
:test_part_carve_too_small:PASS
:test_part_carve_unknown_kind:PASS
```

Every branch of `part_identify` and every error code of `part_carve` has a test - that
is why the module reaches **100% line coverage**.

## Exercises

1. Add a fifth region to the table and update `part_verify_layout`'s loop bound safely.
2. Explain why `part_identify` returns ENV as the default.
3. Show that `part_carve` with `out_size == desc->size` succeeds and `- 1` fails.
4. Reorder the magic checks and describe a buffer that would be misidentified.
5. Find the `memcpy` length and state the overflow it prevents.

## Reference

- `src/partition.c`, `include/partition.h`
- `docs/02` (the model), `docs/04` (carve/verify)
