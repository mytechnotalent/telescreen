# Volume 30: Appendix

**tables, magics, constants, and glossary**

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

## A. Partition Reference

| mtd | name | offset | size | magic |
| --- | ---- | ------ | ---- | ----- |
| 0 | `boot` | `0x000000` | 128 KiB | `15 05 00 ea` |
| 1 | `bootargs` | `0x020000` | 64 KiB | CRC32 (LE) + `k=v\0` |
| 2 | `kernel` | `0x030000` | 1792 KiB | `21 84 1b 00` + `gziphead` |
| 3 | `rootfs` | `0x1F0000` | 14400 KiB | `85 19 03 20` |

`mtdparts=sfc:128K(boot),64K(bootargs),1792K(kernel),14400K(rootfs)`

## B. Constants

| constant | value | source |
| -------- | ----- | ------ |
| image size | `0x1000000` | `partition.h` |
| container magic | `0x001B8421` | `container.h` |
| JFFS2 magic | `0x1985` | `jffs2.h` |
| JFFS2 CLEANMARKER | `0x2003` | `jffs2.h` |
| JFFS2 DIRENT | `0xE001` | `jffs2.h` |
| JFFS2 INODE | `0xE002` | `jffs2.h` |
| erase block | `0x10000` | `partition.h` |
| CRC-32 check | `0xCBF43926` | IEEE 802.3 |
| CRC-16/CCITT check | `0x29B1` | standard |
| boot vector word | `0xEA000515` | `boot.img` |

## C. Crypto Reference

| primitive | sizes | standard |
| --------- | ----- | -------- |
| AES-256-GCM | key 32, nonce 12, tag 16 | NIST SP 800-38D |
| XChaCha20-Poly1305 | key 32, nonce 24, tag 16 | RFC 8439 / xchacha |
| X25519 | key 32 | RFC 7748 |
| HKDF-SHA256 | prk 32, okm any | RFC 5869 |
| Ed25519 | key 32, sig 64 | RFC 8032 |
| Argon2id | memory-hard KDF | RFC 9106 |

## D. Module Map

| module | files | role |
| ------ | ----- | ---- |
| daemon | `teled.h/.c` | the application (router + camera + beacon) |
| AEAD | `aead.h/.c` | AES-256-GCM and XChaCha20-Poly1305 behind one API |
| key agreement | `kex.h/.c` | X25519 + HKDF-SHA256 |
| identity | `identity.h/.c` | Ed25519 |
| partitions | `partition.h/.c` | carve and identify the four images |
| environment | `env.h/.c` | U-Boot env CRC read/verify/edit |
| container | `container.h/.c` | the vendor kernel container |
| jffs2 | `jffs2.h/.c` | JFFS2 nodes and `crc32_le` |
| beacon | `beacon.h/.c` | the exfiltration channel (weak and hardened) |
| collector | `collector.h/.c` | the local lab sink |
| camera | `camera.h/.c` | UVC camera URL helpers (RTSP + MJPEG) |
| crc | `crc.h/.c` | `crc32_le`, U-Boot CRC32, CRC-16/CCITT |

## E. Tool Map

| script | role |
| ------ | ---- |
| `audit_c_standard.py` | the C standard (0 violations) |
| `audit_python_standard.py` | the Python standard |
| `run_tests.py` | the native suite |
| `check_coverage.py` | the 100% coverage gate |
| `carve.py` | carve the four partitions |
| `build_images.py` | build the four images |
| `verify_telescreen.py` | hash + magic verification |
| `weak_decrypt.py` | the recovered weak KDF |
| `gen_banner.py` | the ASCII banner |

## F. Glossary

| term | meaning |
| ---- | ------- |
| AEAD | Authenticated Encryption with Associated Data |
| AP | Access Point (Wi-Fi) |
| CRC | Cyclic Redundancy Check (integrity, not authenticity) |
| DT / FDT / DTB | Device Tree / Flattened DT / DT Blob |
| JFFS2 | Journalling Flash File System 2 |
| MTD | Memory Technology Device (raw flash abstraction) |
| NOR | a flash technology (random-access reads) |
| PPPP | Xiongmai peer-to-peer protocol (the cloud path) |
| RTSP | Real Time Streaming Protocol |
| SPL | Secondary Program Loader (first stage) |
| UVC | USB Video Class (webcams) |
| WAN | Wide Area Network (the uplink) |

## G. Command Index

```bash
# carve / verify / build
python3 scripts/carve.py --image CTF-XX-full.img --out carved/
python3 scripts/verify_telescreen.py --image CTF-XX-full.img
python3 scripts/build_images.py --uboot boot.bin --kernel Image --rootfs rootfs/ --out out/

# standards + tests
python3 scripts/audit_c_standard.py
python3 scripts/audit_python_standard.py
python3 scripts/run_tests.py
python3 scripts/check_coverage.py

# reverse engineering
xxd -l 32 boot.img
arm-none-eabi-objdump -D -b binary -m arm boot.img | head
clang --target=aarch64-unknown-linux-gnu -O2 -S src/jffs2.c -Iinclude -o jffs2.s
```

## H. Cross-Reference

| want to learn | read |
| ------------- | ---- |
| the layout | `docs/02` |
| the boot chain | `docs/05`, `docs/06`, `docs/10` |
| the filesystem | `docs/11`, `docs/12` |
| the bugs | `docs/14`-`docs/17` |
| the crypto | `docs/18`-`docs/23` |
| the hardware | `docs/24`, `docs/25`, `PARTS.md` |
| the defense | `docs/28`, `docs/29` |
| the Cortex-A method | `docs/WEEK12_CORTEX_A_RE.md` |
