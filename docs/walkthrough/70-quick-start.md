# Walkthrough 70: Quick Start

**the whole course in one page**

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

## 1. What This Is

A Raspberry Pi 5 lab that recreates a real IP camera/router's **four-partition** firmware:
U-Boot, environment, kernel container, JFFS2 rootfs. You carve it, reverse it, find the
backdoors, break the weak crypto, and rebuild it hardened. A separate CTF challenges you.

## 2. Get Running in Ten Minutes

```bash
git clone <telescreen>          # this repo
git clone <CTF_telescreen>      # the challenge
cd telescreen
python3 scripts/audit_c_standard.py
python3 scripts/run_tests.py           # 98 checks, 0 failures
python3 scripts/check_coverage.py      # 100.00%
```

## 3. Carve the CTF Image

```bash
cd ../CTF_telescreen
python3 ../telescreen/scripts/carve.py --image CTF-XX-full.img --out carved/
python3 ../telescreen/scripts/verify_telescreen.py --image CTF-XX-full.img
xxd -l 32 carved/boot.img
```

## 4. The Five Facts

```
partition 0  boot      0x000000  128 KiB   magic 15 05 00 ea
partition 1  bootargs  0x020000   64 KiB   CRC32 + key=value
partition 2  kernel    0x030000 1792 KiB   magic 21 84 1b 00 + gziphead
partition 3  rootfs    0x1F0000 14400 KiB  magic 85 19 03 20
reset vector 0xEA000515 -> B 0x145C
```

## 5. The Six Backdoors

| # | defect |
| - | ------ |
| B1 | config sourced as root |
| B2 | `system()` from a request |
| B3 | `tar -C /` on an upload |
| B4 | empty/default credentials |
| B5 | debug root shell |
| B6 | key from the public UID |

## 6. The Crypto Rule

**Never reuse a nonce; never derive a key from public data.** Everything else follows.

## 7. The Command Cheat Sheet

```bash
xxd -l 32 X                       # look at bytes
strings -n6 X | head              # find text
readelf -h X | grep Entry         # entry point
objdump -d X | head               # disassemble
jefferson -d rootfs rootfs.img    # extract JFFS2
python3 scripts/run_tests.py      # the suite
```

## 8. The Reading Path

```
README -> docs/01 -> docs/02 -> docs/04 -> docs/walkthrough/33 (labs)
```

## 9. The Safety Rule

Own hardware or written authorisation. Isolated network. Local collector. Label every
finding measured/inferred. **Never accuse a vendor without a pristine-image proof.**

## 10. Where to Go Next

| goal | read |
| ---- | ---- |
| understand the boot | `docs/05`, `docs/06` |
| understand JFFS2 | `docs/11`, `docs/12` |
| find the bugs | `docs/14`-`docs/16` |
| the crypto | `docs/17`-`docs/23` |
| the CTF solve | `docs/walkthrough/16` |
| harden | `docs/walkthrough/30` |
| the method | `docs/walkthrough/36` |
| everything | `docs/walkthrough/66` (master index) |

## Exercises

1. Complete steps 2 and 3 above.
2. Name the six backdoors from memory.
3. Carve and identify all four regions.
4. Pick one reading path and begin it.

## Reference

- `README.md`, `docs/walkthrough/66`, `docs/walkthrough/32`
