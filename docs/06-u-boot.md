# Volume 06: U-Boot

**identity, environment, and the kernel hand-off**

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

- What U-Boot is and why it is the first stage **you control** on the RP5
- The command line U-Boot passes to the kernel (`bootargs`)
- The command it runs to load the kernel (`bootcmd`)
- How the four partitions map to the load

## 1. Where U-Boot Sits

On an RP5 the closed **VideoCore firmware** is the true first stage. The first stage
**you control** is **U-Boot**, loaded as `kernel8.img`. U-Boot then loads the kernel
from the four-partition image and jumps to it.

```
VideoCore firmware (closed) -> U-Boot 2024.07 -> Linux 6.6 Image -> rootfs -> teled
```

`CTF-XX-boot.img` (partition 0, 128 KiB) is the U-Boot region.

## 2. Identity and Banner

On the serial console (115200 8N1) U-Boot prints a banner and a 1-second autoboot
window:

```
U-Boot 2024.07 (telescreen-rp5)
Hit any key to stop autoboot:  1
```

Interrupting autoboot drops you to a U-Boot prompt. On the TELESCREEN that prompt can
be **password-locked**, which is why the labs change the environment offline
(`docs/07`) rather than at the prompt.

## 3. The Two Important Environment Variables

U-Boot reads its environment from the **`bootargs`** partition (partition 1). The two
variables that matter:

```
bootargs=console=ttyAMA0,115200 root=/dev/mtdblock3 rootfstype=jffs2 rw \
         mtdparts=sfc:128K(boot),64K(bootargs),1792K(kernel),14400K(rootfs)
bootcmd=sf probe 0;boothz 0x41000000 0x40008000 0x30000 0x1C0000
```

**`bootargs`** becomes the kernel command line:

| token | meaning |
| ----- | ------- |
| `console=ttyAMA0,115200` | serial console |
| `mtdparts=sfc:128K(boot),...` | the four-partition map |
| `root=/dev/mtdblock3 rootfstype=jffs2` | mount partition 3 as `/` |
| `rw` | first mount is read-write (init remounts ro) |

**`bootcmd`** is what auto-boot runs:

```
sf probe 0                       ; probe the SPI/image store
boothz 0x41000000 0x40008000 \
       0x30000 0x1C0000          ; load + boot the kernel
```

Against the partition map:

- `0x30000` = the **kernel partition offset** (`mtd2`).
- `0x1C0000` = its **size** (1792 KiB).
- `0x41000000` = a staging buffer; `0x40008000` = the Linux load base.

So U-Boot knows the kernel by **flash offset only** - no filesystem.

## 4. The Four Partitions, One More Time

| mtd | name | offset | size | U-Boot's role |
| --- | ---- | ------ | ---- | ------------- |
| 0 | `boot` | `0x000000` | 128 KiB | U-Boot itself |
| 1 | `bootargs` | `0x020000` | 64 KiB | its environment |
| 2 | `kernel` | `0x030000` | 1792 KiB | the thing it loads |
| 3 | `rootfs` | `0x1F0000` | 14400 KiB | the kernel mounts it |

## 5. Observe It

```bash
# serial console at 115200 8N1
screen /dev/tty.usbserial-XXXX 115200
```

Expected order: VideoCore silence -> `U-Boot 2024.07` -> `Hit any key` -> kernel banner
-> `[RCS]` -> the app. See `docs/10` for the exact hand-off.

## 6. Why U-Boot Matters for Security

U-Boot is where **secure boot would live**. If it validated a signature before jumping
to the kernel, the entire "modify the image" class would be closed. It does **not** -
which is why every lab can reflash and rebuild freely.

## Labs

1. Read `bootargs` and `bootcmd` from `CTF-XX-env.img` with `strings`.
2. Decode `mtdparts` into the four-partition table.
3. Explain what `boothz 0x41000000 0x40008000 0x30000 0x1C0000` does, in words.
4. Name the U-Boot feature that, if enabled, would stop the CTF.

## Reference

- `docs/07` (the environment and its CRC), `docs/08` (the kernel container)
- U-Boot documentation: `mtdparts`, `bootargs`, `bootcmd`
