# Walkthrough 35: Porting the Lab

**take the four-partition model to a different target**

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

## Why Port

The four-partition model is **hardware-independent**. This volume porting the lab to a
different board, so you see which parts are portable and which are target-specific.

## 1. What Is Portable

| layer | portable? |
| ----- | --------- |
| the four-partition layout | **yes** - a software idea |
| the container format | **yes** - bytes are bytes |
| JFFS2 + `crc32_le` | **yes** |
| the CRC discipline | **yes** |
| the crypto (AEAD, KEX, identity) | **yes** - OpenSSL |
| the module code | **yes** - plain C |
| the bootloader | target-specific |
| the kernel/DT | target-specific |
| the app's I/O | target-specific |

The **model and the code** port; the **bootloader and kernel** do not.

## 2. To a Different ARM64 Board

Say you move from the RP5 (BCM2712) to another ARM64 SBC:

1. Build **U-Boot** for the new board; make it the boot payload.
2. Build a **Linux Image** for the new board; read its **device tree**
   (`docs/09`).
3. Keep the **four-partition layout** unchanged (`docs/02`).
4. Rebuild the images (`docs/26`) with the new kernel.
5. The **modules** (`aead`, `crc`, `jffs2`, ...) compile unchanged.

```bash
python3 scripts/build_images.py --uboot <new-uboot> --kernel <new-Image> \
    --rootfs rootfs/ --out out/
python3 scripts/verify_telescreen.py --image out/full.img
```

## 3. To a Camera (the real thing)

The PROJECT teardown did this in reverse: it read a camera's four regions. To port the lab
**to** a camera you would:

1. Dump the camera's flash (`docs/04` discipline: three identical reads).
2. Read its `mtdparts` and map its regions.
3. Identify the container and filesystem by magic.
4. Build the modules against its toolchain (uClibc, ARM32).

The **method** is identical; the toolchain differs.

## 4. To a Bare-Metal Target

The crypto modules (`aead`, `crc`) are **freestanding**: `crc.c` and `jffs2.c` compile with
no libc at all (that is how the AArch64 disassembly was produced). To port to bare metal:

1. Replace OpenSSL with a native implementation (e.g. a small AES-GCM), or use your SoC's
   crypto engine.
2. Replace `snprintf` in `camera.c` with your own formatter.
3. Keep the layout, CRC, and JFFS2 logic unchanged.

## 5. The Magic That Must Match

Whatever the target, these bytes identify the regions:

| region | magic |
| ------ | ----- |
| boot | `15 05 00 ea` (if ARM) / your vector |
| kernel | `21 84 1b 00` + `gziphead` |
| rootfs | `85 19 03 20` |

If your target uses **different** magics, change `partition.h` and `container.h` - the
**logic** does not change, only the constants.

## 6. What Breaks on Port

| symptom | cause |
| ------- | ----- |
| no output on console | different UART base/baud (`docs/09`) |
| kernel will not boot | wrong DTB or machine |
| rootfs will not mount | different erase block or compressor (`docs/11`) |
| no crypto | different toolchain / OpenSSL version |
| wrong endianness | a big-endian target (change the loads) |

## 7. The Port Checklist

```
[ ] build U-Boot for the target
[ ] build a kernel Image + DT for the target
[ ] confirm the target's region magics; update the constants
[ ] confirm the erase block / compressor for the rootfs
[ ] build the four images
[ ] carve and verify (docs/04)
[ ] boot and watch the console
[ ] run the three gates on the host
```

## 8. Why This Matters

A security engineer who can only analyse one board is not much use. The point of the lab is
the **model**: fixed regions, a bootloader that loads by offset, a CRC-protected
environment, a container, a filesystem, and an app with sinks. Learn the model, and any
device is a week's work, not a career's.

## Exercises

1. List five things that port and five that do not, with reasons.
2. Change `container.h`'s magic and rebuild; confirm identification follows.
3. Compile `crc.c` for a second architecture and diff the assembly.
4. Write a port plan for a board you own.

## Reference

- `docs/02`, `docs/08`, `docs/09`, `docs/11`, `docs/26`
- `docs/walkthrough/25` (real-world comparison)
