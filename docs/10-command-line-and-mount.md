# Volume 10: Command Line and Mount

**bootargs, mtdparts, and mounting JFFS2 as `/`**

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

- The exact `bootargs` the kernel receives
- How `mtdparts` becomes `/dev/mtd0..3`
- How the kernel mounts the JFFS2 rootfs as `/`
- The read-only remount and the tmpfs pattern

## 1. The Kernel Command Line

U-Boot passes `bootargs` (from `docs/06`) to the kernel:

```
console=ttyAMA0,115200
mtdparts=sfc:128K(boot),64K(bootargs),1792K(kernel),14400K(rootfs)
root=/dev/mtdblock3 rootfstype=jffs2 rw
```

## 2. `mtdparts` -> `/dev/mtdN`

The `mtdparts` token is parsed by the kernel command-line partition parser into an MTD
map, and the MTD block layer creates device nodes:

| device | partition | size |
| ------ | --------- | ---- |
| `/dev/mtd0` | boot | 128 KiB |
| `/dev/mtd1` | bootargs | 64 KiB |
| `/dev/mtd2` | kernel | 1792 KiB |
| `/dev/mtd3` | rootfs | 14400 KiB |

`/dev/mtdblock3` is the **block** view of `mtd3`, which is what gets mounted.

## 3. Mounting Root

```
root=/dev/mtdblock3 rootfstype=jffs2 rw
```

- `rootfstype=jffs2` tells the kernel to bring up a JFFS2 driver (`docs/11`).
- `rw` means the first mount is read-write, so init can fix things up; it then remounts
  **read-only** (see below).

## 4. The Read-Only Remount

A hardened device remounts `/` read-only once boot finishes. The TELESCREEN does this in
its init script and uses a **tmpfs** for everything writable:

```
mount -t jffs2 -o remount,ro /dev/mtdblock3 /
mount tmpfs /mnt/mtd/ipc/tmpfs -t tmpfs -o size=13m
```

This is why:

- the rootfs stays pristine (fewer flash writes, longer life);
- writable state lives in RAM (`/tmpfs`) and on the SD card;
- an attacker who wants persistence must write **flash**, not just `/tmp`.

## 5. What This Means for the Labs

| goal | consequence |
| ---- | ----------- |
| patch a file | you must edit JFFS2 and reflash (`docs/12`) |
| read config | read from the rootfs or the tmpfs |
| persist | write the image, not the live filesystem |

## 6. Observe It

On the serial console:

```
[    0.000000] Kernel command line: console=ttyAMA0,115200 mtdparts=sfc:...
[    0.000000] Creating 4 MTD partitions on "sfc":
[    0.000000] 0x000000000000-0x000000020000 : "boot"
[    0.000000] 0x000000020000-0x000000030000 : "bootargs"
[    0.000000] 0x000000030000-0x0000001f0000 : "kernel"
[    0.000000] 0x0000001f0000-0x000001000000 : "rootfs"
```

## 7. The Hand-Off

```
U-Boot bootargs -> Linux parses mtdparts -> /dev/mtdN created
                -> mount_root() mounts mtdblock3 (JFFS2) as /
                -> /sbin/init -> rcS -> network -> teled
```

That is the exact boundary from kernel to userland. From `/sbin/init` on, it is the
vendor (or your) application (`docs/13`).

## Labs

1. Reconstruct the four `/dev/mtdN` lines from `mtdparts`.
2. Explain the difference between `/dev/mtd3` and `/dev/mtdblock3`.
3. Explain why a hardened device remounts `/` read-only.
4. Given a config change you want to persist, describe the path on flash.

## Reference

- `docs/06` (U-Boot), `docs/11` (JFFS2), `docs/13` (userland)
- Linux MTD documentation
