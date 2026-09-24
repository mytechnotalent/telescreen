# Walkthrough 79 - A Real OpenWrt Device on the Pi (Mango prep)

**turn a Raspberry Pi 4B or Pi 5 into a genuine OpenWrt router - the same OS family as a GL.iNet Mango**

***
**LEGAL DISCLAIMER:** The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with. **IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
***

[Walkthrough 78](78-raspberry-pi-bringup.md) put **Raspberry Pi OS** on the Pi so
you could run the TELESCREEN application. This walkthrough goes further: it puts
**OpenWrt** on the Pi, so you are holding a **real embedded Linux router** - the
same software family as a GL.iNet Mango, a travel router, or most home routers.

Everything here applies to the **Raspberry Pi 4 Model B** and the
**Raspberry Pi 5**; differences are flagged `[Pi 4B]` / `[Pi 5]`.

---

## 79.1 Why OpenWrt (and why it prepares you for a Mango)

A **Mango** (GL.iNet GL-MT300N-V2) runs **OpenWrt**. If you can drive OpenWrt on
a Pi, you can drive a Mango, a Brume, an AR300M, and a huge fraction of real
routers. The same skills transfer directly:

| skill | where you learn it here | where you use it on a Mango |
| ----- | ----------------------- | --------------------------- |
| serial console | this walkthrough | the 4-pin UART header |
| **UCI** config | `uci show`, `/etc/config` | the whole config system |
| **ubus** / `rpcd` | `ubus list`, `ubus call` | LuCI and the daemons | 
| **squashfs + overlay** | `/rom` + `/overlay` | the read-only firmware + writable overlay |
| **sysupgrade** | `sysupgrade -l` | flashing a new firmware |
| package manager | `apk` (25.12+) / `opkg` | adding packages |
| boot chain | `/boot` (firmware + `kernel8.img`) | the vendor bootloader |

---

## 79.2 Get the OpenWrt image

OpenWrt publishes official images for the Pi 4 and Pi 5. The `25.12.5` release is
current at the time of writing; check
<https://downloads.openwrt.org/releases/> for the newest.

```bash
B=https://downloads.openwrt.org/releases/25.12.5/targets/bcm27xx

# Raspberry Pi 4 (SoC bcm2711)
curl -L -o openwrt-rpi4.img.gz \
  "$B/bcm2711/openwrt-25.12.5-bcm27xx-bcm2711-rpi-4-squashfs-factory.img.gz"

# Raspberry Pi 5 (SoC bcm2712)
curl -L -o openwrt-rpi5.img.gz \
  "$B/bcm2712/openwrt-25.12.5-bcm27xx-bcm2712-rpi-5-squashfs-factory.img.gz"

gzip -dk openwrt-rpi4.img.gz        # -> openwrt-rpi4.img (~79 MB)
```

- **squashfs** = compressed, read-only firmware + a writable overlay (the
  classic router layout). Use this for the course.
- **ext4** variant = a plain writable rootfs (simpler, less "router-like").
- **factory** = for a blank card. **sysupgrade** = for upgrading a running
  OpenWrt.

> The Pi images boot through the Raspberry Pi firmware (`start4.elf`), not a
> separate U-Boot, so the layout is FAT `/boot` + squashfs rootfs. That is
> normal for OpenWrt on the Pi.

---

## 79.3 Flash it

Exactly like Walkthrough 78, but with the OpenWrt image.

**macOS:**

```bash
diskutil list                       # find the card, e.g. /dev/disk5 ("USB3.0 CRW -SD")
diskutil unmountDisk /dev/disk5
sudo dd if=openwrt-rpi4.img of=/dev/rdisk5 bs=4m     # Pi 5: openwrt-rpi5.img
sync
```

**Linux:** `sudo dd if=openwrt-rpi4.img of=/dev/sdX bs=4M conv=fsync`

Move the card to the Pi and power on (`[Pi 5]`: press the power button).

---

## 79.4 First boot and the serial console

Keep the USB-UART wired exactly as in Walkthrough 78 (**GND=6, TX=8, RX=10**,
crossed to the adapter). Open the console at **115200 8N1**:

```bash
screen /dev/cu.usbserial-A50285BI 115200     # macOS; Linux: /dev/ttyUSB0
```

You will see the OpenWrt boot and the banner, then land at a **root shell**:

```
OpenWrt 25.12.5, r33051-f5dae5ece4
 -----------------------------------------------------
root@OpenWrt:~#
```

There is **no root password** yet (OpenWrt warns you). Set one:

```bash
passwd                      # enter it twice
```

Now SSH will work once you have networking.

---

## 79.5 Explore the device (the Mango workflow)

These are the commands you will use on any OpenWrt router.

