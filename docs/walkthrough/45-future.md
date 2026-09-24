# Walkthrough 45: Future Work

**where the lab can go next, and how to extend it**

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

## What the Lab Is, and Is Not

The lab is a **complete teaching model**: four partitions, a real boot chain, a JFFS2
rootfs, a set of backdoors, a crypto lesson, a router, a camera, and a CTF. It is **not** a
real device's firmware, and its images are **synthetic** (correct formats, not a real
dump). Future work closes that gap and extends the surface.

## 1. A Real Dump as the Artifact

The single highest-value extension: replace the synthetic images with a **real, consented
dump** of an owned device, then re-derive the constants. The labs would then analyse a
genuine vendor binary, with all its real imperfections.

## 2. More Defect Classes

The lab teaches six. Add:

| class | example |
| ----- | ------- |
| memory corruption | the RTSP parser overflow (as a live lab) |
| race condition | a TOCTOU in a config setter |
| deserialization | a length field trusted in a binary protocol |
| supply chain | a malicious update image (`docs/26`) |

A **memory-corruption lab** in particular would add the ret2libc/ROP skills
(`docs/walkthrough/29`).

## 3. A Live Attack Range

Turn the lab into a **range**: the device on an isolated network, a scoring server, and a
capture-the-flag arena. Students attack; the scoreboard records; the blue team detects
(`docs/28`).

## 4. More Targets

The same method applies to any device. A **porting lab** (`docs/walkthrough/35`) could ship
two devices and ask students to do the four-partition analysis on both.

## 5. A Blue-Team Track

Add a full defensive track:

- SIEM rules for the beacon and the backdoors;
- a hardened image as the gold standard;
- incident-response playbooks.

## 6. Formal Verification of the Crypto

The crypto is vector-tested but not **verified**. Future work could model the AEAD/KDF
usage in a tool (e.g. a protocol verifier) and prove the nonce discipline holds.

## 7. More Hardware

| addition | lab |
| -------- | --- |
| a second Wi-Fi radio | true dual-radio routing |
| a logic analyzer | watch SPI/UART |
| a flash programmer | the camera path (`docs/02`, `docs/27`) |
| an NVMe HAT | a faster image store |

## 8. More Protocols

Add ONVIF device management, RTMP, GB28181, and a full P2P handshake to the network labs
(`docs/walkthrough/24`).

## 9. Accessibility and Reproducibility

- a **Docker** image with the toolchain pinned;
- a **CI matrix** for Linux and macOS;
- a **one-command** setup for the images and the environment.

## 10. Documentation

The docs are already at teardown depth. Future work: **PDFs** of the walkthrough volumes,
a **searchable index**, and **captions/transcripts** for video versions.

## The Contribution Path

Anything you add should follow the house rules:

1. house C style, eight-line bodies, a `@brief` on everything;
2. tests to **100% line coverage**;
3. the three gates green;
4. the legal disclaimer on every document.

That is the whole standard, and it is what keeps the lab trustworthy.

## A Roadmap

```
near term   : Docker image; CI matrix; PDFs
mid term    : memory-corruption lab; live range
long term   : real consented dump; formal crypto verification
```

## Exercises

1. Pick one extension and write its design (`docs/walkthrough/41` is the template).
2. Add a Docker image for the toolchain.
3. Add a memory-corruption lab using the RTSP parser as a target.
4. Add a PDF target to the build for the walkthrough volumes.

## Reference

- `docs/walkthrough/41` (design decisions), `docs/walkthrough/35` (porting)
- `NATION-STATE-REVIEW.md` (the current limitations)
