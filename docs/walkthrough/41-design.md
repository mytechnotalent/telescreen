# Walkthrough 41: Design Decisions

**why the lab is built the way it is**

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

## Why Explain the Design

A course you cannot modify is a course you cannot teach from. Every choice here has a
reason, and knowing the reasons lets you change the lab for your own class. This volume is
the rationale for the structure.

## Decision 1: Two Repositories

**Decision.** The lab (`telescreen`) and the challenge (`CTF_telescreen`) are **separate
repositories**.

**Why.** The lab is the *defended* device; the CTF is the *compromised* one. Keeping them
apart means a student can study the defence without seeing the flag, and the instructor can
ship the CTF without shipping the answers in the same tree. It also mirrors the reference
series.

**Consequence.** The CTF carries a **self-contained copy** of the lab code so it builds
alone.

## Decision 2: The Four-Partition Model

**Decision.** The images use four fixed regions, identical to a real camera's flash.

**Why.** It is the single most transferable fact about embedded firmware, and it makes every
lab concrete (carve, mount, patch, reflash).

**Consequence.** The RP5 has no SPI-NOR, so the four regions live on microSD/NVMe - the
*format* is preserved, the *silicon* differs.

## Decision 3: Real Linux, Not Bare Metal

**Decision.** The lab boots a real U-Boot -> Linux -> rootfs chain, not bare metal.

**Why.** Real routers and cameras run Linux. Bare metal would be *less* faithful, and it
would lose the JFFS2, the container, and the whole userland, which are where the bugs are.

## Decision 4: A Split AEAD API

**Decision.** One API, two backends (AES-256-GCM and XChaCha20-Poly1305).

**Why.** It teaches *how to choose*: hardware vs software, nonce size, side-channel model.
One backend would hide the decision.

## Decision 5: The Eight-Line Rule

**Decision.** Every function body is at most eight executable lines; no blank lines inside
bodies; a `@brief` on everything.

**Why.** A body over eight lines is usually doing two things. The rule forces decomposition,
which is why a 634-line crypto module is readable and reaches 100% coverage. The gate makes
it automatic.

## Decision 6: 100% Line Coverage

**Decision.** Every owned file must reach 100% line coverage.

**Why.** An uncovered line is untested - and in a security module, an untested branch is a
potential vulnerability. The gate found two dead lines during development.

## Decision 7: Standard Vectors, Not Round-Trips

**Decision.** Crypto is pinned to **published vectors** (RFC 7748/5869/8032), not to
"seal then open".

**Why.** A round-trip passes with a wrong key; only a standard vector catches a wrong
implementation. The RFC 5869 vector caught the `hkdf_block` bug.

## Decision 8: A Weak KDF, on Purpose

**Decision.** `beacon_weak_key` derives a key from a public UID, and it stays in the repo.

**Why.** The lesson is *why key management matters*, not "AES is strong". Keeping the flaw
next to the fix (`docs/17` vs `docs/21`) is the whole pedagogy.

## Decision 9: A Legal Disclaimer on Every Document

**Decision.** Every markdown file carries the disclaimer.

**Why.** The material is dual-use. Stating the terms up front - and repeating them - is the
responsible default, matching the reference course.

## Decision 10: Synthetic Artifacts

**Decision.** The CTF images carry the correct magics, offsets, and sizes, but are not a
dump of a real unit.

**Why.** Shipping a real device's secrets is not appropriate, and the *formats* are what the
labs teach. The limitation is stated explicitly (`NATION-STATE-REVIEW.md`).

## The Structure, in One Picture

```
telescreen/                 the defended device
├── src/ include/           the modules
├── test/                   the suite (98 checks)
├── scripts/                the tools + the gates
└── docs/                   the curriculum (this)

CTF_telescreen/             the compromised device
├── CTF-XX-*.img            the artifacts
├── CTF-XX-I/R/S            the student/instructor docs
├── DESIGN.md               the build spine
└── src/ include/           a self-contained copy
```

## What You Would Change for Your Class

| if you want | change |
| ----------- | ------ |
| a different board | port the bootloader/kernel (`docs/walkthrough/35`) |
| a different fs | change `mkfs.<fs>` and the parser (`docs/walkthrough/37`) |
| a different cipher | add a backend to `aead.c` |
| no crypto | delete the `beacon` volume and the CTF's B6 |
| a shorter course | keep `docs/02`, `docs/04`, `docs/16` |

## Exercises

1. For each decision, state the alternative and why it is worse.
2. Propose one design change and its consequences.
3. Write a one-page rationale for your own lab.

## Reference

- `README.md`, `NATION-STATE-REVIEW.md`, `PARTS.md`
