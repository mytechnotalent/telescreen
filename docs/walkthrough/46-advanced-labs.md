# Walkthrough 46: Advanced Labs

**extension labs for students who finish early**

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

## Why Advanced Labs

The core labs teach the model. These labs go deeper: add a feature, break a defence, build
a tool, and verify formally. Each is a multi-hour project.

## Advanced Lab A: Add a Hardened Feature

**Goal.** Add **authenticated config updates** to the daemon.

1. New module `config.h/.c`: read a config only if an Ed25519 signature over it verifies
   (`docs/22`).
2. Wire it into `teled_init` so the daemon refuses an unsigned config.
3. Tests to 100% coverage.
4. Pass the three gates.

**Deliverable.** The module, its tests, and a demo that an unsigned config is rejected.

## Advanced Lab B: Break Secure Boot (on Paper)

**Goal.** Understand why secure boot closes the class.

1. Describe how U-Boot would verify an Ed25519 signature over the kernel.
2. Then describe the **attack surface that remains**: key storage, the ROM, rollback.
3. Write the residual-risk analysis.

**Deliverable.** A two-page analysis of secure boot and its limits.

## Advanced Lab C: Build a Diff Tool

**Goal.** A reusable firmware-diff tool.

1. Take two images; align by the four regions.
2. Report runs per region, decoding printable runs as text.
3. Handle different sizes (added/removed regions).

**Deliverable.** `scripts/fwdiff.py` plus tests; pass the Python standard.

## Advanced Lab D: Extend the CTF

**Goal.** Add a seventh defect.

1. Choose a class (`docs/walkthrough/45`): a race, a deserialization, or a memory bug.
2. Implement it in the lab.
3. Add the finding to the CTF's `-R`/`-S` and a fixed variant.
4. Update the rubric.

**Deliverable.** A new defect with a graded fix and a hardened image.

## Advanced Lab E: A Live Range

**Goal.** Turn the lab into a scored range.

1. Deploy the device on an isolated network.
2. Stand up a scoring server (flag check).
3. Add a blue-team scoreboard (detections).

**Deliverable.** A working range with a run-book.

## Advanced Lab F: Formal Nonce Analysis

**Goal.** Prove the nonce discipline.

1. Model the beacon's nonce generation (sequence -> nonce).
2. Show the sequence is strictly monotonic (`teled_beacon`).
3. Argue that no two beacons ever share a nonce under one key.
4. State the assumption (one key per boot).

**Deliverable.** A short proof plus the code path that guarantees it.

## Advanced Lab G: Port the Lab

**Goal.** Port the model to a second board (`docs/walkthrough/35`).

1. Build U-Boot and a kernel for the target.
2. Update the magics if the target differs.
3. Build, carve, verify, boot.

**Deliverable.** A second working lab with a port report.

## Advanced Lab H: Write a Fuzzer

**Goal.** Fuzz a lab parser.

1. Take `part_identify` / `env_find` / the JFFS2 parser.
2. Build a harness that feeds random and mutated inputs.
3. Run under ASAN/UBSAN; report crashes.
4. Fix any found.

**Deliverable.** The harness, a run log, and any fixes.

## The Advanced Rubric

| lab | points | emphasis |
| --- | ------ | -------- |
| A feature | 30 | correctness + coverage |
| B secure boot | 15 | analysis |
| C diff tool | 25 | a real tool |
| D extend CTF | 30 | the full cycle |
| E live range | 30 | integration |
| F nonce proof | 20 | rigour |
| G port | 30 | the model transfers |
| H fuzzer | 25 | find a real bug |

## The Standard

Every advanced lab is held to the same bar: **house style, 100% coverage, the three
gates, the disclaimer.** No exceptions. That bar is the course's real lesson.

## Exercises

1. Do Lab A end to end and pass the gates.
2. Do Lab F and write the proof.
3. Do Lab H and report at least one crash (or prove none).
4. Combine B and F into a secure-boot + nonce-discipline white paper.

## Reference

- `docs/walkthrough/45` (extensions), `docs/walkthrough/41` (design)
- every `docs/` volume
