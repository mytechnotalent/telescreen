# Volume 32 - The Bench, the Four Images, and Extraction

***
**LEGAL DISCLAIMER:** The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with. **IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
***

Volume 31 gave you the software workbench. This volume explains **what the
device actually is**, how the four firmware images are laid out, and how a real
reverse engineer gets the code out of them. It ends by handing you the exact file
you will reverse in Volume 33.

Everything in this volume runs on your computer. **You do not need the Raspberry
Pi 5** to complete it. The hardware section exists so you understand the physical
device and so that you *can* do it for real if you own the parts.

---

## 32.1 The two benches

A reverse engineer keeps two benches.

### The software bench (you have this already)

| item | role |
| ---- | ---- |
| Docker Desktop | builds and runs the ARM64 target |
| Python 3 | runs the image tools in `scripts/` |
| Ghidra 12.1.3 + JDK 21 | static analysis |
| `file`, `xxd`, `readelf`, `objdump` | byte-level and ELF inspection |
| `shasum -a 256` (macOS) / `sha256sum` (Linux) | proving you have the right bytes |

On Windows, `readelf`, `objdump` and the GNU coreutils come from Ubuntu/WSL2.

### The hardware bench (optional, only for a real device)

This mirrors a real capture. If you own the parts, you can repeat the whole
journey on physical silicon.

| item | role | notes |
| ---- | ---- | ----- |
| Raspberry Pi 5 (8 GB) | runs the TELESCREEN build | the "device" |
| microSD card (32 GB) | holds the four images | flashed with the four partitions |
| USB-UART adapter (3.3 V) | serial console | 115200 8N1 |
| USB webcam (UVC) | the camera module | driverless on Linux (`/dev/video0`) |
| CH341A + **208-mil** SOP8→DIP8 adapter | reads a real SPI flash | only if you dump a *camera's* flash |
| multimeter | verify 3.3 V, no shorts | safety |

> **The 208-mil trap.** A SOIC-8 flash chip comes in 150-mil and 208-mil widths.
> Using the wrong adapter gives *intermittent contact* and *different bytes every
> read*. This single mistake wastes more time than any other in embedded RE.

---

## 32.2 The four-partition model

The TELESCREEN device stores everything in four flash regions, exactly like the
camera this course is modelled on. The layout is described by a string called
`mtdparts`:

```
mtdparts=sfc:128K(boot),64K(bootargs),1792K(kernel),14400K(rootfs)
```

Read that as: *"the serial-flash controller (`sfc`) is divided into four pieces:
a 128 KiB boot region, a 64 KiB bootargs region, a 1792 KiB kernel region, and a
14400 KiB rootfs region."*

| # | name | offset | size | what it is |
| - | ---- | ------ | ---- | ---------- |
| 0 | `boot` | `0x000000` | 128 KiB | U-Boot bootloader (starts `15 05 00 ea`) |
| 1 | `bootargs` | `0x020000` | 64 KiB | U-Boot environment: `CRC32` + `key=value\0` pairs |
| 2 | `kernel` | `0x030000` | 1792 KiB | vendor container (starts `21 84 1b 00`) wrapping gzip |
| 3 | `rootfs` | `0x1F0000` | 14400 KiB | JFFS2 filesystem (starts `85 19 03 20`) |

You can tell the partitions apart by their first four bytes (their **magic
numbers**) without guessing:

| partition | magic bytes | meaning |
| --------- | ----------- | ------- |
| boot | `15 05 00 ea` | an ARM branch instruction, `B 0x145C` |
| kernel | `21 84 1b 00` | the vendor container magic |
| rootfs | `85 19 03 20` | JFFS2 little-endian magic `0x1985` + cleanmarker |
| bootargs | (none) | printable text beginning with a CRC-32 word |

Finding formats **by magic** rather than by assumption is a core habit. Volume 4
of the wider curriculum drills it.

---

## 32.3 Where the four images come from

There are three honest ways to obtain the images, in order of realism:

