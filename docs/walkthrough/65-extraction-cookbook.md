# Walkthrough 65: Firmware Extraction Cookbook

**recipes for every format you will meet**

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

## How to Use This

Each recipe is **magic -> tool -> command**. Identify the format, apply the recipe. This is
the practical companion to `docs/walkthrough/37`.

## Recipe: JFFS2

**Magic.** `85 19 03 20` (LE `0x1985` + CLEANMARKER).
**Extract.**
```bash
jefferson -d rootfs image.jffs2
```
**Rebuild.**
```bash
mkfs.jffs2 -r rootfs -o out.jffs2 -e 0x10000 -X zlib -n -p
```
**Watch.** the `crc32_le` header CRC (`docs/11`).

## Recipe: squashfs

**Magic.** `sqsh` (`0x73717368`) or `hsqs`.
**Extract.**
```bash
unsquashfs -d rootfs image.squashfs
```
**Rebuild.**
```bash
mksquashfs rootfs out.squashfs -comp xz
```
**Watch.** read-only; use an overlay for writes.

## Recipe: UBI / UBIFS

**Magic.** `UBI#`.
**Extract.**
```bash
ubireader_extract_images image.ubi
ubireader_extract_files image.ubi
```
**Rebuild.** `mkfs.ubifs` + `ubinize` with the target's geometry.
**Watch.** the NAND page/LEB/PEB sizes must match the target.

## Recipe: ext4

**Magic.** `0x53EF` at offset `0x438`.
**Extract.**
```bash
mount -o loop image.ext4 /mnt
```
**Watch.** journals; use `dumpe2fs` for geometry.

## Recipe: cramfs

**Magic.** `0x28cd3d45` (bytes `45 3d cd 28`).
**Extract.** `cramfsck -x rootfs image.cramfs` or `unsquashfs -no`.

## Recipe: gzip stream

**Magic.** `1f 8b 08`.
**Extract.**
```bash
gunzip -c image.gz > image.bin
```

## Recipe: the vendor kernel container

**Magic.** `21 84 1b 00` + `gziphead`.
**Extract.**
```bash
dd if=kernel.img bs=1 skip=16 | gunzip > Image
```
**Watch.** offset 16, not 0 (`docs/08`).

## Recipe: uImage / zImage

**uImage magic.** `0x27051956`; header 64 bytes; then the payload.
**zImage magic.** `0x016F2818` at offset `0x24`.
**Extract.** `binwalk -e`, or strip 64 bytes for a uImage.

## Recipe: device tree blob

**Magic.** `d0 0d fe ed`.
**Extract.**
```bash
dtc -I dtb -O dts -o board.dts board.dtb
```

## Recipe: a rootfs archive

**Magic.** `1f 8b` (tar.gz), `28 b5 2f fd` (zstd), `fd 37 7a 58 5a` (xz), `50 4b` (zip).
**Extract.**
```bash
tar -xf archive.tar.gz         # or zstd -d | tar -xf -, unxz | tar -xf -, unzip
```
**Watch.** path traversal on extract (`docs/16`, B3).

## Recipe: an unknown blob

```bash
binwalk -e blob.bin
# then for each reported offset:
xxd -s <offset> -l 32 blob.bin
```
`binwalk` suggests; the magic confirms.

## The One-Line Identification

```bash
xxd -l 16 X | head -1
```

Then match against this table:

| first bytes | format |
| ----------- | ------ |
| `15 05 00 ea` | ARM vector (boot) |
| `85 19` | JFFS2 node |
| `hsqs` / `sqsh` | squashfs |
| `UBI#` | UBI |
| `1f 8b` | gzip |
| `50 4b 03 04` | zip |
| `28 b5 2f fd` | zstd |
| `fd 37 7a 58 5a` | xz |
| `d0 0d fe ed` | FDT |
| `27 05 19 56` | uImage |
| `45 3d cd 28` | cramfs |

## Exercises

1. Identify five unknown files using the one-line recipe.
2. Extract each with its recipe.
3. Rebuild one extracted filesystem.
4. Add a recipe for a format you meet.

## Reference

- `docs/walkthrough/37`, `docs/04`, `docs/08`, `docs/11`