```bash
cat /etc/openwrt_release           # version, target, revision
uname -a                           # kernel
mount                              # note /rom (squashfs) and /overlay (ext4)
df -h                              # /overlay is the writable space
cat /proc/partitions               # mmcblk0p1 (boot), mmcblk0p2 (rootfs)
ls -la /boot                       # the Pi firmware + kernel8.img + dtbs
ls /etc/config                     # network, wireless, firewall, dhcp, system, ...
uci show network                   # br-lan, 192.168.1.1/24
uci show system                    # hostname, timezone, logging
ubus list                          # services (dhcp, dnsmasq, hostapd, ...)
ubus call system board             # hardware identity
sysupgrade -l                      # files a firmware upgrade preserves
apk --version                      # package manager (25.12 uses apk; older: opkg)
```

What you are looking at:

- `/rom` = the **read-only squashfs** firmware image.
- `/overlay` = the **writable ext4** layer; your changes live here.
- `overlayfs:/overlay` on `/` = the two joined into one writable root.
- `br-lan` on `eth0` = the LAN bridge, `192.168.1.1`.

> **`[Pi 4B / Pi 5]` overlay size.** The stock image gives a small overlay
> (tens of MB). On a real router the flash is small too, which is the lesson.
> To use the whole card you grow the rootfs partition and the overlay (a
> standard OpenWrt task); for this course a small overlay is fine, and Ethernet
> + the shell is all you need.

---

## 79.6 Make it a Mango (a real travel router)

This gives the Pi the same shape as a GL.iNet Mango: **Ethernet WAN** + **Wi-Fi
LAN access point** + **NAT/firewall** + **DHCP/DNS**. A Mango ships at
`192.168.8.1`, but **do not blindly copy that**: `192.168.8.1` is one of the most
common router defaults (GL.iNet and many others), so it is very likely to
**clash** with the network you are already on. Pick a subnet nobody uses; this
course uses **`192.168.77.1/24`**.

> **How to pick a safe LAN subnet.** Avoid the popular defaults:
> `192.168.0.1`, `192.168.1.1`, `192.168.8.1` (GL.iNet), `192.168.10.1`,
> `192.168.50.1` (Asus), `192.168.68.1` (TP-Link), `192.168.88.1` (MikroTik),
> `10.0.0.1`. Something odd like `192.168.77.1` or `172.16.42.1` is unlikely to
> collide.

All of this is plain UCI; run it over the serial console:

```bash
# 1. LAN address (a CLASH-FREE subnet - NOT the 192.168.8.1 default)
uci set network.lan.ipaddr='192.168.77.1/24'

# 2. Make eth0 the WAN (DHCP from upstream) and drop it from the LAN bridge
uci del network.@device[0].ports
uci set network.wan=interface
uci set network.wan.device='eth0'
uci set network.wan.proto='dhcp'
uci set network.wan6=interface
uci set network.wan6.device='eth0'
uci set network.wan6.proto='dhcpv6'

# 3. Wi-Fi LAN access point
RAD=$(uci show wireless | sed -n 's/^\(wireless\.[^=]*\)=wifi-device/\1/p' | head -1)
IF=$(uci show wireless  | sed -n 's/^\(wireless\.[^=]*\)=wifi-iface/\1/p' | head -1)
uci set system.@system[0].hostname='TELESCREEN'
uci set $RAD.disabled='0'
uci set $RAD.country='US'          # set YOUR country
uci set $IF.disabled='0'
uci set $IF.mode='ap'
uci set $IF.ssid='TELESCREEN'
uci set $IF.encryption='psk2'
uci set $IF.key='telescreen'       # change this
uci set $IF.network='lan'

uci commit
wifi reload
/etc/init.d/network restart
```

Verify:

```bash
iwinfo phy0-ap0 info | head -5      # SSID TELESCREEN, Mode Master, WPA2
ip addr show br-lan | grep inet     # 192.168.77.1/24
ps w | grep '[d]nsmasq'             # DHCP/DNS running
```

Then, from **another** device (a phone is perfect), join **`TELESCREEN`**
(password `telescreen`) and open:

- **LuCI**: <http://192.168.77.1>
- **SSH**: `ssh root@192.168.77.1` (password from 79.4)

> **Test from a *different* device, not the one you are working on.** Joining the
> AP moves that device onto the Pi's network; if the Pi has no WAN yet, that
> device loses internet. A phone or a second laptop keeps your main machine
> online while you poke at the router.

### Giving the Wi-Fi clients real internet (WAN options)

The AP and LuCI work with **no** upstream at all. To actually *share* internet,
the Pi needs a WAN path. The onboard Wi-Fi radio can only be an **AP** *or* a
**client** (`Supports VAPs: no`), so a single radio cannot repeat. Your options:

