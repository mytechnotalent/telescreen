# Walkthrough 78 - Raspberry Pi Bring-Up (Pi 4B and Pi 5)

**flash the card, configure it headless, wire the serial console, and see your camera**

***
**LEGAL DISCLAIMER:** The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with. **IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
***

This walkthrough assumes **zero** prior hardware experience. It works for the
**Raspberry Pi 4 Model B** and the **Raspberry Pi 5**. Every command is written
out in full. If you get stuck, read the Troubleshooting section at the end.

> **Every step below is identical on the Pi 4B and the Pi 5** unless you see a
> **`[Pi 5]`** or **`[Pi 4B]`** callout. The only real differences are the power
> supply, the requirement for a 64-bit OS, and a few Pi 5 extras (power button,
> bootloader update, cooling) - all collected in §78.10.

By the end you will have:

- a Raspberry Pi that boots a current Raspberry Pi OS,
- a **serial console** on your Mac/Linux/Windows machine (no monitor needed),
- SSH enabled and a login you chose,
- your **USB camera** visible at `/dev/video0`,
- and the TELESCREEN project copied onto the Pi.

> **Why a serial console?** A headless Pi has no monitor. The UART (a 3-wire
> serial link) lets you watch it boot and log in from your laptop's USB port. It
> is the single most useful tool in embedded work.

---

## 78.1 What you need (parts list)

| item | notes |
| ---- | ----- |
| Raspberry Pi **4 Model B** *or* **5** | either works; differences are called out below |
| microSD card, **16 GB or larger** | Class 10 / A1 is plenty |
| microSD **reader** (USB) | to flash the card from your computer |
| Power supply | **Pi 4B:** 5 V / 3 A USB-C. **Pi 5:** 5 V / 5 A USB-C PD (27 W official) recommended |
| **USB-to-UART adapter**, **3.3 V** | e.g. FTDI FT232R, CP2102, CH340 (this course used an FT232R) |
| 3 **jumper wires** (female-female) | for GND, TX, RX |
| (optional) USB **webcam** | any UVC camera; this course used a Microdia Vitade AF |
| (optional) Ethernet cable | simplest way to get networking without a monitor |

> ⚠️ **The adapter must be 3.3 V logic.** 5 V on the Pi's GPIO can damage the
> SoC. Many adapters have a 5 V/3.3 V jumper — set it to 3.3 V.

---

## 78.2 Get the latest Raspberry Pi OS image

There are two ways. Use whichever you prefer; both end with the same card.

### Option A - Raspberry Pi Imager (easiest, recommended for beginners)

1. Download **Raspberry Pi Imager** for your computer:
   **<https://www.raspberrypi.com/software/>**
2. Install and open it.
3. Click **Choose Device** → your board (**Raspberry Pi 4** or **Raspberry Pi 5**).
4. Click **Choose OS** → **Raspberry Pi OS (other)** → **Raspberry Pi OS Lite (64-bit)**.
   (Lite = no desktop; perfect for a headless lab. Choose the *full* OS only if
   you want a desktop.)
5. Click **Choose Storage** → your **microSD card**.
6. Click the **gear / "Edit Settings"** button and set, under **OS Customisation**:
   - **hostname**: `raspberrypi`
   - **Enable SSH** → *Use password authentication*
   - **username**: `pi`, **password**: something you will remember
   - **Wireless LAN** (optional): your SSID, password, and **country**
   - **Locale**: your time zone
7. Click **Write**. Wait for it to finish and verify.

> If you use Imager's settings, you can **skip section 78.4** — it does the same
> thing for you.

### Option B - download the image yourself (used in this course)

The current image (verified for this course) is **Raspberry Pi OS Lite, 64-bit,
Debian 13 "trixie", released 2026-09-15** (kernel 6.18.50). The `_latest` link
always points at the newest:

```bash
# macOS / Linux
curl -L -o raspios-lite-arm64.img.xz \
  https://downloads.raspberrypi.com/raspios_lite_arm64_latest
xz -dk raspios-lite-arm64.img.xz        # produces raspios-lite-arm64.img (~3 GB)
```

Other links:

- All images and versions: <https://www.raspberrypi.com/software/operating-systems/>
- Direct "latest lite arm64": <https://downloads.raspberrypi.com/raspios_lite_arm64_latest>

