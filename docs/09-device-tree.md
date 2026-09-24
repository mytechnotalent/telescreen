# Volume 09: Device Tree

**the machine description and the node that maps the image store**

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

- What a device tree (FDT/DTB) is and why Linux needs it
- How to read a DTB back to text with `dtc`
- The RP5 device tree and the nodes that matter to this lab
- How the CMake/build config in this repo mirrors a real device-tree choice

## 1. Why a Device Tree

The arm64 Linux kernel is **generic**: it can run on thousands of boards. The
**device tree** tells it what *this* board is: CPUs, memory, UART, Ethernet, SD/NVMe,
and the peripheral controllers. U-Boot passes the blob (DTB) to the kernel.

The RP5's device tree is the standard Raspberry Pi 5 one
(`bcm2712-rpi-5-b.dtb`). Its root compatible identifies the board:

```
/ {
    compatible = "raspberrypi,5-model-b", "brcm,bcm2712";
    model = "Raspberry Pi 5 Model B";
    ...
};
```

## 2. Read a DTB

The device tree compiles from `.dts` (text) to `.dtb` (binary). Reverse it:

```bash
dtc -I dtb -O dts -o rpi5.dts bcm2712-rpi-5-b.dtb
grep -E 'model|compatible' rpi5.dts | head
```

If `dtc` warns about a malformed mem-reserve header, parse the structure anyway - vendor
and firmware DTBs are often patched at boot, and the header can look odd while the tree
is fine. (This is exactly what you meet when you pull a DTB out of a vendor kernel; see
the PROJECT camera teardown for that worked example.)

## 3. The Nodes That Matter Here

| node | why it matters |
| ---- | -------------- |
| `cpus` / `cpu@0..3` | four Cortex-A76 cores |
| `memory@...` | DRAM base and size |
| `serial@...` | the UART behind `console=ttyAMA0` |
| `ethernet@...` | the WAN interface (`eth0`) |
| `mmc@...` | the SD image store (or NVMe over PCIe) |
| `chosen` / `bootargs` | the runtime command line |

## 4. Reading Your Own Board

Do not trust a tutorial's addresses; trust your board. On a running RP5:

```bash
# the live device tree as text
dtc -I fs -O dts /proc/device-tree 2>/dev/null | head
# or dump the blob
ls /boot/firmware/*.dtb
```

This is the same discipline as `docs/04`: **the bytes on your board are the truth.**

## 5. The Build Choice

In this repo the platform is fixed by CMake - the RP5 target - the same way a real
product fixes its device tree at build time:

```cmake
set(CMAKE_C_STANDARD 11)
# the lab is an application-class ARM64 target (RP5, Cortex-A76)
```

A product that ships one board compiles one DT; a product that ships many picks at
runtime. Either way, the DT is the machine description the kernel reads first.

## 6. Prior Art

The camera teardown in the PROJECT repository extracts a **vendor DTB out of the kernel
partition** and reads `model = "…"`, the flash controller node, and the MTD layout. The
method is identical: find the FDT magic `d0 0d fe ed`, read `totalsize`, parse the
structure block. Your RP5 DT is cleaner because it is a standard one, but the skill is
the same.

## Labs

1. Reverse the RP5 device tree to `.dts` and record the root `compatible`.
2. List the `serial@` node and its `reg` base address. Match it to `console=ttyAMA0`.
3. Find the `memory@` node and record the DRAM base and size.
4. Explain why arm64 needs a device tree when arm32 often did not.

## Reference

- Raspberry Pi 5 device tree (`bcm2712-rpi-5-b.dtb`)
- `dtc` (device tree compiler)
- `docs/08` (kernel container), `docs/10` (cmdline and mount)
