# Walkthrough 31: Questions and Answers

**the questions students actually ask, answered**

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

## On the Hardware

**Q: Do I need a Raspberry Pi 5, or will a Pi 4 work?**
The lab targets the RP5 (BCM2712, Cortex-A76, ARMv8 crypto). A Pi 4 is ARMv8 too and will
run most of it; the crypto is hardware on both. Use the RP5 for fidelity.

**Q: Do I need a camera?**
No. The router, backdoor, and crypto labs run without one. A cheap USB webcam adds the
video lab (`docs/25`).

**Q: Do I need the serial adapter?**
Yes - it is the only console. You cannot watch the boot chain otherwise.

**Q: My board throttles. Help.**
Your PSU is under 5 V / 5 A. Measure it. Throttling looks exactly like a firmware bug.

## On the Software

**Q: Why four partitions and not a single image?**
Because that is how real devices are built (`docs/02`). It also makes the labs concrete:
carve, mount, patch, reflash.

**Q: Why is `main` excluded from coverage?**
It is the entry point; the OS exercises it, not the unit suite (`docs/walkthrough/10`).

**Q: Why is every function at most eight lines?**
A body over eight lines is usually doing too much. The rule forces decomposition, which is
why the modules are readable and testable (`docs/walkthrough/22`).

**Q: Why no blank lines in function bodies?**
House convention, enforced by the audit. It keeps a function visually compact.

## On the Crypto

**Q: Is AES-256-GCM unhackable?**
No - nothing is. It is confidential **and** authenticated, but only if the nonce never
repeats and the key stays secret (`docs/18`).

**Q: ChaCha or AES?**
On the RP5, AES-GCM (hardware, constant-time). On a portable/software target, XChaCha20
(no tables, 192-bit nonce). The repo has both (`docs/19`, `docs/20`).

**Q: Why not RSA?**
Slow, large, padding footguns, no forward secrecy. It survives only for X.509/TLS
compatibility (`docs/19`).

**Q: Why is a key from the UID worthless?**
The UID is public, so the key is public. The cipher does not matter (`docs/17`).

**Q: What is the one rule I must not break?**
Never reuse a nonce under one key (`docs/18`).

## On the CTF

**Q: Where is the compromised image?**
In the separate `CTF_telescreen/` repo: `CTF-XX-full.img`.

**Q: What is `CTF-XX-full_fixed.img`?**
The hardened reference. Diff against it to check your work (`docs/walkthrough/27`).

**Q: What does `XX` mean?**
It is this challenge's identifier. Other challenges in the series use other identifiers.

**Q: How do I know I found everything?**
Six defect classes (B1-B6). Match them all (`docs/16`).

## On the Method

**Q: How do I find `main` in a stripped binary?**
Follow the entry point: `_start` hands `main` to `__libc_start_main`'s first argument
(`docs/WEEK12_CORTEX_A_RE.md`).

**Q: How do I find the reset handler?**
Decode the branch at offset 0 (`0xEA000515 -> 0xB, 0x145C`) (`docs/05`).

**Q: How do I know which CRC to use?**
U-Boot env -> standard CRC32; JFFS2 -> `crc32_le`. The polynomial `0xEDB88320` in the
assembly tells you (`docs/walkthrough/01`).

**Q: How do I stay accurate?**
Label every sentence `[measured]`, `[inferred]`, or `[claim]` (`docs/walkthrough/26`).

## On Ethics

**Q: Can I test a device I found on my network?**
Only if you own it 100% or have written authorisation. Otherwise no (`docs/29`).

**Q: What do I do with a real bug?**
Disclose responsibly: document, report to the vendor, do not weaponise (`docs/29`).

**Q: Is this legal?**
On your own isolated hardware, yes. On anything else, that is your responsibility
(`docs/29` and the disclaimer).

## On the Repo

**Q: How do I run everything?**
```bash
python3 scripts/audit_c_standard.py
python3 scripts/audit_python_standard.py
python3 scripts/run_tests.py
python3 scripts/check_coverage.py
```

**Q: Where are the reference tables?**
`docs/30` (appendix) and `docs/appendix/A-G`.

**Q: Where is the deep disassembly?**
`docs/appendix/B-aarch64-disasm.md`.

**Q: Where is the full source?**
`docs/appendix/C-source-listing.md`, and every module volume in `docs/modules/`.

## Exercises

1. Add three questions of your own and answer them with citations.
2. For each "why" answer, name the volume that proves it.
3. Pick the two answers most useful to a beginner and expand them.

## Reference

- every volume in `docs/` and `docs/walkthrough/`
