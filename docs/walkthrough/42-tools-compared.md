# Walkthrough 42: Tools Compared

**when to reach for which tool**

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

## Bytes

| tool | best for | example |
| ---- | -------- | ------- |
| `xxd` | a hex dump with offsets | `xxd -l 32 boot.img` |
| `hexdump -C` | the same, different style | `hexdump -C boot.img` |
| `xxd -s N` | a dump at an offset | `xxd -s 0x30000 -l 64 kernel.img` |

**Rule:** the hex dump is the ground truth. When in doubt, look.

## Strings

| tool | best for |
| ---- | -------- |
| `strings` | printable runs |
| `strings -t x` | runs with file offsets |
| `strings -n N` | a minimum length |

**Pitfall:** `strings` on a compressed region gives nothing - decompress first.

## Disassembly

| tool | best for | note |
| ---- | -------- | ---- |
| `objdump` | quick, scriptable | needs the right `-m`/`-b` |
| `objdump -D -b binary -m arm` | raw ARM32 blob | bare-metal |
| `arm-none-eabi-objdump` | ARM32 ELF/objects | |
| Ghidra | decompile + xrefs | the workhorse |
| radare2 | CLI, scripting | `pdg` gives Ghidra's decompiler |
| `clang -S` | what the compiler does | for code you have |

**Rule:** for "what does it do", Ghidra. For "confirm this one function", `objdump`.

## ELF

| tool | best for |
| ---- | -------- |
| `readelf -h` | entry, type, machine |
| `readelf -s` | symbols (survivors in stripped) |
| `readelf -l` | segments, load addresses |
| `nm -D` | dynamic symbols |
| `ldd` | dynamic dependencies |

**Pitfall:** a "stripped" ELF still has `.dynsym`; you can often name every import.

## Filesystems

| tool | filesystem |
| ---- | ---------- |
| `jefferson` | JFFS2 |
| `unsquashfs` | squashfs |
| `ubireader` | UBI/UBIFS |
| `mount -o loop` | ext4 |
| `cramfsck` | cramfs |

`docs/walkthrough/37`.

## Firmware Identification

| tool | best for |
| ---- | -------- |
| `file` | a first guess |
| `binwalk` | embedded formats, offsets |
| `xxd` + the magic table | certainty |

**Rule:** `binwalk` suggests; the magic confirms.

## Crypto

| tool | best for |
| ---- | -------- |
| OpenSSL CLI | quick round trips |
| the lab's `aead`/`kex`/`identity` | the primitives, tested |
| RFC vectors | proving correctness |
| `weak_decrypt.py` | recovering a weak key |

## Network

| tool | best for |
| ---- | -------- |
| `tcpdump` | capture |
| Wireshark | visual analysis |
| `curl` | HTTP requests |
| VLC | RTSP |
| `nc` | raw connect |

## Build and Gates

| tool | best for |
| ---- | -------- |
| `cmake` | the product build |
| `run_tests.py` | the suite |
| `check_coverage.py` | the gate |
| `audit_*_standard.py` | the standards |

## The Decision Tree

```
bytes?        -> xxd
text?         -> strings
what runs?    -> Ghidra / objdump
where is it?  -> readelf
extract?      -> jefferson / unsquashfs / ubireader
prove crypto? -> RFC vectors
prove build?  -> the three gates
```

## Pitfalls Summary

| pitfall | tool | fix |
| ------- | ---- | --- |
| strings on compressed data | `strings` | decompress first |
| wrong `-m` | `objdump` | check the arch |
| trust a filename | `file` | check the magic |
| trust a single read | `dd` | three identical reads |
| zlib CRC for JFFS2 | Python | `crc32_le` |
| "it uses AES" | — | find the key source |

## Exercises

1. Dump the first 32 bytes of each image with `xxd` and one alternative tool.
2. Disassemble one function with `objdump` and again in Ghidra; compare.
3. Name the tool you would use for each of the six filesystem magics.
4. Build the decision tree for a new device.

## Reference

- `docs/walkthrough/14`, `docs/walkthrough/19`, `docs/walkthrough/37`
