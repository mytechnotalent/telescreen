# Walkthrough 04: `container`

**every line of `src/container.c`, explained**

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

`container.c` recognises the **vendor kernel container** (`docs/08`): a header, a length,
a `gziphead` tag, then a gzip stream named `Image`. It exists so that a tool can tell a
real container from noise before inflating it.

```
cont_read32      (private)   little-endian word load
container_magic  (public)    read the magic
container_length (public)    read the length field
container_tag_ok (public)    verify "gziphead"
container_valid  (public)    magic AND tag
```

## 1. The Bytes

```
21 84 1b 00   magic 0x001B8421
00 f0 2c 00   length
67 7a 69 70 68 65 61 64   "gziphead"
1f 8b 08 08 ...            gzip stream
```

`CONTAINER_MAGIC` is `0x001B8421`; `CONTAINER_HDR_SIZE` is `16u` (`include/container.h`).

## 2. `cont_read32`

```c
static uint32_t cont_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}
```

The same portable little-endian load used across the project.

## 3. The Three Accessors

```c
uint32_t container_magic(const uint8_t *buf) {
    return cont_read32(buf);
}

uint32_t container_length(const uint8_t *buf) {
    return cont_read32(&buf[4]);
}

bool container_tag_ok(const uint8_t *buf) {
    return memcmp(&buf[8], "gziphead", 8u) == 0;
}
```

| function | offset | reads |
| -------- | ------ | ----- |
| `container_magic` | 0 | the 32-bit magic |
| `container_length` | 4 | the 32-bit length |
| `container_tag_ok` | 8 | the 8-byte ASCII tag |

These are **trusting** accessors: they assume the buffer is at least 16 bytes. That is
why they are private-ish in intent and the caller must go through `container_valid`
first. In C you enforce that by convention and by the doc comment.

## 4. `container_valid` - the gate

```c
bool container_valid(const uint8_t *buf, size_t len) {
    if ((buf == NULL) || (len < CONTAINER_HDR_SIZE)) return false;
    return (container_magic(buf) == CONTAINER_MAGIC) && container_tag_ok(buf);
}
```

| line | what it does |
| ---- | ------------ |
| guard | null or shorter than 16 bytes -> false |
| `&&` | **both** the magic and the tag must match |

Two independent checks: a random 4-byte match is 1-in-2^32, and requiring the ASCII tag
as well makes a false positive astronomically unlikely. This is the pattern for "is this
really the format I think it is" - **more than one field**. A single magic is easy to
stumble onto; magic + tag is not.

## 5. Why Not Just `gunzip`

You can hand `dd if=kernel.img bs=1 skip=16 | gunzip` a stream and let gzip complain.
But the lab wants to **identify** the format *before* touching it:

- to reject the wrong partition early;
- to record the magic/length for the write-up;
- to teach that a container is `[magic][len][tag][payload]`, not a magic.

## 6. AArch64

`container_valid` compiles to two loads, two compares, and a `memcmp` call for the tag.
The `&&` short-circuits: if the magic fails, the tag is never compared.

## 7. Tests

```bash
$ python3 scripts/run_tests.py | grep container
:test_container_fields:PASS
:test_container_invalid:PASS
```

- `fields` builds the canonical 16-byte header and checks magic, length
  (`0x002CF000`), tag, and `container_valid`.
- `invalid` checks `NULL`, a short buffer, and a corrupted magic all return `false`.

## 8. Rebuilding

`scripts/build_images.py` builds the container in Python (`docs/26`):

```python
def container(payload: bytes) -> bytes:
    gz = gzip.compress(payload)
    head = struct.pack("<I", CONTAINER_MAGIC) + struct.pack("<I", len(payload))
    return head + b"gziphead" + gz
```

Read and rebuild are mirror images: C validates, Python constructs.

## Exercises

1. Parse the container header of `CTF-XX-kernel.img` and record magic, length, tag.
2. Explain why `container_valid` checks **two** fields.
3. Rebuild a container from a small payload and confirm `container_valid` accepts it.
4. Show that a wrong magic fails even with a correct tag.

## Reference

- `src/container.c`, `include/container.h`
- `docs/08` (kernel container), `docs/26` (building images)
