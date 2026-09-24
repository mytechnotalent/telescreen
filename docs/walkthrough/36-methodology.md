# Walkthrough 36: Methodology

**the repeatable process, and the pitfalls**

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

## The Pipeline

```
physical/target -> get the image -> identify -> carve -> boot chain
   -> rootfs -> app -> sinks -> crypto -> findings -> fix -> rebuild -> verify
```

Every step in this course is one hop in this pipeline. This volume is the pipeline itself,
with the pitfalls.

## 1. Get the Image

**Do:** read the storage **three times**, require byte-identical results, hash them.
**Pitfall:** a single read is never trustworthy. A bad read makes every later finding
wrong.

```bash
shasum -a 256 image.bin      # record FIRST
```

## 2. Identify

**Do:** identify by **magic**, not by filename (`docs/02`).
**Pitfall:** trusting a label. A file named `kernel` that starts `85 19` is a JFFS2 image.

## 3. Carve

**Do:** carve by the **declared** offsets (`mtdparts`), not by guess.
**Pitfall:** "fixing" a deliberate pad. The layout is intentional.

## 4. Boot Chain

**Do:** find the reset vector, decode the branch, follow the hand-off.
**Pitfall:** expecting a `zImage`/`uImage` magic. The vendor container is
`[magic][len][gziphead][gzip]` (`docs/08`).

## 5. Rootfs

**Do:** extract with the right tool; use `crc32_le` for JFFS2.
**Pitfall:** using zlib's CRC. The node will not mount (`docs/walkthrough/01`).

## 6. The App

**Do:** find `main` via the ABI (`_start -> __libc_start_main`) and follow the call graph
(`docs/WEEK12_CORTEX_A_RE.md`).
**Pitfall:** searching for named functions that were **inlined**.

## 7. Sinks

**Do:** list every `system()`, `popen()`, extraction, and file write; trace the taint
(`docs/15`).
**Pitfall:** reading functions in isolation; the bug is in the **path**.

## 8. Crypto

**Do:** ask **where the key comes from**. If it is public, the crypto is theatre
(`docs/17`).
**Pitfall:** "it uses AES, so it is secure".

## 9. Findings

**Do:** write mechanism, path, impact, fix. Label measured/inferred
(`docs/walkthrough/26`).
**Pitfall:** overclaiming. A byte is a fact; a motive is not.

## 10. Fix and Rebuild

**Do:** apply the fix, rebuild, verify, diff (`docs/26`, `docs/27`,
`docs/walkthrough/27`).
**Pitfall:** shipping a diff that touches more than intended.

## 11. Verify

**Do:** require a pass. Hash, magic, CRC, test.
**Pitfall:** treating "the write returned" as "the write succeeded".

## The Tool, per Step

| step | tools |
| ---- | ----- |
| get image | `flashrom`, `dd`, a reader; 3x + hash |
| identify | `xxd`, `binwalk`, `file` |
| carve | `dd`, `carve.py` |
| boot | `objdump`, Ghidra, `python3` |
| rootfs | `jefferson`, a Python parser |
| app | Ghidra, radare2, `readelf` |
| sinks | `strings`, `grep`, Ghidra xrefs |
| crypto | `weak_decrypt.py`, Ghidra |
| rebuild | `build_images.py`, `mkfs.jffs2` |
| verify | `verify_telescreen.py`, the gates |

## The Pitfalls Table

| pitfall | cost | fix |
| ------- | ---- | --- |
| one read | wrong everything | three identical reads |
| trust a label | misidentify | check the magic |
| zlib CRC for JFFS2 | node will not mount | `crc32_le` |
| expect zImage | "corrupt" kernel | parse the container |
| named functions | miss inlined code | read the call graph |
| function viewing | miss the path | trace the taint |
| "it uses AES" | miss the flaw | find the key source |
| overclaim | lose credibility | label the evidence |

## The One Discipline

**Trust the bytes. Prove every claim with a command. Label what you infer.** Everything else
is detail.

## Exercises

1. Walk the pipeline on your own device and record each hop.
2. For each pitfall, describe how it would change your conclusion.
3. Write the pipeline as a checklist for a new target.

## Reference

- every `docs/` volume; `docs/walkthrough/25`, `docs/walkthrough/26`