1. **Dump them from a real device** (`flashrom` over a CH341A reading the SPI
   flash, or `dd` from the microSD on an RP5). This is the real world; it needs
   the hardware bench.
2. **Build them** with the tools in this repository (below). This is what the
   course does so that everyone has identical, provable bytes.
3. **Use the pre-built CTF images** in the companion `CTF_telescreen`
   repository, which already contain the defects you will hunt.

### Building the images

The builder takes three inputs - a bootloader image, a kernel image, and a rootfs
directory - and produces the four partitions:

```bash
python3 scripts/build_images.py \
    --uboot  path/to/u-boot.bin \
    --kernel path/to/kernel            \
    --rootfs path/to/rootfs-tree/      \
    --out    images/
```

It writes `images/boot.img`, `images/bootargs.img`, `images/kernel.img`, and
`images/rootfs.img`. The `bootargs` image is built for you from a default
environment string; the `kernel` image is wrapped in the vendor container; the
`rootfs` image is packed as JFFS2.

### Verifying an image

Never trust an image you did not verify. `verify_telescreen.py` checks the
four-partition layout by magic and can also check a known SHA-256:

```bash
python3 scripts/verify_telescreen.py --image images/full.img
python3 scripts/verify_telescreen.py --image images/full.img --sha256 <known-hash>
```

### Carving the four partitions back out

To split a whole-flash image into its four pieces:

```bash
python3 scripts/carve.py --image images/full.img --out carved/
# carved/boot.img  carved/bootargs.img  carved/kernel.img  carved/rootfs.img
```

`carve.py` knows the offsets and sizes from §32.2. This is the same operation as
the `dd` commands a hardware engineer would type, wrapped so you cannot fat-finger
an offset.

---

## 32.4 The RE target: how the code gets out of the image

On a real device the application lives inside the JFFS2 rootfs, at a path such as
`/usr/sbin/teled`. To reverse it you would:

1. carve `rootfs.img` out of the whole image (above),
2. extract the JFFS2 filesystem (walkthrough `28-jffs2-parser.md` shows a
   from-scratch node parser; `jefferson` is the usual off-the-shelf tool),
3. copy `/usr/sbin/teled` out,
4. confirm it is a stripped ELF and open it in Ghidra.

So that every student works on **identical, reproducible bytes**, this course
also builds that same program directly and hands it to you:

```
firmware/teled.stripped     <- the program you reverse (names removed)
firmware/teled.unstripped   <- the instructor answer key (names present)
```

`teled.stripped` is the exact application binary that would sit at
`/usr/sbin/teled` in the rootfs. Building it directly removes every source of
variation (which U-Boot you used, which JFFS2 packer, ...) so that the addresses
in this course are true for everyone.

> **Realism note.** The four-image flow in §32.3 is the part a professional does
> on real hardware: dump, verify, carve, extract. The direct `firmware/teled.*`
> build is the *pedagogical* on-ramp: same code, same compiler, same bytes, no
> hardware required. Do the image flow once to learn it; use the direct build to
> learn to reverse.

---

## 32.5 Confirming you have a stripped ARM64 ELF

Before reversing anything, answer three questions about the file. On macOS and
Linux:

```bash
file firmware/teled.stripped
# ELF 64-bit LSB executable, ARM aarch64, ... dynamically linked, ... stripped

readelf -h firmware/teled.stripped | grep -E 'Machine|Entry'
# Machine:  AArch64
# Entry point address: 0x401000

readelf -s firmware/teled.stripped | head
# (almost empty - the symbol table was removed, i.e. "stripped")
```

On Windows, run those same commands inside Ubuntu/WSL2.

If `Machine:` says `AArch64` and `file` says `stripped`, you are holding a real
reverse-engineering target: no names, no source, just bytes.

---

## 32.6 What you have now

- the four-partition model and the magic of each partition,
- the builder, verifier, and carver for the firmware images,
- the extraction path from a whole image down to a single binary,
- and `firmware/teled.stripped` - the stripped ARM64 application you will reverse.

Volume 33 is where the real work begins: opening that binary in Ghidra and
recovering the name and meaning of every function.
