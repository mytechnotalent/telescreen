# Walkthrough 63: Embedded Linux Fundamentals

**the Linux an embedded engineer must know**

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

## Why Embedded Linux

The camera class runs **Linux** (`docs/08`). Reading its firmware means reading Linux: the
boot, the device tree, the init, the filesystems, and the userspace. This volume is the
Linux subset that matters.

## 1. The Boot, Revisited

```
head.S -> start_kernel -> setup_arch (DT + cmdline) -> mm_init
        -> console_init -> rest_init -> kernel_init -> /sbin/init
```

The kernel is **generic**; the **device tree** and the **command line** specialise it
(`docs/09`, `docs/10`).

## 2. The Device Tree

A tree of nodes describing the hardware: CPUs, memory, buses, devices. The kernel's drivers
**bind** to nodes by `compatible`. `docs/09`.

## 3. init and the Userspace

BusyBox `init` reads `/etc/inittab` -> `sysinit` -> `rcS` -> `Snn` scripts -> the
application (`docs/13`). The userspace is a **shell and a handful of scripts** plus the app.

## 4. Procfs and Sysfs

Two virtual filesystems expose kernel state:

| path | exposes |
| ---- | ------- |
| `/proc/cmdline` | the boot command line |
| `/proc/mtd` | the MTD partitions |
| `/proc/partitions` | block devices |
| `/proc/net/*` | network state |
| `/sys/class/...` | device classes |

Reading them on a live device is often the fastest recon (`docs/walkthrough/21`).

## 5. MTD and the Rootfs

`mtdparts` -> `/dev/mtdN` -> mount (`docs/10`). The rootfs format (JFFS2 here) determines
extraction and patching (`docs/11`, `docs/12`).

## 6. Users, Permissions, and Root

The app runs as **root**. That is the lab's central weakness (`docs/walkthrough/49`). A
defended device drops privileges for the network-facing parts.

## 7. The Init Ramdisk and Overlay

Many devices boot an **initramfs** (a small rootfs in RAM) then mount the real rootfs; some
use an **overlay** so the read-only rootfs can be written without touching flash. The lab
uses a **tmpfs** for writable state (`docs/10`).

## 8. Device Drivers

A driver binds a `compatible` string to a node, maps its registers, and handles interrupts.
Kernel modules (`*.ko`) can be loaded at runtime - the camera loaded a stack of `xm_*.ko`
media modules. The lab's modules are userspace, but the concept is the same.

## 9. Logging

| source | where |
| ------ | ----- |
| kernel | `dmesg` / the console |
| the app | stdout / a log file |
| the bootloader | the console |

The **console** (`docs/walkthrough/21`) is the union of all three.

## 10. Debugging a Device

```
1. The console         -> the boot story
2. /proc and /sys      -> live state
3. strace / ltrace     -> syscalls (if present)
4. gdbserver           -> live debugging (if present)
5. the flash dump      -> the ground truth
```

Most embedded devices lack (3) and (4); the flash dump is always available.

## 11. The Build

An embedded Linux is built from: a **bootloader**, a **kernel** (with a DT), and a **rootfs**
(a cross-built userspace). The lab provides all three as the four images (`docs/26`).

## 12. The Embedded-Linux Checklist

```
[ ] the boot command line is known
[ ] the device tree is read
[ ] the rootfs format is identified
[ ] the init chain is traced
[ ] the app's privileges are noted
[ ] the writable regions are known (tmpfs/overlay)
[ ] the console is captured
```

## Exercises

1. Read `/proc/cmdline` and `/proc/mtd` on the RP5.
2. Trace the init chain and name each script.
3. Name the two virtual filesystems and one useful file in each.
4. Identify where the lab's app runs as root and propose the fix.

## Reference

- `docs/08`-`docs/13`, `docs/walkthrough/21`
- the Linux kernel documentation; BusyBox documentation
