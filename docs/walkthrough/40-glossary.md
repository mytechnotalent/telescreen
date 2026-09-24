# Walkthrough 40: Glossary

**every term in the course, defined**

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

## A

**AEAD** - Authenticated Encryption with Associated Data. Gives confidentiality **and**
integrity in one construction. `docs/18`.

**AArch64** - the 64-bit ARM instruction set. The RP5 runs it. `docs/05`.

**A32** - the 32-bit ARM instruction set (ARM mode). The `0xEA` branch table is an A32
feature. `docs/05`.

**AP** - Access Point. The Wi-Fi mode the device uses for the LAN. `docs/24`.

**Argon2id** - a memory-hard password KDF. `docs/23`.

## B

**beacon** - the periodic sealed telemetry the device sends to its collector. `docs/17`.

**bootargs** - the kernel command line, stored in the U-Boot environment. `docs/06`.

**bootcmd** - the command U-Boot runs to load the kernel. `docs/06`.

**BootROM** - the closed on-die first stage. `docs/05`.

## C

**CGI** - here, an internal HTTP route in the application, not a filesystem file.
`docs/14`.

**container** - the vendor kernel wrapper `[magic][len]["gziphead"][gzip]`. `docs/08`.

**CRC** - Cyclic Redundancy Check. **Integrity, not authenticity.** `docs/03`.

**`crc32_le`** - the reflected CRC-32 JFFS2 uses (seed 0, no final inversion).
`docs/walkthrough/01`.

**CLEANMARKER** - a JFFS2 node marking an erased block. `docs/11`.

## D

**DHCP** - Dynamic Host Configuration Protocol; how the AP hands out leases. `docs/24`.

**DNS** - Domain Name System; `dnsmasq` resolves for clients. `docs/24`.

**DT / DTB / DTS** - Device Tree / Blob / Source. The machine description. `docs/09`.

## E

**Ed25519** - an EdDSA signature scheme; the identity primitive. `docs/22`.

**erase block** - the smallest erase unit; 64 KiB here. `docs/02`.

## F

**FDT** - Flattened Device Tree. `docs/09`.

**flash** - non-volatile memory; here, the image store. `docs/03`.

## G

**GCM** - Galois/Counter Mode, the AEAD used with AES. `docs/19`.

**GHASH** - GCM's polynomial MAC. `docs/walkthrough/18`.

**ghash/PMULL** - the carry-less multiply used by GHASH. `docs/19`.

## H

**HKDF** - HMAC-based Key Derivation Function. `docs/21`.

**HMAC** - Hash-based Message Authentication Code. `docs/21`.

## I

**IGD** - Internet Gateway Device; the UPnP port-mapping service. `docs/walkthrough/24`.

**INODE** - a JFFS2 node carrying file metadata and data. `docs/11`.

**ISP** - Image Signal Processor; the camera pipeline. `docs/25`.

## J

**JFFS2** - Journalling Flash File System 2. `docs/11`.

## K

**KDF** - Key Derivation Function. `docs/17`, `docs/21`.

## L

**LR** - Link Register; the return address on ARM. `docs/05`.

## M

**magic** - a fixed byte sequence that identifies a format. `docs/02`.

**MTD** - Memory Technology Device; the raw-flash abstraction. `docs/02`.

**MJPEG** - Motion JPEG; the app-free HTTP stream. `docs/25`.

## N

**nonce** - a number used once. **Never reuse under one key.** `docs/18`.

**NOR** - a flash technology with random-access reads. `docs/03`.

## O

**ONVIF** - the camera interoperability standard (SOAP). `docs/walkthrough/24`.

## P

**PMULL** - the ARMv8 carry-less multiply instruction. `docs/19`.

**Poly1305** - the one-time MAC in ChaCha20-Poly1305. `docs/20`.

**PPPP** - a real-world P2P/cloud protocol (Xiongmai lineage). The lab keeps the shape.
`docs/walkthrough/25`.

## R

**reset handler** - the first code executed after reset; the target of `vector[0]`.
`docs/05`.

**RTSP** - Real Time Streaming Protocol; the camera stream. `docs/25`.

## S

**SOAP** - the XML protocol ONVIF uses. `docs/walkthrough/24`.

**SPL** - Secondary Program Loader; the first stage in SRAM. `docs/05`.

**SSDP** - the UPnP discovery protocol. `docs/walkthrough/24`.

**squashfs** - a read-only compressed filesystem. `docs/walkthrough/37`.

## T

**tmpfs** - a RAM filesystem; where the writable state lives. `docs/10`.

## U

**U-Boot** - the main bootloader. `docs/06`.

**UBI / UBIFS** - a NAND wear-management layer and its filesystem.
`docs/walkthrough/37`.

**UVC** - USB Video Class; the webcam standard. `docs/25`.

## V

**VBAR** - Vector Base Address Register; where the CPU finds the exception table.
`docs/05`.

## W

**WAN** - Wide Area Network; the uplink. `docs/24`.

## X

**X25519** - ECDH on Curve25519. `docs/21`.

**XChaCha20-Poly1305** - ChaCha20-Poly1305 with a 192-bit nonce. `docs/20`.

## Numbers and Symbols

**`0x001B8421`** - the kernel container magic. `docs/08`.

**`0x1985`** - the JFFS2 node magic (bytes `85 19`). `docs/11`.

**`0xEA000515`** - an A32 branch: `B 0x145C`. `docs/05`.

**`0xEDB88320`** - the reflected CRC-32 polynomial. `docs/walkthrough/01`.

## Exercises

1. Cover the right column and define each term.
2. Add five terms the course uses that are missing here.
3. For each term, name the volume that teaches it.

## Reference

- `docs/30`, every volume
