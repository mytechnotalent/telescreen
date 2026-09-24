# Walkthrough 47: The Journey of One Device

**a single narrative that touches every concept in the course**

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

## The Idea

Every concept in this course is a step in one story: **a device, from the wall to a
finding to a fix**. This volume tells that story once, in order, with a pointer to the
volume that proves each step. Read it as a map.

## Chapter 1: The Device on the Wall

A TELESCREEN is bolted to a wall. It watches, it routes, and it whispers home. You decide
to understand it. **`docs/01`** (the setting and threat model).

## Chapter 2: The Flash

You open the case. There is an image store. You read it **three times** and hash it; the
reads match. **`docs/04`** (evidence discipline: get one clean image).

## Chapter 3: The Shape

The image is **four regions**. At offset 0 there is a vector table; further on, a CRC +
text; then a magic + `gziphead`; then a JFFS2 magic. You identify each **by magic**.
**`docs/02`** and **`docs/03`**.

## Chapter 4: The Boot

Power on. The first stage sets the CPU and DRAM; U-Boot reads its environment and loads the
kernel by offset. You watch it on the serial console. **`docs/05`**, **`docs/06`**,
**`docs/07`**, **`docs/08`**, **`docs/09`**, **`docs/10`**.

## Chapter 5: The Filesystem

The kernel mounts **JFFS2** as `/`. You learn the node model and the `crc32_le` trap, then
extract the tree. **`docs/11`**.

## Chapter 6: The Application

From `/sbin/init` the launcher starts the daemon. You read the composition:
beacon, dispatcher, router. **`docs/13`**, **`docs/14`**, **`docs/walkthrough/34`**.

## Chapter 7: The Sinks

You list every `system()` and every extraction. You find a route that runs `tar -C /` and a
config that is **sourced as root**. **`docs/15`**, **`docs/16`**.

## Chapter 8: The Crypto

You ask where the beacon key comes from. It comes from the **public UID**. The channel is
not encrypted; it is obfuscated. You recover the key. **`docs/17`**.

## Chapter 9: The Ledger

You write findings in a fixed format, and you label every sentence **measured / inferred /
claim**. **`docs/walkthrough/26`**.

## Chapter 10: The Fix

You replace the weak KDF with **X25519 + HKDF**, switch to **AES-256-GCM**, remove the
`system()` sites, validate the archive entries, and set real credentials.
**`docs/18`**-`docs/23`, **`docs/walkthrough/30`**.

## Chapter 11: The Rebuild

You rebuild the four images, verify them, and diff against the stock.
**`docs/26`**, **`docs/27`**, **`docs/walkthrough/27`**.

## Chapter 12: The Defense

You write a detection playbook and a hardening checklist. The device is now defensible.
**`docs/28`**, **`docs/29`**.

## The Map

```
1 wall       docs/01
2 flash      docs/04
3 shape      docs/02, docs/03
4 boot       docs/05-docs/10
5 filesystem docs/11
6 app        docs/13, docs/14, docs/walkthrough/34
7 sinks      docs/15, docs/16
8 crypto     docs/17
9 ledger     docs/walkthrough/26
10 fix       docs/18-docs/23, docs/walkthrough/30
11 rebuild   docs/26, docs/27, docs/walkthrough/27
12 defense   docs/28, docs/29
```

## The Back Cover

The device was a wall-mounted camera. The lesson is that it is a **stack of decisions**, and
every decision is inspectable. You did not break AES. You found where the key came from. You
did not exploit a zero-day. You followed a `system()` to its input. That is what real
security work looks like: **boring, methodical, and decisive.**

## Exercises

1. Tell this story for a device you own, with pointers to what you find.
2. For each chapter, name the one command that advances it.
3. Where a chapter does not apply, explain why.

## Reference

- every volume; `docs/walkthrough/36` (the methodology behind the story)
