# Walkthrough 21: Hardware and Bench

**the RP5, the camera, the console, and the power, in detail**

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

## The Board

The TELESCREEN lab is a **Raspberry Pi 5 (BCM2712)**. What matters for this course:

| feature | detail | why it matters |
| ------- | ------ | -------------- |
| CPU | 4x Cortex-A76, ARMv8-A | AArch64 code, the crypto extensions |
| RAM | 2/4/8 GB | the image store and the tmpfs |
| Ethernet | 1x Gigabit RJ45 | the WAN interface (`eth0`) |
| Wi-Fi | CYW43455, 802.11ac | the AP (`wlan0`) |
| USB | 2x USB 3.0, 2x USB 2.0 | the webcam, the UART adapter |
| PCIe | 1x (M.2 HAT) | optional NVMe image store |
| Storage | microSD | the four-partition image store |
| UART | a 3-pin debug header | the boot console |
| GPIO | 40-pin | status LEDs, extras |

## The Console (the item people forget)

The boot log appears only on the **UART**, not on HDMI. Without it you are blind to the
boot chain - which is graded.

| adapter pin | connects to RP5 | note |
| ----------- | --------------- | ---- |
| GND | GND | common ground |
| TX | **RX** (GPIO 15) | crosses over |
| RX | **TX** (GPIO 14) | crosses over |

- **3.3 V logic only.** Never 5 V.
- Do **not** connect the adapter's VCC; power the RP5 from its own PSU.
- Settings: **115200 8N1**.

```bash
screen /dev/tty.usbserial-XXXX 115200
# or
picocom -b 115200 /dev/tty.usbserial-XXXX
```

The easiest option is the **Raspberry Pi Debug Probe**, which plugs into the RP5's
dedicated 3-pin UART header.

### What you should see

```
U-Boot 2024.07 (telescreen-rp5)
Hit any key to stop autoboot:  1
Starting kernel ...
[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Machine model: Raspberry Pi 5 Model B
[RCS]: /etc/init.d/S80network
teled: beacon ready
```

If you see U-Boot but not the kernel, the kernel container is wrong (`docs/08`). If you
see the kernel but not the app, the rootfs or the launcher is wrong (`docs/13`).

## The Camera

The default is a **cheap USB webcam** (UVC). It needs no driver work:

```bash
ls -l /dev/video0
v4l2-ctl --device /dev/video0 --list-formats-ext
```

The camera module (CSI) is an alternative and needs the RP5's **15-pin** FPC cable
(`PARTS.md`).

## Power

The RP5 wants **USB-C 5 V / 5 A (27 W) with Power Delivery**. A weaker supply:

- throttles the CPU (`vcgencmd get_throttled` shows the bits);
- cannot reliably power the USB webcam;
- produces flaky behaviour that looks like a firmware bug.

**Measure it:** a USB-C power meter between the supply and the board. If it is under
5 V / 5 A, fix it before blaming the code.

## The Isolated Bench

```
   [ upstream? ]        [ ─────── RP5 ─────── ]        [ client ]
        |                 eth0            wlan0             |
   (or nothing)            |                |                |
                    [ lab switch ]     (Wi-Fi AP)      (phone/laptop)
```

Rules:

- **No production network.** Host-only or a dedicated switch.
- The collector (`lab-sink`) is on this bench, not the internet.
- The webcam faces a wall, not people.

## The Image Store

The four regions live on the microSD (or NVMe). Layout is fixed (`docs/02`). To write it
from a host, use a card reader and `dd` (`docs/27`). To read it back, image the card and
carve (`docs/04`).

## A Practical Bring-Up Order

1. Flash the four images to the card (`docs/27`).
2. Insert the card; connect the UART; power on.
3. Watch U-Boot -> kernel -> app on the console.
4. Plug in the webcam; confirm `/dev/video0`.
5. Bring up the AP and WAN (`docs/24`).
6. Join with a phone; confirm the lease; view the stream (`docs/25`).

## Fault Checklist

| symptom | likely cause |
| ------- | ------------ |
| nothing on the UART | TX/RX are not crossed, or wrong baud |
| no HDMI console | expected - the console is UART |
| boots, no webcam | USB power or the UVC device |
| flaky, resets | PSU under 5 V / 5 A |
| U-Boot but no kernel | bad kernel container (`docs/08`) |
| kernel but no app | bad rootfs or launcher (`docs/13`) |

## The Hardware Lesson

The four-partition model is a **software** idea that maps onto any storage. The camera did
it on SPI-NOR with `flashrom`; the RP5 does it on microSD/NVMe with `dd`. The bench is just
the carrier for the model.

## Exercises

1. Bring up the console and capture the full boot log to a file.
2. Measure the PSU voltage under load; record it.
3. List the USB devices and identify the webcam's VID/PID.
4. Draft your bench diagram and mark the isolation boundary.

## Reference

- `PARTS.md`, `docs/24`, `docs/25`, `docs/27`
