# Walkthrough 37: Filesystems for Embedded Devices

**JFFS2, UBI, squashfs, and ext4, and when each is used**

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

## Why Filesystem Matters

The rootfs format decides **how you extract, patch, and rebuild**. Get it wrong and the
device will not boot. The lab uses JFFS2; real devices use several. Knowing them makes you
portable.

## The Four You Meet

| fs | runs on | writable | compression | typical device |
| -- | ------- | -------- | ----------- | -------------- |
| **JFFS2** | raw flash (MTD) | yes | zlib/lzo | this lab, many cameras |
| **UBIFS** | UBI on raw flash | yes | lzo/zlib | newer NAND devices |
| **squashfs** | any block/MTD | **no** | gzip/lzo/xz | read-only rootfs |
| **ext4** | block (eMMC/SD) | yes | no | eMMC/SD devices |

## 1. JFFS2 (this lab)

**Model:** log-structured nodes on raw flash; a scan rebuilds the tree (`docs/11`).
**Extract:** `jefferson`.
**Patch:** append a higher-version INODE (`docs/12`).
**Rebuild:** `mkfs.jffs2 -e 0x10000 -X zlib`.
**Watch for:** the `crc32_le` header CRC; the erase-block size.

## 2. UBIFS

**Model:** UBIFS sits **on top of UBI**, which manages wear on NAND. Two images: the UBI
volume table and the UBIFS.
**Extract:** `ubireader` / `ubi_reader` tools.
**Rebuild:** `mkfs.ubifs` then `ubinize`.
**Watch for:** NAND geometry (page size, LEB size, PEB size). Getting these wrong makes an
image the device rejects.

## 3. squashfs

**Model:** a compressed, **read-only** image; the rootfs is immutable. Writable state goes
in an **overlay** (often JFFS2/UBI) or a tmpfs.
**Extract:** `unsquashfs`.
**Rebuild:** `mksquashfs`.
**Watch for:** the compression (gzip/xz/lzo) and block size.

## 4. ext4

**Model:** a normal block filesystem on eMMC/SD.
**Extract:** mount it (loopback).
**Rebuild:** `mke2fs`.
**Watch for:** it is a real filesystem with inodes/journals, not a raw layout.

## The Extraction Decision

```
look at the magic:
  85 19 ...          -> JFFS2   -> jefferson
  UBI#               -> UBI     -> ubireader
  sqsh (0x73717368)  -> squashfs-> unsquashfs
  ext  (0x53ef)      -> ext4    -> mount
  cramfs (0x28cd3d45)-> cramfs  -> cramfsck / unsquashfs -no
```

Identify first (`docs/02`); extract second. Never guess.

## The Patching Decision

| fs | patch method |
| -- | ------------ |
| JFFS2 | append a higher-version INODE (`docs/12`) |
| UBIFS | rebuild the UBIFS image (`mkfs.ubifs`) |
| squashfs | extract, edit, `mksquashfs` |
| ext4 | mount, edit, unmount |

JFFS2 is the only one you can patch **in place**; the others you rebuild. That is why the
lab teaches JFFS2's node model in depth.

## The Lab's Choice

The lab uses JFFS2 because:

- it is **in-place patchable** (the best teaching);
- it is **self-validating** (`crc32_le` per node);
- it is **common** on the camera class.

## Exercises

1. Write the magic for each filesystem and the tool you would use.
2. Explain why squashfs cannot be patched in place.
3. Name the two UBI parameters that must match the NAND.
4. Extract a squashfs image (any board) with `unsquashfs`.

## Reference

- `docs/11`, `docs/12`
- UBI/UBIFS documentation; squashfs documentation
