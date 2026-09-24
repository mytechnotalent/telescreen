# TELESCREEN RP5 Lab - Parts and Requirements

**What you actually need to run the CTF, what is optional, and what the camera adds.**

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

## 1. Core - a stock Raspberry Pi 5 (required)

A plain, off-the-shelf RP5 kit is enough to boot the four-partition lab and run the
router, the backdoors, and the crypto exercises.

| item | role | notes |
|------|------|-------|
| Raspberry Pi 5 (2 GB is enough; 4/8 GB comfortable) | the lab board | BCM2712, Cortex-A76 |
| USB-C power supply (5 V, 5 A / 27 W official) | power | under-voltage throttles the SoC |
| microSD card (32 GB Class A2) | the four-partition image store | this is your "flash" |
| USB-UART adapter (3.3 V) + jumper wires | **boot console** | 115200 8N1; the one item people forget |
| Ethernet cable | WAN uplink | or an isolated switch |
| Wi-Fi client (phone/laptop) | prove the AP hands out DHCP | any device |

> The RP5's **on-board Wi-Fi (CYW43455)** is AP-capable, so a **stock RP5 already does
> Wi-Fi + router**. You do **not** need extra radios for the core lab.

## 2. Required for the full four-partition / flash lab

| item | role | notes |
|------|------|-------|
| A host Linux machine | build the images | or build on the RP5 itself |
| `mkfs.jffs2`, `gzip`, Python 3 | `scripts/build_images.py` | from `mtd-utils` |
| (optional) NVMe + M.2 HAT | faster image store | SD works fine |

## 3. Optional - the camera lab (`docs/25-rp5-as-a-camera.md`)

The RP5 has **no camera by default**. The router, backdoor, and crypto exercises do
**not** need one. Add a camera only if you want the video/RTSP half of the device.

**Default: a cheap USB webcam.** It enumerates as a standard **UVC** device at
`/dev/video0` with **no driver work**, so the pipeline is plain V4L2 + `ffmpeg`.

| item | role | notes |
|------|------|-------|
| **Xweiryn USB webcam** (default) | emulate the sensor | `/dev/video0`, UVC, plug-and-play - [Amazon B0GG3JNQGF](https://www.amazon.com/Xweiryn-Computer-Definition-Conference-Streaming/dp/B0GG3JNQGF) |
| (alt) Camera Module 3 (CSI) | emulate the sensor | needs the RP5 **15-pin** FPC cable |
| **Phone** (client) | DHCP proof, stream viewing, web UI | see the phone table below |

### The phone (what it is for, what app it needs)

| role | app needed? | how |
|------|-------------|-----|
| Prove the AP/DHCP (router lab) | **No app** | join SSID `TELESCREEN-XXXX`; confirm an IP from `dnsmasq` |
| Watch the stream | **VLC** (one free app) for `rtsp://<ip>:554/stream`, **or no app** via the MJPEG URL in a browser | the lab serves both RTSP and MJPEG |
| Web UI / backdoor labs | **No app** | any browser (or `curl` from the host) |

> This is the faithful part of the story: the real TELESCREEN used an **SC2336** image
> sensor and **USB** Wi-Fi (MediaTek MT7601 / Realtek RTL8733). A CSI Camera Module 3 or
> a USB webcam is the closest RP5 equivalent for the camera half.

## 4. Optional - extras that improve fidelity

| item | why |
|------|-----|
| USB Wi-Fi dongle (MediaTek MT76 or Realtek) | **mirrors the device's USB Wi-Fi**; enables true dual-radio (WAN on `eth0`, AP on the dongle) |
| USB-TTL serial console permanently wired | always-on boot log |
| Small OLED/LEDs on GPIO | status/heartbeat like the real unit |
| microSD reader | flashing/backup from the host |

## 5. Minimum viable kit

If you want the cheapest path that still teaches everything except video:

```
Raspberry Pi 5 + PSU + microSD + USB-UART adapter + Ethernet cable + a phone
```

That runs: the four-partition carve, the boot chain, the JFFS2 patch, the backdoors,
the weak-KDF break, the AEAD hardening, and the router labs.

## 6. What maps to what (course reference)

| lab | hardware needed |
|-----|-----------------|
| `docs/04` carve and verify | host + the supplied `images/full.img` |
| `docs/05`-`docs/12` boot chain / JFFS2 | RP5 + UART + image store |
| `docs/24` RP5 as a router | RP5 + Ethernet + a client |
| `docs/16`-`docs/23` backdoors / crypto | RP5 (or the host unit tests) |
| `docs/25` RP5 as a camera | **+ a camera module or USB webcam** |
| `docs/26`-`docs/27` build and flash | host + image store |

## 7. Isolated-lab rule (non-negotiable)

Keep the lab **off any production network**. The collector is **local**. Use a dedicated
switch or a host-only network. You are studying a device class that ships backdoors -
do not bridge it to anything you care about.

## Glossary of the two items people ask about

### PSU (Power Supply Unit)
The **wall power adapter**. For the RP5 the correct PSU is **USB-C, 5 V / 5 A (27 W)**.
The RP5 negotiates **USB-C Power Delivery**; a weaker supply still boots but throttles
the CPU and cannot reliably power USB peripherals such as the webcam. Use the official
27 W supply or any PD supply that advertises 5 V / 5 A. Charge-only phone chargers
(often 5 V / 2-3 A) are not sufficient.

### USB-UART adapter
A small board that converts a **USB port on your host** into a **3.3 V serial (UART)
port**, used to read the board's **serial console** - the only place the boot log
(`U-Boot -> Linux -> app`) appears.

| adapter | direction | RP5 |
|---------|-----------|-----|
| GND | - | GND |
| TX | -> | **RX** (GPIO 15) |
| RX | <- | **TX** (GPIO 14) |

Rules:

- Use a **3.3 V** adapter, never 5 V logic.
- Do **not** connect the adapter's VCC; power the RP5 from its own PSU.
- Settings are **115200 8N1**: `screen /dev/tty.usbserial-XXXX 115200`.
- Common chips: **CP2102**, **CH340**, **FT232/FTDI**. Easiest option: the
  **Raspberry Pi Debug Probe**, which connects to the RP5's dedicated 3-pin UART header.
