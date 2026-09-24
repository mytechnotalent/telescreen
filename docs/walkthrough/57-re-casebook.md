# Walkthrough 57: Reverse-Engineering Casebook

**worked problems, from bytes to conclusions**

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

## How a Casebook Works

Each case is a **puzzle with a method**. Read the prompt, try it, then read the solution. The
technique is what you keep.

## Case 1: What Is This File?

**Prompt.** You have `unknown.bin`. Identify it.
**Method.** `file`, then `xxd -l 16`, then the magic table.
**Worked.**

```
$ xxd -l 16 unknown.bin
00000000: 85 19 03 20 0c 00 00 00  b1 b0 1e e4  ...
```

`85 19` is little-endian `0x1985` (JFFS2) and the type is `0x2003` (CLEANMARKER).
**Answer.** a JFFS2 image (`docs/11`). **Keep.** magic first, always.

## Case 2: Where Does It Jump?

**Prompt.** A boot blob begins `15 05 00 ea`. Where does it go?
**Method.** decode the A32 branch.
**Worked.** `0xEA000515`; offset `0x515` words; `0x08 + 0x515*4 = 0x145C`.
**Answer.** the reset handler is at `0x145C`. **Keep.** PC = address + 8.

## Case 3: Which CRC?

**Prompt.** A function computes a CRC and stores it. Which one?
**Method.** read the constant.
**Worked.**

```asm
mov  w8, #33568
movk w8, #60856, lsl #16     ; 0xEDB88320
```

`0xEDB88320` is the **reflected** CRC-32.
**Answer.** a reflected CRC-32. **Keep.** the constant names the algorithm.

## Case 4: Is It Packed?

**Prompt.** A function reads bytes and writes others, with no `bl` calls.
**Method.** look for a decompression signature (an inflate loop, a dictionary).
**Worked.** no calls, a byte loop, a small window -> a decompressor.
**Answer.** it is likely an inflate or a custom unpacker; name it, do not trace every byte.
**Keep.** recognise, do not trace.

## Case 5: Where Is `main`?

**Prompt.** A stripped Linux binary; find `main`.
**Method.** entry -> `_start` -> first arg of `__libc_start_main`.
**Worked.** the entry's `ldr r0, [pc, #12]` pool word is `0x10780`.
**Answer.** `main = 0x10780`. **Keep.** the ABI hands you `main`.

## Case 6: Is This Check Real?

**Prompt.** A verifier computes a tag and returns.
**Method.** does it **branch** on the comparison?
**Worked.**

```asm
bl   verify
mov  w0, #0        ; return 0 regardless of the result  <-- the bug
ret
```

**Answer.** the check is **fake** (no branch). **Keep.** a check without a branch is not a
check.

## Case 7: Which Key?

**Prompt.** A routine derives a key from a string.
**Method.** trace the string's source.
**Worked.** the string is the device's **public** UID read from a config.
**Answer.** the key is public; the crypto is theatre (`docs/17`). **Keep.** find the key
source.

## Case 8: How Do I Patch It?

**Prompt.** Change a config value in a JFFS2 image.
**Method.** the in-place INODE patch.
**Worked.** append a higher-version INODE; pad; fix `hdr_crc`, `data_crc`, `node_crc`.
**Answer.** the new node wins at mount. **Keep.** CRC order matters (`docs/12`).

## Case 9: Is It Calling Out?

**Prompt.** Determine whether the device beacons.
**Method.** capture and measure.
**Worked.** a periodic UDP flow to a fixed collector; near-zero jitter; high entropy.
**Answer.** yes; it is the beacon (`docs/28`). **Keep.** interval + entropy.

## Case 10: What Device Is This?

**Prompt.** Identify a device with no labels.
**Method.** provenance strings + the device tree.
**Worked.** the kernel banner names the board; the DT `compatible` names the SoC.
**Answer.** the model from the banner + DT (`docs/walkthrough/25`). **Keep.** banner + DT.

## The Method, Distilled

```
identify   -> magic
locate     -> decode the branch / the pool
classify   -> the constant
verify     -> does it branch?
attribute  -> the string + the DT
```

## Exercises

1. Work cases 1-10 on a device you own.
2. Add a case from a problem you hit.
3. For each case, write the one command that solves it.
4. Combine cases 3 and 6 into a "verify the crypto" checklist.

## Reference

- `docs/appendix/*`, `docs/walkthrough/19`
- every `docs/` volume
