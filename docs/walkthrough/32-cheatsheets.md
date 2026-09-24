# Walkthrough 32: Cheat Sheets

**one-page references for every task**

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

## Partitions

```
mtd0  boot      0x000000  128 KiB   magic 15 05 00 ea
mtd1  bootargs  0x020000   64 KiB   CRC32(LE) + key=value\0
mtd2  kernel    0x030000 1792 KiB   magic 21 84 1b 00 + gziphead
mtd3  rootfs    0x1F0000 14400 KiB  magic 85 19 03 20 (JFFS2)
mtdparts=sfc:128K(boot),64K(bootargs),1792K(kernel),14400K(rootfs)
```

## Carve

```bash
dd if=full.img of=boot.img   bs=1 count=$((0x20000))
dd if=full.img of=env.img    bs=1 skip=$((0x20000)) count=$((0x10000))
dd if=full.img of=kernel.img bs=1 skip=$((0x30000)) count=$((0x1C0000))
dd if=full.img of=rootfs.img bs=1 skip=$((0x1F0000)) count=$((0xE10000))
python3 scripts/carve.py --image CTF-XX-full.img --out carved/
```

## Verify

```bash
python3 scripts/verify_telescreen.py --image CTF-XX-full.img --sha256 <hash>
shasum -a 256 CTF-XX-*.img
```

## Build

```bash
python3 scripts/build_images.py --uboot u-boot.bin --kernel Image --rootfs rootfs/ --out out/
```

## The Four Magics

| region | magic | meaning |
| ------ | ----- | ------- |
| boot | `15 05 00 ea` | ARM `B` at offset 0 |
| kernel | `21 84 1b 00` | vendor container |
| kernel +8 | `67 7a 69 70 68 65 61 64` | `gziphead` |
| rootfs | `85 19 03 20` | JFFS2 + CLEANMARKER |
| rootfs node | `85 19` | JFFS2 node magic |

## Reset Vector Decode

```
word = 0xEA000515
target = (0x08) + ((word & 0xFFFFFF) * 4) = 0x145C
```

## CRCs

| layer | algorithm | seed / final |
| ----- | --------- | ------------ |
| U-Boot env | CRC32 | init `0xFFFFFFFF`, final xor |
| JFFS2 | `crc32_le` | seed 0, **no** final invert |
| frames | CRC-16/CCITT | init `0xFFFF`, poly `0x1021` |
| polynomial | `0xEDB88320` (reflected) | |

Check values: `crc32("123456789")=0xCBF43926`, `crc16("123456789")=0x29B1`.

## JFFS2 Node

```
u16 magic 0x1985 | u16 type | u32 totlen | u32 hdr_crc
hdr_crc = crc32_le(0, node[0:8])
types: 0x2003 CLEANMARKER, 0xE001 DIRENT, 0xE002 INODE, 0x2004 PADDING
INODE: data at node+68; fields ino/version/isize/csize/compr
```

## Crypto

| primitive | sizes | standard |
| --------- | ----- | -------- |
| AES-256-GCM | key 32, nonce 12, tag 16 | SP 800-38D |
| XChaCha20-Poly1305 | key 32, nonce 24, tag 16 | RFC 8439 |
| X25519 | key 32 | RFC 7748 |
| HKDF-SHA256 | prk 32 | RFC 5869 |
| Ed25519 | key 32, sig 64 | RFC 8032 |
| Argon2id | memory-hard | RFC 9106 |

Rule: **never reuse a nonce**; **never derive a key from public data**.

## Gates

```bash
python3 scripts/audit_c_standard.py     # no output
python3 scripts/audit_python_standard.py # exit 0
python3 scripts/run_tests.py            # 0 failures
python3 scripts/check_coverage.py       # exit 0, 100.00%
```

## Console

```
115200 8N1 ; adapter GND->GND, TX->RX(GPIO15), RX->TX(GPIO14) ; 3.3V logic, no VCC
screen /dev/tty.usbserial-XXXX 115200
```

## Router

```bash
hostapd /etc/hostapd.conf -B
dnsmasq --interface=wlan0 --dhcp-range=192.168.50.10,192.168.50.200,12h
nft add rule ip nat postrouting oifname "eth0" masquerade
udhcpc -i eth0
```

## Reverse Engineering

```bash
file X ; readelf -h X | grep Entry ; strings -n6 X | head
xxd -l 32 X ; objdump -d X | head
clang --target=aarch64-unknown-linux-gnu -O2 -S src/m.c -Iinclude -o m.s
r2 -a arm -b 32 -q -c 'aaa; s entry0; pd 20' X
```

## The Finding Format

```
DEFECT : B<n>
WHERE  : partition + file + function/offset
MECH   : the exact line or instruction
PATH   : how to reach it
IMPACT : what you get
FIX    : the change
```

## The One-Page Map

```
1. carve      docs/04
2. boot       docs/05
3. env        docs/07
4. kernel     docs/08
5. rootfs     docs/11
6. backdoors  docs/14-16
7. crypto     docs/17-23
8. router     docs/24
9. rebuild    docs/26-27
10. defend    docs/28
```

## Exercises

1. Print the partitions sheet and fill it from a real device.
2. Add a sheet for "JFFS2 patch steps".
3. Add a sheet for "the CTF solve path".

## Reference

- `docs/30`, `docs/appendix/*`
