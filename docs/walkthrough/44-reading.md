# Walkthrough 44: Reading List and Further Study

**where to go after this course**

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

## The Course Path

This lab is one node in a course. The intended progression:

```
reverse engineering fundamentals -> embedded C -> this lab (embedded hacking) -> forensics
```

| stage | what it adds |
| ----- | ------------ |
| reverse engineering | reading assembly and binaries |
| embedded C | pointers, the stack, the ABI |
| this lab | Cortex-A, Linux, partitions, backdoors, crypto |
| forensics | provenance and attribution |

## Primary Documents

| topic | document |
| ----- | -------- |
| ARM branch decode, vectors | ARM Architecture Reference Manual (ARMv7-A / ARMv8-A) |
| U-Boot | U-Boot documentation (`mtdparts`, `bootargs`, `bootcmd`) |
| Linux boot | `Documentation/arm/` / `arch/arm64/` in the kernel tree |
| device tree | Devicetree Specification; `dtc` |
| MTD / JFFS2 | `mtd-utils`; the JFFS2 format in the kernel docs |
| squashfs / UBI | squashfs and UBI/UBIFS docs |
| AES-GCM | NIST SP 800-38D |
| ChaCha20-Poly1305 | RFC 8439 |
| X25519 | RFC 7748 |
| HKDF | RFC 5869 |
| Ed25519 | RFC 8032 |
| Argon2 | RFC 9106 |
| ONVIF | ONVIF Core Specification |
| UPnP | UPnP IGD specification |
| DHCP / DNS | RFC 2131 / RFC 1035 |

## Books and Papers

| work | why |
| ---- | --- |
| "The Art of Software Security Assessment" | the sink/taint method |
| "Practical Reverse Engineering" | ARM/x86 binaries |
| "Hacking: The Art of Exploitation" | memory-corruption fundamentals |
| "The Hardware Hacking Handbook" | physical and interface attacks |
| "To Kill a Centrifuge" (Langner) | the log-desynchronization idea |
| OWASP Embedded/OT guidance | defensive baselines |

## Tools to Learn Next

| tool | next step |
| ---- | --------- |
| Ghidra | write a script; do a diff |
| radare2 | script analysis |
| OpenOCD / gdb | live debugging on hardware |
| `binwalk` | custom signatures |
| `unsquashfs`/`ubireader` | more filesystems |

## Practice Targets

- **Your own** old routers, cameras, and SBCs.
- Firmware update files from **vendors you own**.
- Capture-the-flag platforms with hardware categories.
- Open firmware projects (**OpenIPC**, OpenWrt) as *reference implementations*, not
  targets.

**Always:** your own hardware, or written authorisation (`docs/29`).

## The Transferable Skills

```
1. get a clean image               (docs/04)
2. identify formats by magic       (docs/02)
3. read a boot chain               (docs/05-docs/10)
4. extract a filesystem            (docs/11)
5. find the app and its sinks      (docs/14, docs/15)
6. find the crypto and its key     (docs/17)
7. write findings, label evidence  (docs/walkthrough/26)
8. fix, rebuild, verify            (docs/26, docs/27)
```

These are the same on any device. The details change; the method does not.

## Exercises

1. Pick one primary document and read the section your favourite volume cites.
2. Do one exercise from each of `docs/walkthrough/19` and `docs/walkthrough/16`.
3. Choose a device you own and apply the eight-step method.
4. Write a one-page study plan from this list.

## Reference

- every volume in `docs/` and `docs/walkthrough/`
