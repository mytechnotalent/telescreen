# Walkthrough 17: Byte-Level Artifact Walkthrough

**the real bytes of the four regions, annotated**

***
**LEGAL DISCLAIMER:** The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with. **IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
***

Source artifact: `CTF-XX-full.img` (16777216 bytes).

## Region 0 - boot (offset 0x000000)

```
00000000: 1505 00ea feff ffea feff ffea feff ffea  ................
00000010: feff ffea feff ffea feff ffea feff ffea  ................
00000020: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000030: 0000 0000 0000 0000 0000 0000 0000 0000  ................
```

The first word `15 05 00 ea` = `0xEA000515` = `B 0x145C`; the next seven words are
`fe ff ff ea` = `0xEAFFFFFE` = `B .` (the Cortex-A vector filler).

## Region 1 - bootargs (offset 0x020000)

```
00020000: 1182 dda5 626f 6f74 6172 6773 3d63 6f6e  ....bootargs=con
00020010: 736f 6c65 3d74 7479 414d 4130 2c31 3135  sole=ttyAMA0,115
00020020: 3230 3000 626f 6f74 6465 6c61 793d 3100  200.bootdelay=1.
00020030: 0000 0000 0000 0000 0000 0000 0000 0000  ................
```

Leading CRC32 (4 bytes LE), then `key=value\0` text.

## Region 2 - kernel (offset 0x030000)

```
00030000: 2184 1b00 0010 0000 677a 6970 6865 6164  !.......gziphead
00030010: 1f8b 0800 0000 0000 02ff edc1 010d 0000  ................
00030020: 00c2 a0f7 4f6d 0f07 1400 0000 f06e 1100  ....Om.......n..
00030030: 1cc7 0010 0000 0000 0000 0000 0000 0000  ................
```

Magic `21 84 1b 00` (`0x001B8421`), the length field, `gziphead` at +8, gzip at +16.

## Region 3 - rootfs (offset 0x1F0000)

```
001f0000: 8519 0320 0c00 0000 0000 0000 0000 0000  ... ............
001f0010: 0000 0000 0000 0000 0000 0000 0000 0000  ................
001f0020: 0000 0000 0000 0000 0000 0000 0000 0000  ................
001f0030: 0000 0000 0000 0000 0000 0000 0000 0000  ................
```

Magic `85 19 03 20` (JFFS2 little-endian + CLEANMARKER), then the node header CRC at +8.

## The Hardened Variant (offset 0x000000)

```
00000000: 1505 00ea feff ffea feff ffea feff ffea  ................
00000010: feff ffea feff ffea feff ffea feff ffea  ................
```

## Exercises

1. Hex-dump each region and match every byte to `docs/30`.
2. Decode the boot vector by hand.
3. Locate the gzip magic at kernel +16.
4. Read the CLEANMARKER type from the rootfs header.
