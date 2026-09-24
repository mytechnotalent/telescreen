# Walkthrough 43: Reference Index

**every hash, check value, address, and command in one place**

***
**LEGAL DISCLAIMER:** The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with. **IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
***

## Artifact Hashes

```
4193d6a9e9a29848813056f05356c17e4a4e64f8206148c86fd3b0d50a5e69fd  CTF-XX-boot.img
e935f998fd6289615d88d8d2c2d4d3aada0e04ad6dd7592c27a853431fec4d7e  CTF-XX-env.img
6451bc5b4127ff4e8dd144c23618a32b94183a53d8a6a3600902d3a8c0c2d199  CTF-XX-full_fixed.img
037cc979a58f04f3285132a7b745ca859b1604aeb80c9ef62eb5377e48bd18ec  CTF-XX-full.img
8a5fb5725420d79ae4e21e39e27c161b7df88dbbdbd2b061d859dd889c8b0230  CTF-XX-kernel.img
38a8684afb5f8bb8b717b2ab333dc729054d36192aa6541be6e9fed6f19cd10e  CTF-XX-rootfs.img
```

## Check Values

```
crc32_uboot("123456789") = 0xCBF43926
crc16_ccitt("123456789") = 0x29B1
crc32_le(seed, "", 0)    = seed
X25519  (RFC 7748) = 4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742
HKDF    (RFC 5869) = 3cb25f25faacd57a90434f64d0362f2a2d2d0a90cf1a5a4c5db02d56ecc4c5bf34007208d5b887185865
Ed25519 (RFC 8032) pub = d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a
Ed25519 (RFC 8032) sig = e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b
```

## Constants

```
PART_IMAGE_SIZE      0x1000000
PART_BOOT_OFF        0x000000   PART_BOOT_SIZE   0x020000
PART_ENV_OFF         0x020000   PART_ENV_SIZE    0x010000
PART_KERNEL_OFF      0x030000   PART_KERNEL_SIZE 0x1C0000
PART_ROOTFS_OFF      0x1F0000   PART_ROOTFS_SIZE 0xE10000
PART_BOOT_MAGIC      0xEA000515
PART_CONTAINER_MAGIC 0x001B8421
PART_JFFS2_MAGIC     0x1985
AEAD_AES_NONCE_SIZE     12
AEAD_XCHACHA_NONCE_SIZE 24
AEAD_PAYLOAD_SIZE       48
AEAD_TAG_SIZE           16
```

## Addresses (from the walkthroughs)

```
boot reset vector    0xEA000515 -> B 0x145C
kernel +8            "gziphead"
JFFS2 data (INODE)   node + 68
```

## Every Command

```bash
# carve / build / verify
dd if=full.img of=boot.img bs=1 count=$((0x20000))
dd if=full.img of=env.img bs=1 skip=$((0x20000)) count=$((0x10000))
dd if=full.img of=kernel.img bs=1 skip=$((0x30000)) count=$((0x1C0000))
dd if=full.img of=rootfs.img bs=1 skip=$((0x1F0000)) count=$((0xE10000))
python3 scripts/carve.py --image CTF-XX-full.img --out carved/
python3 scripts/verify_telescreen.py --image CTF-XX-full.img --sha256 <hash>
python3 scripts/build_images.py --uboot u-boot.bin --kernel Image --rootfs rootfs/ --out out/

# standards + tests
python3 scripts/audit_c_standard.py
python3 scripts/audit_python_standard.py
python3 scripts/run_tests.py
python3 scripts/check_coverage.py

# reverse engineering
xxd -l 32 X ; strings -n6 X | head ; readelf -h X | grep Entry
arm-none-eabi-objdump -D -b binary -m arm boot.img | head
clang --target=aarch64-unknown-linux-gnu -O2 -S src/jffs2.c -Iinclude -o jffs2.s
r2 -a arm -b 32 -q -c 'aaa; s entry0; pd 20' X
jefferson -d rootfs rootfs.img

# network
printf 'OPTIONS rtsp://127.0.0.1/ RTSP/1.0\r\nCSeq: 1\r\n\r\n' | nc 127.0.0.1 554
curl -v http://192.168.50.1/param.cgi?cmd=getlanguage
sudo tcpdump -i eth0 -n -ttt 'not arp' | head

# router
hostapd /etc/hostapd.conf -B
dnsmasq --interface=wlan0 --dhcp-range=192.168.50.10,192.168.50.200,12h
nft add rule ip nat postrouting oifname "eth0" masquerade
udhcpc -i eth0

# console
screen /dev/tty.usbserial-XXXX 115200
```

## The Volume Map

```
docs/01-30        the curriculum
docs/modules/     per-module (source + disasm + tests)
docs/appendix/    A functions, B disasm, C source, D tools, E tests, F constants
docs/walkthrough/ line-by-line, labs, references
```

## Exercises

1. Reproduce every hash in this volume and confirm they match.
2. Run one command from each section.
3. Add this device's numbers to your own notes.