| method | what you need | notes |
| ------ | ------------- | ----- |
| **Ethernet WAN** (simplest) | a cable from `eth0` to your router/switch | the Pi NATs Wi-Fi clients - exactly a Mango in Ethernet-WAN mode |
| **USB Wi-Fi dongle** | a supported dongle | dongle radio = client/WAN, onboard radio = AP; needs `kmod-*` packages |
| **USB phone tethering** | a phone + USB cable | phone shares mobile data; needs `kmod-usb-net-rndis` (or similar) |

So: **with a cable** it just works; **without one**, add a second radio (a cheap
USB Wi-Fi dongle) or tether a phone. None of this touches your existing
`192.168.8.0/24` network - the Pi is its own island until you feed it a WAN.

---

## 79.7 Pi 4B vs Pi 5 - what changes

| | Raspberry Pi 4 Model B | Raspberry Pi 5 |
| - | ---------------------- | -------------- |
| OpenWrt target | **bcm2711** | **bcm2712** |
| Image | `...bcm2711-rpi-4-squashfs-factory.img.gz` | `...bcm2712-rpi-5-squashfs-factory.img.gz` |
| Power | 5 V / 3 A | **5 V / 5 A USB-C PD** |
| Power on | automatic | **press the button** |
| Serial UART | GPIO 8 / 10 / 6 | **same** GPIO 8 / 10 / 6 |
| Cooler | optional | **recommended** |
| Everything else | identical | identical |

---

## 79.8 From here to a real device (the Mango)

On a Mango you will:

1. Open a **serial console** on its UART header - same as 79.4.
2. Read the boot log and interrupt the bootloader (the Mango's U-Boot).
3. Log in and inspect `/etc/config`, `uci`, `ubus`, `sysupgrade` - same as 79.5.
4. Back up and restore configuration, flash a new firmware - same as OpenWrt.

The Pi is a **safe, unbrickable classroom** for exactly those steps. Break it,
re‑flash it, repeat.

---

## 79.9 The other "real thing" - a custom U-Boot device

OpenWrt is the *software* side of a real device. If you also want the *bootloader*
side - a **U-Boot prompt**, a **`mtdparts`/four-region layout**, an environment
with a CRC, and a kernel the bootloader loads - that is a separate build
(U-Boot for `rpi_4`/`rpi_5` + a Linux `Image` + a minimal rootfs, chained from
the Pi's FAT boot partition). It is the deepest, most "camera/router internals"
version of the lab and can be layered on top of this.

---

## 79.10 Troubleshooting

| symptom | cause | fix |
| ------- | ----- | --- |
| No serial output | TX/RX swapped | swap adapter RX/TX (pins 8/10) |
| Boots but no network | `192.168.1.1` clash / no cable | connect directly to laptop, or change LAN IP |
| `Bad password: too weak` | password too simple | enter a stronger one, or retype the same to force it |
| Wi-Fi won't enable | country / radio disabled | `uci set wireless.radio0.disabled=0`, set country, `wifi reload` |
| `/overlay` nearly full | small stock overlay | remove packages, use ext4 image, or grow the partition |
| `[Pi 5]` no boot | button / power | press power; use a 5 V/5 A supply |

---

## 79.11 Working completely offline

**The Pi does not need internet for this lab.** The router, the serial console,
LuCI, UCI, ubus, the Wi-Fi AP, DHCP/DNS, the firewall, squashfs+overlay and
sysupgrade all work with **no upstream at all**. Treat the Pi as an isolated lab
router.

| works offline | needs internet |
| ------------- | -------------- |
| serial console, root shell | `apk update` / `apk add` (installing packages) |
| LuCI web UI | sharing *real* internet to clients (WAN to the outside) |
| UCI, ubus, `/etc/config` | downloading a new firmware image |
| Wi-Fi AP + DHCP/DNS + firewall rules | NTP time sync |
| squashfs + overlay, sysupgrade backup/restore | |
| the whole TELESCREEN reverse-engineering lab (Ghidra runs on your computer) | |

If you later need a package on the offline Pi, download the `.apk` on a machine
that *does* have internet and copy it in over the SD card or the serial console -
the Pi itself never has to be online.

---

## 79.12 What you have now

- a Raspberry Pi (4B or 5) running **OpenWrt 25.12.5** - a real embedded Linux,
- a **serial console** and **root access**,
- hands-on with **UCI, ubus, squashfs+overlay, sysupgrade, apk**,
- and the exact skills you will use the first time you open a **Mango** - all
  with **no internet required** on the device.

Next: revisit [Walkthrough 78](78-raspberry-pi-bringup.md) if you ever want
Raspberry Pi OS back, or ask about the **custom U-Boot four-region build**.
