# Walkthrough 19: Reverse-Engineering Labs

**using the tools on the real binary, step by step**

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

## The Tools

| tool | role |
| ---- | ---- |
| `xxd` / `hexdump` | look at the bytes |
| `strings` | find text |
| `readelf` | ELF structure, entry, imports |
| `objdump` | disassemble |
| `clang -S` | compile our source to read the compiler's output |
| Ghidra | decompile and cross-reference |
| radare2 | CLI analysis |
| `binwalk` | identify embedded formats |
| `jefferson` | extract JFFS2 |

## Lab A: Look at the Bytes

**Goal.** Identify the four regions without any labels.

```bash
xxd -l 32 CTF-XX-full.img
xxd -s 0x020000 -l 32 CTF-XX-full.img
xxd -s 0x030000 -l 32 CTF-XX-full.img
xxd -s 0x1F0000 -l 32 CTF-XX-full.img
```

**What you should see.** Boot vectors, a CRC + text, the container magic + `gziphead`,
and the JFFS2 magic. Match each to `docs/30`.

**Deliverable.** The four-region table, filled from the bytes.

## Lab B: Decode the Reset Vector

**Goal.** Find the reset handler by hand.

```bash
python3 - <<'PY'
w = 0xEA000515
off = w & 0xFFFFFF
if off & 0x800000: off -= 0x1000000
print(hex(0x08 + off*4))
PY
```

**What you should see.** `0x145c`.

**Then:**

```bash
arm-none-eabi-objdump -D -b binary -m arm carved/boot.img \
    --start-address=0x145c --stop-address=0x1500
```

**Deliverable.** The first eight instructions of the reset handler, with a one-line note
each.

## Lab C: Compile and Read

**Goal.** See what the compiler does to our own C.

```bash
clang --target=aarch64-unknown-linux-gnu -O2 -S src/jffs2.c -Iinclude -o jffs2.s
sed -n '/jffs2_hdr_crc_valid/,/ret/p' jffs2.s
```

**What you should see.** The `bl crc32_le`, the `ldr w8, [x19, #8]`, and the `cset w0, eq`
(`docs/walkthrough/05`).

**Deliverable.** The asm of `jffs2_hdr_crc_valid`, annotated.

## Lab D: Read the ELF

**Goal.** Find the entry point and imports of the daemon.

```bash
readelf -h teled | grep Entry
readelf -s teled | grep -E '_start|system|openssl'
```

**What you should see.** The entry address (`_start`) and the imports (`system`,
OpenSSL functions). `system` in the import list is a red flag (`docs/15`).

**Deliverable.** The entry address and the list of imported dangerous functions.

## Lab E: Ghidra

**Goal.** Decompile a function and follow the call graph.

1. Ghidra, new project, import `teled`, processor `AArch64`, little-endian.
2. Auto-analyse.
3. Go to the entry point; follow the first argument into `__libc_start_main`; that is
   `main`.
4. Find `teled_beacon`; read its decompilation; confirm it matches
   `docs/walkthrough/09`.

**Deliverable.** The decompiled `main`, annotated with the C source line it matches.

## Lab F: Find the Dispatcher

**Goal.** Locate the HTTP route dispatcher by its strings.

```bash
strings -n4 teled | grep -E '\.cgi'
```

Then cross-reference the string in Ghidra (Ctrl+Shift+F for a string, then X for xrefs) to
the function that holds it. That function is the dispatcher (`docs/14`).

**Deliverable.** The dispatcher address and the list of routes it handles.

## Lab G: Find `system()`

**Goal.** Every place the daemon shells out.

```bash
objdump -d teled | grep -B1 '<system' | head
```

In Ghidra, list the callers of `system`. For each, read the arguments and classify
(`docs/15`).

**Deliverable.** A table: caller, command template, parameter, verdict.

## Lab H: Extract the JFFS2

**Goal.** Get the rootfs tree.

```bash
jefferson -d rootfs carved/rootfs.img
find rootfs -maxdepth 2 | head
```

**Deliverable.** The tree, with the application, config, and scripts noted.

## Lab I: Patch and Rebuild

**Goal.** Change a config value in JFFS2 and rebuild.

Follow `docs/12`: append a higher-version INODE, pad, fix the CRCs in order. Then
re-carve, re-verify, and hash.

**Deliverable.** The patched `rootfs.img`, its hash, and the three CRC values.

## The Method, in One Paragraph

You do not guess. You read the bytes (`xxd`), identify by magic (`docs/02`), disassemble
(`objdump` / Ghidra), follow the ABI (`_start -> main`), trace the dangerous calls
(`system`), and prove every claim with a command. That is the whole discipline, and it is
the same whether the target is this lab or a real camera.

## Exercises

1. Repeat Labs A-D on `CTF-XX-full_fixed.img` and diff.
2. In Ghidra, name every function you can from `strings` and `docs/walkthrough/`.
3. For one `system` caller, write the exploit path.
4. Produce a short report: findings, addresses, and fixes.

## Reference

- `docs/12`, `docs/14`, `docs/15`, `docs/walkthrough/16`