> **Pi 5 note:** the Pi 5 *requires* the **64-bit** OS. The `arm64` image above
> is correct for both the Pi 4B and the Pi 5.

---

## 78.3 Flash the card

### With Imager

Just **Write** (section 78.2, Option A) — done.

### From the command line (macOS)

Insert the card, find its device name, and write the image. **Read carefully** —
this **erases the card**.

```bash
diskutil list                 # find your card, e.g. /dev/disk4  ("USB3.0 CRW -SD")
diskutil unmountDisk /dev/disk4
sudo dd if=/path/to/raspios-lite-arm64.img of=/dev/rdisk4 bs=4m
sync
```

**Linux:**

```bash
lsblk                         # find your card, e.g. /dev/sdb (NOT a partition like sdb1)
sudo umount /dev/sdb?*
sudo dd if=raspios-lite-arm64.img of=/dev/sdb bs=4M status=progress conv=fsync
sync
```

> Double-check the device name. On macOS use the whole disk (`/dev/rdisk4`, not
> `/dev/disk4s1`); on Linux use `/dev/sdb`, not `/dev/sdb1`.

---

## 78.4 Configure it headless (SSH + user + UART)

This is the part Imager does automatically. If you used Imager's settings, skip
to 78.5. Otherwise, after flashing the card, a small **FAT32** partition mounts
as **`bootfs`** (macOS) or **`boot`** (older). On it:

### 1. Enable SSH

Create an **empty** file named `ssh`:

```bash
# macOS example (the partition mounted at /Volumes/bootfs)
touch /Volumes/bootfs/ssh
```

### 2. Create your user (there is no default user any more)

Create `userconf.txt` containing `username:hashed-password`. Generate the hash:

```bash
openssl passwd -6 'yourpassword'
# $6$....long string....
```

Then write it (this example uses username `pi`):

```bash
printf 'pi:%s\n' "$(openssl passwd -6 'yourpassword')" > /Volumes/bootfs/userconf.txt
```

### 3. Turn the GPIO serial port on

Append one line to **`config.txt`**:

```
enable_uart=1
```

> **Where is `config.txt`?** On current Raspberry Pi OS (Bookworm/Trixie - both
> Pi 4B and Pi 5) it lives on the boot partition at
> **`/boot/firmware/config.txt`**; on older Pi 4 images it was `/boot/config.txt`.
> On your computer it is the `config.txt` inside the mounted `bootfs` partition.

### 4. Confirm the console is on serial

**`cmdline.txt`** should contain the word `console=serial0,115200`. On current
Raspberry Pi OS images it already does. If it is missing, add it (keep the file
on a single line):

```
console=serial0,115200 console=tty1 root=PARTUUID=... rootfstype=ext4 fsck.repair=yes rootwait
```

Then **eject** the card and move it to the Pi:

```bash
diskutil eject /dev/disk4      # macOS
```

---

## 78.5 Wire the USB-UART adapter to the Pi

This is the only physical step. You need **three** wires: **GND**, **TX**, **RX**.

### Find pin 1 (this is the whole trick)

The 40-pin header is a **2x20 block**. Numbering always starts at **pin 1**, and
pin 1 is:

- the corner pin **nearest the microSD card slot**, and
- the one with a **square solder pad** on the underside (the others are round),
- and it reads **3.3 V** to a GND pin (the pin beside it reads **5 V**).

### The three pins you need

| signal | physical pin | GPIO |
| ------ | ------------ | ---- |
| **GND** | **6** (or 9) | — |
| **TXD** (Pi sends) | **8** | GPIO14 |
| **RXD** (Pi receives) | **10** | GPIO15 |

Pins 6, 8 and 10 are all on the **same row** (the even row).

### Cross TX and RX

| USB-UART adapter | → | Pi pin |
| ---------------- | - | ------ |
| GND | → | **6** |
| TXD | → | **10** (RXD) |
| RXD | → | **8** (TXD) |
| 3.3 V / 5 V | ✗ | **leave disconnected** |

Power the Pi from its **own** supply, never from the adapter.

### Pi 5 difference

The Pi 5 has the **same** GPIO UART on pins 8/10/6, so the table above applies.
The Pi 5 **also** has a dedicated **3-pin debug UART** (a tiny JST-SH connector
next to the USB-C port) intended for the Raspberry Pi Debug Probe. Either works;
the GPIO pins are the beginner path.

---

## 78.6 Open the serial console from your computer

