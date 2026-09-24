# Walkthrough 28: Writing a JFFS2 Parser

**build the tool the vendor did not give you, line by line**

***
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. 

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:

1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws concerning cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**

***

## Why Write a Parser

`jefferson` extracts a JFFS2 tree, but understanding the format means you can read a node
the tool rejects, or patch one the tool will not. This volume builds a minimal parser in
Python, one line at a time. The result reads the same nodes as `src/jffs2.c`
(`docs/walkthrough/05`), in a language you can run in seconds.

## 1. The Node Header

```
u16 magic    0x1985
u16 nodetype
u32 totlen
u32 hdr_crc
```

```python
import struct

MAGIC = 0x1985

def read_header(d, off):
    magic, ntype, totlen, hcrc = struct.unpack_from("<HHII", d, off)
    return magic, ntype, totlen, hcrc
```

`struct.unpack_from("<HHII", d, off)` reads `u16, u16, u32, u32` little-endian (`<`) at
`off`. This is the whole header in one call.

## 2. The CRC

JFFS2 uses the **reflected** `crc32_le` with seed 0 (`docs/walkthrough/01`). Python's
`zlib.crc32` is **not** the same. Implement it:

```python
def crc32_le(seed, data):
    crc = seed & 0xFFFFFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ (0xEDB88320 if crc & 1 else 0)
    return crc & 0xFFFFFFFF
```

Same loop as `crc32_byte` in C: XOR the byte, then eight times shift right and conditionally
XOR the polynomial.

## 3. Validate a Header

```python
def header_ok(d, off):
    magic, ntype, totlen, hcrc = read_header(d, off)
    if magic != MAGIC:
        return False
    if 12 <= totlen <= (len(d) - off):
        return hcrc == crc32_le(0, d[off:off+8])
    return False
```

A node is valid when the magic matches, the length is sane, and the stored CRC equals
`crc32_le(0, header[0:8])`. The length check `12 <= totlen <= remaining` prevents a
bogus `totlen` from walking the scan off the end - the same guard `part_carve` uses.

## 4. Scan the Image

```python
def scan(d):
    off = 0
    while off < len(d) - 12:
        if d[off] == 0x85 and d[off+1] == 0x19 and header_ok(d, off):
            _, ntype, totlen, _ = read_header(d, off)
            yield off, ntype, totlen
            off += totlen
        else:
            off += 1
```

The scanner looks for the two magic bytes, validates the whole header, and steps by
`totlen` on a valid node (or by 1 byte on noise). `yield` makes it a generator - the same
shape as the C scanner, but no buffer needed.

## 5. Decode the Two Node Types

```python
def dirent(d, off):
    pino, ver, ino, mctime = struct.unpack_from("<IIII", d, off+12)
    nsize, = struct.unpack_from("<B", d, off+28)
    name = d[off+40:off+40+nsize].decode("latin1")
    return ino, name

def inode(d, off):
    ino, ver = struct.unpack_from("<II", d, off+12)
    isize, csize = struct.unpack_from("<I", d, off+28)[0], struct.unpack_from("<I", d, off+48)[0]
    compr, = struct.unpack_from("<B", d, off+56)
    return ino, isize, csize, compr
```

The DIRENT carries the **name**; the INODE carries the **size and compression**. Track the
newest INODE per inode number and you have the file.

## 6. Decompress

```python
import zlib

def data_of(d, off):
    _, _, _, _ = read_header(d, off)
    # data begins after the 68-byte INODE struct
    _, _, _, csize, _, compr = inode(d, off)
    raw = d[off+68:off+68+csize]
    if compr == 6:
        return zlib.decompress(raw)
    return raw
```

`compr == 6` is zlib (`docs/11`); `compr == 0` is stored. This is the one place zlib's
`decompress` (not its CRC) is correct.

## 7. Put It Together

```python
def tree(d):
    best = {}
    for off, ntype, totlen in scan(d):
        if ntype == 0xE001:
            ino, name = dirent(d, off)
            best.setdefault(ino, {})["name"] = name
        elif ntype == 0xE002:
            ino, isize, csize, compr = inode(d, off)
            if isize and compr in (0, 6):
                best.setdefault(ino, {})["data"] = data_of(d, off)
    return best
```

## 8. The Lesson

This parser is the *same program* as `src/jffs2.c`, written in Python: read the header,
check `crc32_le`, step by `totlen`, decode. Write it once in each language and the format
stops being mysterious. That is the point of the module and this walkthrough.

## Exercises

1. Run the parser on `CTF-XX-rootfs.img` and list the nodes.
2. Confirm the CLEANMARKER's `hdr_crc` matches `crc32_le(0, header[0:8])`.
3. Extend `tree` to handle multiple INODE versions (keep the highest).
4. Compare its output with `jefferson`'s tree.

## Reference

- `src/jffs2.c`, `src/crc.c`, `docs/11`, `docs/12`
