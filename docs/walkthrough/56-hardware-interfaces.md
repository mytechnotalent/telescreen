# Walkthrough 56: Hardware Interfaces

**the buses and pins a firmware engineer meets**

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

## Why Interfaces

Firmware drives **hardware**. Reading a driver means knowing the bus: what the wires carry,
how the controller is programmed, and where the registers live. This volume is the interface
map for this course.

## 1. The Buses You Meet

| bus | wires | typical use | on the lab |
| --- | ----- | ----------- | ---------- |
| **UART** | TX, RX (+GND) | console, GPS, MCUs | the 115200 console |
| **SPI** | SCLK, MOSI, MISO, CS | flash, sensors, displays | the image store (camera) |
| **I2C** | SCL, SDA | sensors, RTC, EEPROM | (camera peripherals) |
| **USB** | D+, D- | cameras, Wi-Fi, storage | the UVC webcam, the UART adapter |
| **MIPI CSI** | differential lanes | image sensors | (an alternative camera) |
| **GPIO** | one pin | LEDs, buttons, reset | status/heartbeat |

## 2. UART

The simplest bus: 8N1, a baud rate, no clock line (both sides agree on timing).

```
TX ---- RX
RX ---- TX        (crossed)
GND --- GND
```

The lab's console is UART at **115200 8N1** (`docs/walkthrough/21`). If you see garbage,
it is a baud or level problem, not a firmware bug.

## 3. SPI

Four wires, a clock, and a chip-select per device. The flash is an SPI target - the SoC is
the controller.

```
SCLK  clock
MOSI  controller -> device
MISO  device -> controller
CS#   select (active low)
```

Programming the flash means issuing SPI **commands** (`0x9F` ID, `0x03` read, `0x20` erase,
`0x02` program) - the layer under `flashrom` (`docs/03`).

## 4. I2C

Two wires, addressed devices, pulled up.

```
SCL  clock
SDA  data (bidirectional)
```

Every device has an **address**; the controller talks to one at a time. Cameras use I2C for
some peripherals and for sensor register access.

## 5. USB

Host and device; the RP5 is a host. A USB device is a set of **descriptors** (vendor, product,
class). The webcam is **UVC** (Video Class).

```bash
lsusb                      # list devices with VID:PID
lsusb -v -d 0000:0000      # descriptors
```

The VID:PID is how a device is identified - the same technique `load_drv.sh` used on the
camera to detect its USB Wi-Fi (`docs/walkthrough/25`).

## 6. MIPI CSI

Differential lanes for a camera sensor. It needs the right **FPC cable** (the RP5 uses a
15-pin connector). A USB webcam avoids this entirely (`docs/25`).

## 7. GPIO

A pin you drive high or low, or read.

```bash
# on the RP5 with libgpiod
gpioset ...        # set a pin
gpioget ...        # read a pin
```

LEDs on GPIO are the simplest status indicator (`docs/walkthrough/21`).

## 8. MMIO: How a Driver Talks to a Controller

A peripheral is a block of **registers** mapped into memory. A driver reads/writes them:

```c
*((volatile uint32_t *)0x120280F8) = 1;   /* poke a control register */
```

The reset handler does exactly this first thing (`docs/05`). `volatile` prevents the
optimiser from removing the access.

## 9. Reading a Schematic-less Board

Without a schematic:

1. **Identify the SoC** (silkscreen, the DT, the kernel log).
2. **Identify the buses** (chips on the board: flash, sensor, Wi-Fi).
3. **Find the registers** in the SoC's reference manual or the device tree.
4. **Find the driver** in the kernel tree.

The **device tree** (`docs/09`) is often the fastest map: it names the controllers and their
addresses.

## 10. The Interface -> Volume Map

| interface | volume |
| --------- | ------ |
| UART console | `docs/walkthrough/21` |
| SPI flash | `docs/03` |
| USB webcam | `docs/25` |
| MMIO / reset | `docs/05` |
| DT addresses | `docs/09` |

## Exercises

1. List the buses on the RP5 and one device on each.
2. Identify the webcam's VID:PID with `lsusb`.
3. Find one MMIO write in the reset handler and name its register block.
4. Map one device-tree node to its bus.

## Reference

- `docs/03`, `docs/05`, `docs/25`, `docs/walkthrough/21`
- the RP5 device tree and the SoC reference manual