Speed is **115200, 8 data bits, no parity, 1 stop bit (8N1)**, no flow control.

**macOS / Linux:**

```bash
ls /dev/tty.* /dev/ttyUSB* /dev/cu.usb*     # find your adapter
# macOS example:
screen /dev/cu.usbserial-A50285BI 115200
# Linux example:
screen /dev/ttyUSB0 115200
```

Exit `screen` with **Ctrl-A** then **K**. (`tio /dev/ttyUSB0` is a nicer
alternative if installed.)

**Windows:** install **PuTTY**, choose *Serial*, set the COM port (from Device
Manager), speed **115200**, then Open.

**No `screen`?** Any serial terminal works: PuTTY, `minicom`, `tio`, the
Arduino IDE's serial monitor, or the Raspberry Pi Imager's own console.

---

## 78.7 First boot

Power the Pi. On first boot it **expands the filesystem and reboots once** — this
is normal; do not panic if the log stops halfway.

You should see the kernel log, then a login prompt:

```
Debian GNU/Linux 13 raspberrypi ttyS0
raspberrypi login:
```

Log in with the username/password you set in 78.4 (this course used
`pi` / `telescreen`). Then check you are on **64-bit**:

```bash
uname -m        # aarch64
cat /etc/os-release | head -2
```

`aarch64` means the same CPU family as the RP5 and the same userspace the
TELESCREEN binary is built for.

> **No output at all?** The two data wires are probably swapped. Power off, swap
> the adapter's TX/RX wires, and try again — that fixes ~90% of cases.

> **`[Pi 5]` first-boot extras.** Press the **power button** to start the Pi 5
> (a Pi 4B starts the instant power is applied). Fit an **active cooler** - the
> Pi 5 throttles under sustained load without one. Once online, update the
> bootloader:
>
> ```bash
> sudo rpi-eeprom-update -a && sudo reboot
> ```

---

## 78.8 See your camera

**USB webcam (UVC):**

```bash
ls -l /dev/video*          # you want /dev/video0
v4l2-ctl --list-devices    # names the camera, e.g. "USB Camera: ... (/dev/video0)"
lsusb                      # e.g. "Microdia Webcam Vitade AF"  -> it is a UVC cam
```

**CSI camera (Pi Camera on the ribbon connector):**

```bash
rpicam-hello --list-cameras    # Raspberry Pi OS Bookworm/Trixie
```

**Prove it actually captures a frame** (UVC webcam):

```bash
v4l2-ctl --device /dev/video0 \
  --set-fmt-video=width=640,height=480,pixelformat=MJPG \
  --stream-mmap=3 --stream-count=1 --stream-to=/tmp/frame.mjpg
ls -l /tmp/frame.mjpg       # a few tens of KB means a real frame was captured
```

The TELESCREEN lab's `camera.c` helpers build the RTSP/MJPEG URLs that a client
such as VLC uses to pull video from a UVC camera.

> **`[Pi 5]` cameras.** The Pi 5 has **two** 4-lane MIPI connectors
> (CAM/DISP0 and CAM/DISP1) rather than the Pi 4B's single CSI connector. USB/UVC
> webcams behave identically on both boards and appear at `/dev/video0`.

---

## 78.9 Put the TELESCREEN project on the Pi

Simplest path (no network needed): the project is a tarball on the card's boot
partition.

```bash
mkdir -p ~/telescreen
cp /boot/firmware/telescreen-project/telescreen-and-ctf.tar.gz ~/telescreen/
tar xzf ~/telescreen/telescreen-and-ctf.tar.gz -C ~/telescreen
ls ~/telescreen
```

> The tar messages about *"time stamp ... in the future"* are harmless
> (macOS/UTC clock skew). The files extract fine.

Build and run the lab natively (the Pi **is** aarch64, so no Docker needed):

```bash
sudo apt-get update && sudo apt-get install -y build-essential libssl-dev
cd ~/telescreen/telescreen
gcc -O2 -Iinclude -o teled src/*.c -lcrypto
./teled; echo "exit=$?"
```

Do the Ghidra reverse-engineering from your laptop against
`firmware/teled.stripped` (see [Volume 33](../33-ghidra-nation-state-re.md)).

### Optional: networking

If you want SSH from your laptop instead of the serial cable:

