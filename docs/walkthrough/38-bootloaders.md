# Walkthrough 38: Bootloaders

**U-Boot, SPL, and the closed first stages**

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

## The Shape

A bootloader is a **chain of stages**, each one bringing up more of the machine:

```
ROM (closed) -> first stage (SPL) -> main bootloader (U-Boot) -> kernel
```

- The **ROM** is silicon; you cannot change it.
- The **first stage** fits in SRAM and brings up DRAM (`docs/05`).
- The **main bootloader** runs from DRAM, reads the environment, loads the kernel.

On the RP5 the ROM stage is the **VideoCore firmware** (closed). The first stage you control
is **U-Boot**.

## 1. U-Boot

The workhorse. What it gives you:

| feature | meaning for this course |
| ------- | ----------------------- |
| `mtdparts` | the four-region map (`docs/02`) |
| environment | `bootargs` / `bootcmd` (`docs/06`, `docs/07`) |
| scripting | `bootcmd` runs commands |
| secure boot | **optional** - the lab leaves it off |
| console | the serial prompt |

## 2. The Environment, Again

```
bootargs=console=ttyAMA0,115200 mtdparts=sfc:... root=/dev/mtdblock3 rootfstype=jffs2 rw
bootcmd=sf probe 0;boothz 0x41000000 0x40008000 0x30000 0x1C0000
```

The environment is **CRC-protected** (`docs/07`); U-Boot falls back to defaults if the CRC
is wrong.

## 3. Secure Boot (What Is Missing)

A secure bootloader would:

1. hold a **public key** (burned in, or in one-time-programmable fuses);
2. read the kernel image;
3. verify an **Ed25519 signature** over it (`docs/22`);
4. refuse to jump if the signature is bad.

None of that is enabled on the lab. That is why the CTF can rebuild the kernel freely
(`docs/27`). It is also the single highest-value defence (`docs/walkthrough/30`).

## 4. Other Bootloaders You Meet

| bootloader | where |
| ---------- | ----- |
| U-Boot / Das U-Boot | most ARM SBCs, cameras, routers |
| Barebox | some industrial devices |
| Coreboot | x86 |
| Little Kernel / ABL | some Android/Qualcomm |
| vendor closed | phones, many cameras |

The **method** is the same: find the vector table, read the environment, follow the load.

## 5. Reading an Unknown Bootloader

```
1. The image starts with the reset vector (ARM) or a header (some formats).
2. Find the version string and the build date (provenance).
3. Find the environment (a CRC + key=value blob).
4. Find the load command (the kernel offset and size).
5. Find the hand-off (the jump to the kernel).
```

On the RP5 you build your own U-Boot and read it the same way.

## 6. The Bootloader and Security

| threat | bootloader's role |
| ------ | ----------------- |
| unsigned image | **should** refuse; usually does not |
| password prompt | U-Boot can lock the console |
| env tampering | the CRC catches accidents, not attackers |
| recovery | a corrupt boot region needs an external reflash (`docs/27`) |

## Exercises

1. Find the version string and environment in a U-Boot image you own.
2. Decode the boot command and name the kernel offset/size.
3. Describe, step by step, how secure boot would change the CTF.
4. Name two bootloaders and one way each reads its environment.

## Reference

- `docs/05`, `docs/06`, `docs/07`, `docs/27`
- U-Boot documentation
