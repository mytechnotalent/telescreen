# Walkthrough 25: Real-World Comparison

**how the lab maps to the devices that actually ship**

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

## Why Compare

The TELESCREEN is fiction, but the class is real: cheap, unsigned, backdoored cameras and
routers sold under dozens of brands. Every lab technique exists because a real device had
that flaw. This volume maps each lab concept to its real-world counterpart, so you leave
the course able to analyse a device you have never seen.

## 1. The Four-Partition Model, in the Wild

Real devices lay their firmware out as a fixed set of regions. The names vary:

| region | common names on real devices |
| ------ | ---------------------------- |
| bootloader | `boot`, `u-boot`, `uboot`, `spl` |
| environment | `bootargs`, `env`, `uboot-env` |
| kernel | `kernel`, `uImage`, `zImage`, `linux` |
| rootfs | `rootfs`, `rootfs_data`, `ubi`, `overlay` |

The **idea** is identical: fixed offsets, a `mtdparts`-style map, a bootloader that loads
by offset. Once you can read one device's map, you can read them all (`docs/02`).

## 2. The Boot Chain, in the Wild

```
BootROM -> SPL -> U-Boot -> kernel -> rootfs -> app
```

On the RP5 the first stage is the VideoCore firmware (closed). On a phone it is the
Primary Boot Loader. On a camera it is the on-die BootROM. The **shape** is universal:
a closed first stage, then a chain you can analyse (`docs/05`, `docs/06`).

## 3. The Real Flaws We Modelled

| lab defect | real-world class | named examples |
| ---------- | ---------------- | -------------- |
| B1 config-sourced root exec | dirty config / init injection | countless router firmwares |
| B2 CGI command injection | `system()` from a request | web-shell classes |
| B3 archive-to-root restore | Zip-Slip / tar traversal | path-traversal CVEs |
| B4 default credentials | shipped defaults | the "admin/admin" class |
| B5 debug root shell | vendor debug backdoors | telnet/UART backdoors |
| B6 weak KDF | key from a public ID | the P2P-cloud class |

None of these is exotic. That is the point: **the boring bugs are the ones that own
devices.**

## 4. The Cloud/P2P Class

Real cameras talk to a vendor relay so the app works behind NAT. The lessons:

- the relay is a **third party** in the trust path;
- if the client-to-device crypto is weak or key-derived, the relay (or anyone) can read;
- the device **beacons** by default, mapping the network.

The lab keeps the shape (a beacon to a local collector) and removes the internet
(`docs/28`).

## 5. The Routing Class

Real cameras are also **routers** (or sit on one). Because they route, they see the social
graph. The lab builds the router half (`docs/24`) so the privacy consequence is concrete,
not abstract.

## 6. How to Analyse an Unknown Device

The transferable method:

```
1. Get the image (dump the flash, or a vendor update).
2. Identify the partitions by magic (docs/02).
3. Read the boot chain (docs/05-docs/10).
4. Extract the rootfs (docs/11).
5. Find the app and its dangerous sinks (docs/14, docs/15).
6. Find the crypto and the key source (docs/17).
7. Find the cloud endpoints (docs/28).
8. Write findings with mechanism, path, impact, fix.
```

This is exactly the method the PROJECT camera teardown used, and it is the method you now
own.

## 7. What Changes Device to Device

| variable | example |
| -------- | ------- |
| SoC | GOKE, HiSilicon, SigmaStar, Goke, Ambarella, ... |
| storage | SPI-NOR, SPI-NAND, eMMC, SD |
| filesystem | JFFS2, UBI/squashfs, ext4, cramfs |
| app | a vendor daemon, often named for the brand |
| cloud | a vendor P2P, sometimes rebranded |

What does **not** change: the four-region idea, the boot shape, the sink classes, the
crypto mistakes, and the method.

## 8. The Responsibility

Now that you can do this to any device:

- do it only to **your own** or **authorised** hardware (`docs/29`);
- **disclose** real defects responsibly;
- remember the **users**: a camera is someone's home.

## Exercises

1. Take a device you own and identify its partition map without documentation.
2. Classify its app's dangerous sinks.
3. Find where its update or cloud key comes from.
4. Write a one-page report in the finding format.

## Reference

- `docs/02`, `docs/05`, `docs/14`, `docs/15`, `docs/17`, `docs/28`, `docs/29`
- The PROJECT camera teardown (separate repository) for a full worked example
