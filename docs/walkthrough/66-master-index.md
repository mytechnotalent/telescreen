# Walkthrough 66: Master Index

**every document in the project, and what it teaches**

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

## The Repositories

- **`telescreen`** - the defended lab (this repo).
- **`CTF_telescreen`** - the compromised challenge (separate repo).

## Root Documents

| file | contents |
| ---- | -------- |
| `README.md` | the manifest and the 30-volume plan |
| `PARTS.md` | the bill of materials |
| `NATION-STATE-REVIEW.md` | the adversarial accuracy audit |
| `LICENSE` | MIT |

## The Curriculum (`docs/01-30`)

| # | title | teaches |
| - | ----- | ------- |
| 01 | World and Threat | the setting, trust boundaries, defect classes |
| 02 | The Four Partitions | the model, source, disassembly, bytes |
| 03 | The Image Store | regions, integrity, the two CRCs |
| 04 | Carve and Verify | proving the layout from bytes |
| 05 | First-Stage Boot | Cortex-A vectors, the reset handler |
| 06 | U-Boot | identity, environment, the hand-off |
| 07 | Environment CRC | reading and editing `bootargs` |
| 08 | Kernel Container | the vendor format, inflate |
| 09 | Device Tree | the machine description |
| 10 | Command Line and Mount | `mtdparts`, mounting JFFS2 |
| 11 | JFFS2 Nodes | the log-structured filesystem |
| 12 | JFFS2 In-Place Patch | CRC-ordered editing |
| 13 | Userland Boot | init, network, the daemon |
| 14 | HTTP Dispatcher | the internal routes |
| 15 | `system()` Sites | templates and taint |
| 16 | Backdoor Catalogue | B1-B6 with paths |
| 17 | Weak KDF | why a public key is not encryption |
| 18 | AEAD Fundamentals | nonces, tags, the reuse catastrophe |
| 19 | AES-256-GCM on RP5 | the ARMv8 crypto extensions |
| 20 | XChaCha20-Poly1305 | the portable twin |
| 21 | X25519 and HKDF | agreement and derivation |
| 22 | Ed25519 Identity | signatures instead of RSA |
| 23 | Argon2id | password hashing |
| 24 | RP5 as a Router | AP, WAN, NAT, DHCP, DNS |
| 25 | RP5 as a Camera | UVC, RTSP, MJPEG |
| 26 | Building the Images | the four images from source |
| 27 | Flash and Verify | writing and proving |
| 28 | Blue-Team Detection | detecting the beacon and backdoors |
| 29 | Ethics and Law | authorisation, disclosure |
| 30 | Appendix | tables, magics, constants |

## The Module Volumes (`docs/modules/01-13`)

Each: header, full source, AArch64 disassembly, tests.

`crc, partition, env, container, jffs2, aead, beacon, kex, identity, collector, camera, teled,
main`.

## The Appendices (`docs/appendix/A-G`)

| file | contents |
| ---- | -------- |
| `A-functions.md` | the function catalogue |
| `B-aarch64-disasm.md` | the full AArch64 disassembly |
| `C-source-listing.md` | the full source |
| `D-scripts-listing.md` | the full tooling |
| `E-test-listing.md` | the full test suite |
| `F-constants.md` | constants and check values |
| `G-api-reference.md` | the per-function API |
| `H-annotated-disasm.md` | the disassembly per function |

## The Walkthroughs (`docs/walkthrough/01-66`)

| range | contents |
| ----- | -------- |
| 01-13 | line-by-line module walkthroughs |
| 14-15 | the tooling and the test suite |
| 16 | the CTF solve path (offensive) |
| 17 | the byte-level artifact walkthrough |
| 18 | cryptography internals |
| 19 | reverse-engineering labs |
| 20 | network protocols |
| 21 | hardware and bench |
| 22 | build and CI |
| 23 | data flows |
| 24 | protocol deep-dives |
| 25 | real-world comparison |
| 26 | forensics |
| 27 | diff analysis |
| 28 | writing a JFFS2 parser |
| 29 | offensive techniques |
| 30 | defensive hardening |
| 31 | Q&A |
| 32 | cheat sheets |
| 33 | the complete lab book |
| 34 | the daemon, end to end |
| 35 | porting the lab |
| 36 | methodology |
| 37 | filesystems compared |
| 38 | bootloaders compared |
| 39 | assessment and review |
| 40 | glossary |
| 41 | design decisions |
| 42 | tools compared |
| 43 | reference index |
| 44 | reading list |
| 45 | future work |
| 46 | advanced labs |
| 47 | the journey of one device |
| 48 | case studies |
| 49 | the security model |
| 50 | instructor lab manual |
| 51 | reference architecture |
| 52 | code review |
| 53 | embedded C for security |
| 54 | reading assembly |
| 55 | networking fundamentals |
| 56 | hardware interfaces |
| 57 | RE casebook |
| 58 | security engineering |
| 59 | deployment and operations |
| 60 | standards and compliance |
| 61 | web security for firmware |
| 62 | cryptanalysis of real-world mistakes |
| 63 | embedded Linux fundamentals |
| 64 | project FAQ |
| 65 | firmware extraction cookbook |
| 66 | this index |

## The Source Tree

```
src/        the 13 modules
include/    the headers
test/       the suite + harness
scripts/    the tools + the gates
CMakeLists.txt, .clang-format, .clangd
.github/    CI
```

## The Gates

```bash
python3 scripts/audit_c_standard.py      # 0 violations
python3 scripts/audit_python_standard.py # exit 0
python3 scripts/run_tests.py             # 98 checks, 0 failures
python3 scripts/check_coverage.py        # exit 0, 100.00%
```

## Reading Paths

**Beginner.** `README` -> `docs/01` -> `docs/02` -> `docs/04` -> `docs/walkthrough/33` ->
`docs/walkthrough/32` (cheat sheets).

**Developer.** `docs/modules/*` -> `docs/walkthrough/01-13` -> `docs/walkthrough/22` ->
`docs/walkthrough/52` (code review).

**Security.** `docs/16` -> `docs/17` -> `docs/walkthrough/16` (solve) ->
`docs/walkthrough/30` (harden) -> `docs/walkthrough/49` (model).

**Instructor.** `docs/walkthrough/50` (manual) -> `docs/walkthrough/33` (labs) ->
`docs/walkthrough/39` (assessment).

## Exercises

1. Read one volume from each section.
2. Pick a reading path and finish it.
3. Add your own notes as a volume.

## Reference

- every file in the repository
