# Volume 01: World and Threat

**the setting, the trust boundaries, and the device**

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

- The fictional setting and why it matters for real research
- The TELESCREEN device and its exact hardware
- A precise **threat model**: actors, capabilities, trust boundaries
- The six defect classes the whole course is built around
- How to run the lab safely

## 1. Why a Story

A backdoor in an IP camera is not exciting. A backdoor in a device that watches your
home and routes your traffic is. The **TELESCREEN** is fiction; the technology is
real. Every technique here applies to the cheap, unsigned, backdoored cameras and
routers that ship by the million. The story is a memory aid: it gives each defect a
*motive*, so you remember *why* someone would ship it.

> The engineers who built the silicon are **teachers, not villains**. The surveillance
> is the crime; the craft is admirable. Criticise the design, not the people.

### Where this fits

TELESCREEN is the **surveillance backbone of the Ministry** - the Linux/RP5
appliance that watches the industrial edge built in OPERATION COLD IRON, a
planned ten-act saga still in development. It is the **finale after OPERATION
COLD IRON**: that saga teaches the bare-metal **ARM** device (the ARM Cortex-M33
cold-chain monitor); TELESCREEN builds the application-class **ARM** system
(ARM Cortex-A76, RP5/Linux) that watches over it. The whole saga is **ARM** - the
same architecture, one level up. The Ministry is the same antagonist across the
saga; against it stands WHITEOUT. This act is the wall unit that watches the edge.

## 2. The Device

The lab **is** the device. There is no separate "captured board" to chase down: the
Raspberry Pi 5 you build **is** the TELESCREEN.

| Component     | TELESCREEN (Raspberry Pi 5)               |
| ------------- | ----------------------------------------- |
| SoC           | BCM2712, 4x Cortex-A76, ARMv8-A           |
| Word size     | 64-bit (AArch64); the app is built for it |
| Image store   | microSD / NVMe - holds the four images    |
| First stage   | VideoCore bootloader (closed)             |
| Bootloader    | U-Boot 2024.07 (RP5 build)                |
| Kernel        | vendor container -> Linux 6.6 `Image`     |
| Rootfs        | JFFS2 (read-only)                         |
| WAN           | Ethernet (`eth0`)                         |
| LAN / AP      | Wi-Fi AP (`wlan0`, hostapd)               |
| Camera        | USB webcam, UVC, `/dev/video0`            |
| Exfil target  | local collector (`lab-sink`)              |

### The Four-Partition Image

Everything on the device fits in four fixed regions. This is the single most
important structural fact in the course:

```
mtd0  boot      0x000000  128 KiB    U-Boot (the first stage you control)
mtd1  bootargs  0x020000   64 KiB    U-Boot environment (CRC32 + key=value)
mtd2  kernel    0x030000 1792 KiB    vendor container -> Linux Image
mtd3  rootfs    0x1F0000 14400 KiB   JFFS2 (the application + web)
```

Full detail is in `docs/02`. For now, hold the shape in your head: **boot / env /
kernel / rootfs**.

## 3. Threat Model

A threat model answers three questions: **who** attacks, **what** they can reach, and
**what** they get. Everything else in the course is a consequence of this section.

### 3.1 Actors and Capabilities

| actor | access path | capability gained |
| ----- | ----------- | ----------------- |
| LAN attacker | TCP 80 / 554 | web admin, unauthenticated media |
| WAN attacker | NAT / port-forward / relay | the same, remotely |
| Malicious client | RTSP / ONVIF | stream, DoS, and (V1) RCE |
| Physical attacker | image store / SD / UART | total control - no secure boot |
| Cloud / vendor | default relays | telemetry + metadata |

### 3.2 Trust Boundaries

```
+------------+---------------------------------------------------------+
|                              TELESCREEN                              |
+------------+---------------------------------------------------------+
| [Internet] | PPPP relay / DDNS / NTP ---> [beacon] (weak crypto)     |
| [Clients]  | RTSP :554 / HTTP :80    ---> [app]    (unauth surface)  |
| [Operator] | UART :115200            ---> [U-Boot] (password-locked) |
| [Physical] | image store / SD        ---> [flash]  (no secure boot)  |
+------------+---------------------------------------------------------+
```

Each arrow is a boundary you will cross in the labs:

- **Network boundary** (unauthenticated web + media) -> the remote-root class.
- **Cloud boundary** (PPPP to relays) -> the crypto-failure class.
- **Physical boundary** (image store) -> total control; the recovery path.

### 3.3 What the Device Is *Meant* to Expose

Video/audio and an admin UI on a **managed LAN**, with **authenticated** access.

### 3.4 What It *Actually* Exposes

- RTSP with no authentication by default.
- A weak/empty web auth design.
- An embedded server whose config handlers call `system()`.
- An unsigned boot chain with **no secure boot**.

## 4. The Six Defect Classes

The CTF (`CTF_telescreen/`) hands you the *compromised* image with these defects. The
project (this repo) builds the *defended* device. Learn the defect, then the fix.

| # | defect | where it lives | class |
| - | ------ | -------------- | ----- |
| B1 | config-sourced root exec | rootfs config + init | root code execution |
| B2 | CGI command injection | app (HTTP dispatcher) | unauthenticated remote root |
| B3 | archive-to-root restore | app (`tar -C /`) | arbitrary root write |
| B4 | empty / default credentials | rootfs config | auth bypass |
| B5 | debug root shell | rootfs scripts | local privilege |
| B6 | weak key schedule | app (`beacon`) | crypto failure |

Each maps to a volume later in the course (`docs/16` catalogues them all).

## 5. The Lab's Rules (non-negotiable)

1. **Isolated network.** Dedicated switch or host-only net. Never production.
2. **Local collector.** The exfil sink (`lab-sink`) is on your bench.
3. **Own hardware.** You test what you own or are authorised to test.
4. **Keep the golden image.** Always have the stock image for rollback.

## 6. Commands You Will Use All Course

```bash
# the four-partition tools
python3 scripts/carve.py --image CTF-XX-full.img --out carved/
python3 scripts/verify_telescreen.py --image CTF-XX-full.img
python3 scripts/build_images.py --uboot boot.bin --kernel Image --rootfs rootfs/ --out out/

# the standards + tests
python3 scripts/audit_c_standard.py
python3 scripts/run_tests.py
python3 scripts/check_coverage.py

# reverse engineering
xxd -l 32 boot.img
arm-none-eabi-objdump -D -b binary -m arm boot.img | head
```

## Lab 1: Draw the Model

1. From memory, write the four-partition table (offsets, sizes, magics).
2. Name the four trust boundaries and one attack per boundary.
3. For each defect class B1-B6, write one sentence: the flaw and the fix.

## Reference

- `docs/02` - the four partitions, source and disassembly
- `PARTS.md` - the hardware
- `CTF_telescreen/CTF-XX-I.md` - the challenge this project defends against
