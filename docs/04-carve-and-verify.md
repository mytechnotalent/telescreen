# Volume 04: Carve and Verify

**proving the layout from the bytes**

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

- Carve a whole image into the four partitions two ways (`dd` and a tool)
- Verify with hashes and magic bytes
- The rule that governs the entire course: **trust bytes, not labels**

## 1. Carve with `dd`

The offsets come straight from `mtdparts` (`docs/02`):

```bash
dd if=CTF-XX-full.img of=boot.img    bs=1 count=$((0x20000))
dd if=CTF-XX-full.img of=env.img     bs=1 skip=$((0x20000)) count=$((0x10000))
dd if=CTF-XX-full.img of=kernel.img  bs=1 skip=$((0x30000)) count=$((0x1C0000))
dd if=CTF-XX-full.img of=rootfs.img  bs=1 skip=$((0x1F0000)) count=$((0xE10000))
```

## 2. Carve with the tool

`scripts/carve.py` encodes the layout once so every lab uses the same numbers:

```python
PARTS = (
    ("boot", 0x000000, 0x020000),
    ("bootargs", 0x020000, 0x010000),
    ("kernel", 0x030000, 0x1C0000),
    ("rootfs", 0x1F0000, 0xE10000),
)
```

```bash
$ python3 scripts/carve.py --image CTF-XX-full.img --out carved/
boot      off=0x000000 size=0x020000 wrote 131072 bytes
bootargs  off=0x020000 size=0x010000 wrote 65536 bytes
kernel    off=0x030000 size=0x1C0000 wrote 1835008 bytes
rootfs    off=0x1F0000 size=0xE10000 wrote 14745600 bytes
```

## 3. Verify

`scripts/verify_telescreen.py` checks the SHA-256 (when supplied) and the three
magic-bearing partitions:

```bash
$ python3 scripts/verify_telescreen.py --image CTF-XX-full.img \
      --sha256 037cc979a58f04f3285132a7b745ca859b1604aeb80c9ef62eb5377e48bd18ec
sha256 037cc979a58f04f3285132a7b745ca859b1604aeb80c9ef62eb5377e48bd18ec
boot      magic ok
kernel    magic ok
rootfs    magic ok
```

The verifier checks exactly three magics (boot, kernel, rootfs); `bootargs` has no
magic (it is CRC + text), so it is validated by its CRC instead (`docs/07`).

## 4. Hash Everything

Record a hash for each partition and the whole image. You will use them as the
reference for every later lab:

```bash
sha256sum CTF-XX-full.img CTF-XX-boot.img CTF-XX-env.img \
          CTF-XX-kernel.img CTF-XX-rootfs.img
```

## 5. The Rule: Trust Bytes, Not Labels

| the label says | the bytes say | trust |
| -------------- | ------------- | ----- |
| "boot.img" | `15 05 00 ea` | **bytes** |
| "kernel.img" | `21 84 1b 00` `gziphead` | **bytes** |
| "rootfs.img" | `85 19 03 20` | **bytes** |

A file named `kernel.img` that starts with `85 19` is a JFFS2 image with the wrong
name. The magic is the truth.

## 6. Self-Test: Corrupt and Catch

```bash
cp CTF-XX-boot.img /tmp/bad.img
printf '\xff' | dd of=/tmp/bad.img bs=1 seek=0 count=1 conv=notrunc
xxd -l 4 /tmp/bad.img        # 15 -> ff : magic broken
```

The identification table (`docs/02`) tells you `part_identify` will now return
`PART_KIND_UNKNOWN` for that buffer.

## Labs

1. Carve all four partitions, hash each, and record the values.
2. Prove each partition's identity with a hex dump and the table in `docs/02`.
3. Corrupt one byte in the boot magic and show the failure.
4. Compute the full-image SHA-256 and compare with the value in `CTF-XX-R.md`.

## Reference

- `scripts/carve.py`, `scripts/verify_telescreen.py`
- `docs/02`, `docs/03`
