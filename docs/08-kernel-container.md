# Volume 08: Kernel Container

**the vendor format, not zImage or uImage**

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

- The vendor kernel container format and why it is **not** a `zImage` or `uImage`
- The real `container.c` implementation
- How to parse the header and inflate the payload
- Why the container exists (staging, integrity, vendor metadata)

## 1. Why Not a Standard Image

Most people expect a kernel to be a `zImage` (magic `0x016F2818` at offset 0x24) or a
`uImage` (magic `0x27051956` at offset 0). The TELESCREEN kernel is **neither**. It is a
**vendor container**:

```
00000000: 21 84 1b 00   magic   0x001B8421
00000004: 00 f0 2c 00   length  (payload length field)
00000008: 67 7a 69 70 68 65 61 64   "gziphead"
00000010: 1f 8b 08 08   gzip stream begins (FNAME flag set)
0000001a: 49 6d 61 67 65 00   FNAME = "Image"
```

If you search for `0x016F2818` or `0x27051956` you will find nothing and wrongly conclude
the image is corrupt. The vendor tag **`gziphead`** is the giveaway.

## 2. The Real Source - `include/container.h`

```c
#define CONTAINER_MAGIC 0x001B8421u
#define CONTAINER_HDR_SIZE 16u

uint32_t container_magic(const uint8_t *buf);
uint32_t container_length(const uint8_t *buf);
bool     container_tag_ok(const uint8_t *buf);
bool     container_valid(const uint8_t *buf, size_t len);
```

## 3. The Real Source - `src/container.c`

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

bool container_valid(const uint8_t *buf, size_t len) {
    if ((buf == NULL) || (len < CONTAINER_HDR_SIZE)) return false;
    return (container_magic(buf) == CONTAINER_MAGIC) && container_tag_ok(buf);
}
```

## 4. Inflate the Payload

The gzip stream begins at offset **16** (after the 8-byte magic+length and the 8-byte
`gziphead` tag). Strip the 16-byte header and inflate:

```bash
dd if=CTF-XX-kernel.img bs=1 skip=16 | gunzip > Image
file Image        # ELF / ARM64 "Image"
```

The result is the raw bootable kernel image U-Boot jumps to.

## 5. Rebuild the Container

`scripts/build_images.py` wraps a kernel in the container:

```python
def container(payload: bytes) -> bytes:
    gz = gzip.compress(payload)
    head = struct.pack("<I", CONTAINER_MAGIC) + struct.pack("<I", len(payload))
    return head + b"gziphead" + gz
```

Note the length field records the **uncompressed** payload length, not the compressed
size. Getting that backwards is a common mistake when rebuilding.

## 6. Why a Container

The container gives U-Boot a cheap way to:

- recognise the payload (`magic`) and staging length (`length`);
- skip metadata (`gziphead`);
- inflate with one pass (`hw_decompress_v1` on the camera; `gunzip` on the host).

## Labs

1. Parse the container header and record magic, length, and tag.
2. Inflate the payload and record the raw image size.
3. Rebuild a container from a small payload and confirm `container_valid` passes.
4. Explain why a `zImage` magic search fails on this kernel.

## Reference

- `src/container.c`, `include/container.h`
- `scripts/build_images.py`
- `docs/09` (device tree), `docs/10` (cmdline and mount)
