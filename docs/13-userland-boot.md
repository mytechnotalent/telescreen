# Volume 13: Userland Boot

**init, network bring-up, and the daemon**

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

- The exact chain from `/sbin/init` to the TELESCREEN daemon
- How the network comes up (and the `eth0`/`wlan0` split)
- The read-only rootfs and the tmpfs working directory
- Where the daemon is launched

## 1. The Chain

```
kernel -> /sbin/init (BusyBox)
   /etc/inittab:  ::sysinit:/etc/init.d/rcS
      rcS -> S00devs -> S01udev -> S80network
          S80network: ifconfig lo; bring up interfaces; /etc/starts &
              /etc/starts -> /mnt/mtd/ipc/run
                  run -> "$TARGET/teled &"
```

## 2. `/sbin/init` and `inittab`

BusyBox init runs the `sysinit` line first:

```
::sysinit:/etc/init.d/rcS
::respawn:/sbin/getty -L ttyAMA0 115200 vt100
```

`respawn` gives a **serial login** on the UART. On the TELESCREEN that login is a
potential local‑access path if its password is weak (`docs/16`, B5).

## 3. `rcS` and the `Snn` Scripts

```sh
#!/bin/sh
/bin/mount -a
for initscript in /etc/init.d/S[0-9][0-9]* ; do
    [ -x "$initscript" ] && "$initscript"
done
```

Scripts run in name order: `S00devs`, `S01udev`, `S80network`, ...

## 4. Network Bring-Up

`S80network` sets loopback, brings up the interfaces, and starts the application
launcher. The TELESCREEN split is:

| interface | role |
| --------- | ---- |
| `eth0` | **WAN** - uplink (Ethernet) |
| `wlan0` | **LAN/AP** - `hostapd` access point |

The two roles are the core of `docs/24` (RP5 as a router).

## 5. The Launcher

`/etc/starts` is a one-liner:

```sh
#!/bin/sh
/mnt/mtd/ipc/run
```

`run` remounts `/` read-only, mounts the tmpfs, loads drivers, brings up the network,
and finally launches the daemon:

```sh
mount -t jffs2 -o remount,ro /dev/mtdblock3 /
mount tmpfs /mnt/mtd/ipc/tmpfs -t tmpfs -o size=13m
...
$TARGET/teled &
```

## 6. Why the Design Matters

- The rootfs is **read-only**; writable state is in **tmpfs** (`docs/10`).
- The launcher is a **shell script**; a shell script that **sources a writable config as
  root** is exactly defect **B1** (`docs/16`).
- The daemon runs as **root**, so any bug in it is a root bug.

## 7. Observe It

```bash
# serial console, 115200 8N1
[RCS]: /etc/init.d/S80network
teled: beacon ready
```

## Labs

1. Write the chain from `/sbin/init` to `teled` from memory.
2. Map `eth0`/`wlan0` to WAN/LAN and justify the choice.
3. Find the point where a shell script runs as root and explain the risk.
4. Explain what a read-only rootfs buys you (and what it does not).

## Reference

- `docs/10` (mount), `docs/14` (dispatcher), `docs/16` (backdoors)
