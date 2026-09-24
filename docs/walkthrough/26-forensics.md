# Walkthrough 26: Forensics

**provenance, timeline, attribution, and evidence handling**

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

## Why Forensics

Reverse engineering answers "what does it do". Forensics answers "**where did it come from,
when, and who touched it**". The same bytes carry both. This volume teaches the forensic
lens: build provenance, timeline, and the discipline of evidence handling.

## 1. Evidence Handling

For a real investigation, the rules are strict:

1. **Acquire once, hash immediately.** Record the SHA-256 of the image before you touch it.
2. **Work on copies.** Never analyse the original.
3. **Log every command.** Reproducibility is what makes a finding defensible.
4. **Separate `[measured]` from `[inferred]`.** A byte is measured; a motive is inferred.

```bash
shasum -a 256 CTF-XX-full.img     # record this FIRST
cp CTF-XX-full.img work.img       # analyse the copy
```

## 2. Build Provenance

Embedded binaries often carry their **build environment** as strings - paths, user names,
compiler versions. In the PROJECT camera teardown these revealed a vendor SDK path and a
build host. On the TELESCREEN:

```bash
strings -n6 teled | grep -iE 'gcc|[0-9]+\.[0-9]+\.[0-9]+|/home/|/build/'
```

| artifact | what it reveals |
| -------- | --------------- |
| compiler version | which toolchain |
| source path | the vendor's directory layout |
| SDK name | the platform generation |
| build timestamp | when it was built |

Even a lab binary carries what a real one would. Practise extracting it.

## 3. The Timeline

Strings and headers give a timeline:

| source | timestamp |
| ------ | --------- |
| kernel banner (`#1 Sat ...`) | the kernel build date |
| container gzip MTIME | when the kernel was gzipped |
| `U-Boot ... (date)` | the bootloader build date |

Order the artifacts by date to see a **release history**. A newer kernel with an older
bootloader is a maintenance drop, not a fresh platform. This is a **forensic
inference** - label it.

## 4. Config Forensic Markers

Config files reveal intent and defaults:

| marker | meaning |
| ------ | ------- |
| default credentials | a shipped default |
| empty password | an auth bypass |
| hardcoded IP/domain | a vendor endpoint |
| debug flags | a development leftover |

```bash
grep -rniE 'passwd|password|admin|debug|key' rootfs/etc rootfs/mnt 2>/dev/null | head
```

## 5. Attribution (Careful)

Attribution is the hardest part and the easiest to get **wrong**:

- A **magic byte** is a fact.
- "This magic means vendor X shipped it" is an **inference**.
- "Vendor X **intends** to spy" is a **claim you cannot support from bytes**.

Label everything. In the lab, when you find `beacon_weak_key`, the correct statement is:
*"the beacon key is derived from the public UID"* (measured), not *"the vendor wants to be
read"* (unsupported).

## 6. Indicators of Compromise (Checklist)

| indicator | means |
| --------- | ----- |
| a config file written at boot | a possible operator artifact |
| an unexpected listener | a possible backdoor |
| a downloaded payload | a possible stager |
| a marker file (e.g. `pwned`) | a test/operator artifact |
| a key derived from public data | weak crypto |

## 7. Rebuild the Story

Given the artifacts, write the story in order:

```
1. What is it?           (docs/02, docs/08)
2. When was it built?    (this volume, §3)
3. Who built it?         (this volume, §2)  [label inference]
4. What does it do?      (docs/13-docs/15)
5. How does it fail?     (docs/16, docs/17)
6. What is default?      (this volume, §4)
```

## 8. The Forensic Discipline

```
MEASURED : a byte, a string, a hash, a command output
INFERRED : a conclusion drawn from measurements
CLAIM    : an assertion you cannot yet support
```

Write the label next to every sentence in a report. It is the difference between a
finding and a guess.

## Exercises

1. Hash the image and record the value before analysis.
2. Extract the build provenance strings from `teled`.
3. Order the timestamps into a timeline.
4. List the config markers and label each measured/inferred.
5. Write a one-page attribution statement that stays within the evidence.

## Reference

- `docs/02`, `docs/08`, `docs/16`, `docs/17`, `docs/29`
- The PROJECT camera teardown for a full forensic example