- plug in an **Ethernet** cable (simplest), or
- set **Wi-Fi** (Imager settings, or `sudo raspi-config` → System → Wireless LAN;
  **set the country first** or Wi-Fi stays `rfkill`-blocked), then:

```bash
hostname -I          # the Pi's IP
```

From your laptop: `ssh pi@<ip>`.

---

## 78.10 Pi 4B vs Pi 5 - the differences that matter

| | Raspberry Pi 4 Model B | Raspberry Pi 5 |
| - | ---------------------- | -------------- |
| OS | 32- or 64-bit (use 64-bit here) | **64-bit only** |
| Power | 5 V / 3 A USB-C | **5 V / 5 A USB-C PD** (27 W) recommended |
| Power button | none (starts on power) | **yes** (press to start) |
| Bootloader update | `rpi-eeprom-update` (optional) | **`sudo rpi-eeprom-update -a`** (recommended) |
| Cooling | heatsink optional | **active cooler strongly recommended** |
| HDMI | 2x micro-HDMI | 2x micro-HDMI |
| GPIO header | 40-pin, pin 1 nearest microSD | **same** 40-pin layout, pin 1 nearest microSD |
| GPIO UART | pins **8 / 10 / 6** | **same** pins **8 / 10 / 6** |
| Dedicated debug UART | none | **yes** (3-pin JST-SH, for the Debug Probe) |
| Camera connectors | 1x CSI (15-pin) | **2x** 4-lane MIPI (CAM/DISP) |
| USB | 2x USB3 + 2x USB2 | 2x USB3 + 2x USB2 (via RP1) |
| Audio jack | 3.5 mm analogue | **none** |
| PCIe | no | **yes** (single-lane, needs enabling) |
| RTC / fan header | no | **yes** |
| Image used here | Raspberry Pi OS Lite (64-bit) | the **same** image |

**In short:** for this course, the *only* Pi 5 things you must remember are
**(1) use the 64-bit image, (2) give it 5 V/5 A, (3) press the power button,
(4) fit a cooler.** Everything else - flashing, `enable_uart=1`, `ssh`,
`userconf.txt`, the three wires, the serial console, the camera - is the same on
both boards.

Everything else in this walkthrough — flashing, `enable_uart=1`, headless
`ssh`/`userconf.txt`, the three wires, the serial console — is identical.

---

## 78.11 Troubleshooting

| symptom | cause | fix |
| ------- | ----- | --- |
| No output on serial | **TX/RX swapped** | swap the two data wires (pin 8 <-> pin 10) |
| Green LED blinks a repeating pattern; no boot | card has no bootable OS, or is corrupt | re-flash a current Raspberry Pi OS (78.2-78.3) |
| `raspberrypi login:` then rejects your password | `userconf.txt` wrong/missing | re-create it (78.4 step 2); or use Imager settings |
| 0 bytes on the port | adapter in 5 V mode, or wrong port | set 3.3 V; confirm the device with `ls /dev/tty*` |
| Wi-Fi will not come up (`rfkill`) | country not set | `sudo raspi-config` → Localisation → WLAN Country, reboot |
| `/dev/video0` missing | camera not UVC, or unplugged | check `lsusb`; for CSI use `rpicam-hello --list-cameras` |
| `Permission denied` opening `/dev/video0` | not in `video` group | add your user: `sudo usermod -aG video $USER` (re-login) |
| Screen prints only `??` or gibberish | wrong baud | set exactly **115200 8N1** |
| `[Pi 5]` nothing happens with power connected | Pi 5 needs the button | press the **power button** once |
| `[Pi 5]` reboots or slows under load | cooling / power | fit an **active cooler**; use a **5 V / 5 A** supply |
| `[Pi 4B / Pi 5]` `config.txt` edits ignored | wrong file | edit `/boot/firmware/config.txt` (older Pi 4: `/boot/config.txt`) |

---

## 78.12 What you have now

- a Raspberry Pi (4B or 5) that boots **Raspberry Pi OS Lite (64-bit)**,
- a working **serial console** from your laptop,
- **SSH** and a user you chose,
- your **USB camera** at `/dev/video0`,
- and the **TELESCREEN project** running natively on the Pi.

Next: [Volume 33 - Ghidra and the Nation-State RE Workflow](../33-ghidra-nation-state-re.md)
to reverse `firmware/teled.stripped`, and
[Walkthrough 77](77-function-resolution.md) to resolve all 42 functions.
