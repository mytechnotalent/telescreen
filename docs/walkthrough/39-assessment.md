# Walkthrough 39: Assessment and Review

**how the course is graded, and how to review your own work**

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

## The Course Assessment

| section | weight | measures |
| ------- | ------ | -------- |
| carve/layout | 10 | byte identification |
| boot chain | 15 | assembly reading |
| kernel/DT | 15 | format parsing |
| rootfs/CRCs | 15 | the CRC discipline |
| backdoors | 20 | sink analysis |
| crypto | 15 | key management |
| router/camera | 10 | systems integration |
| harden/rebuild | 15 | the fix and the diff |
| playbook | +10 | detection |

## The Exam (closed book)

Pick one. Each is designed to be answerable only if you did the labs.

**Q1 (boot).** A 32-bit ARM image begins `15 05 00 ea`. Give the reset target and the eight
exception offsets. *(10)*

**Q2 (carve).** Decode `mtdparts=sfc:128K(boot),64K(bootargs),1792K(kernel),14400K(rootfs)`
into a table of offsets and sizes. *(10)*

**Q3 (kernel).** Why does searching for `0x016F2818` fail on a container kernel, and what
do you search for instead? *(10)*

**Q4 (CRCs).** State the seed and final inversion for `crc32_le` and for the U-Boot
environment CRC, and name one consequence of confusing them. *(10)*

**Q5 (JFFS2).** Give the exact field order of a JFFS2 INODE header and the offset of the
data. *(10)*

**Q6 (sinks).** Given `sprintf(buf, "ping %s", host); system(buf);`, describe the exploit
and the fix. *(15)*

**Q7 (crypto).** A device encrypts beacons with AES-256-GCM using a key derived from its
public serial. Is the channel confidential? Justify. *(15)*

**Q8 (rebuild).** Describe the in-place JFFS2 patch, including the CRC order. *(10)*

**Q9 (defense).** Name the single highest-value control that would close the image-modify
class, and why. *(10)*

**Q10 (ethics).** You find a real bug in a camera you do not own but can reach on a shared
network. What do you do? *(10)*

**Total: 110.**

## The Review Checklist (self-assessment)

```
[ ] I can carve from a raw image by offsets.
[ ] I can identify the four regions by magic.
[ ] I can decode an ARM branch by hand.
[ ] I can read a U-Boot environment and fix its CRC.
[ ] I can parse the container and inflate the kernel.
[ ] I can extract JFFS2 and verify a node's crc32_le.
[ ] I can patch a JFFS2 file in place.
[ ] I can find system() sites and trace the taint.
[ ] I can explain why a public-ID key is not encryption.
[ ] I can build the split AEAD API and pass the RFC vectors.
[ ] I can bring up the RP5 as a router and prove DHCP.
[ ] I can harden, rebuild, and diff.
[ ] I label measured vs inferred.
```

## Grading Your Own Report

| criterion | full | partial | none |
| --------- | ---- | ------- | ---- |
| partition table correct | 10 | offsets only 6 | 0 |
| boot chain traced | 15 | partial 9 | 0 |
| backdoors with paths | 20 | list only 10 | 0 |
| crypto break with proof | 15 | formula only 8 | 0 |
| hardened image built | 15 | partial 8 | 0 |
| evidence labelled | throughout | sometimes | never |

## The Standard of Proof

Every claim needs:

1. a **command** that produces it;
2. the **output**;
3. a **label** (measured/inferred/claim).

A report that says "the device has a backdoor" is worthless. A report that says
`GET /restore.cgi` reaches `system("tar -xvzf <path> -C /")` at offset `0x…`, and a crafted
archive writes `/etc/init.d/S99x`, is a finding.

## Exercises

1. Answer all ten exam questions from memory, then verify.
2. Complete the review checklist honestly.
3. Grade one of your own reports against the rubric.
4. Rewrite one weak claim as a measured one.

## Reference

- every `docs/` and `docs/walkthrough/` volume
- `CTF_telescreen/CTF-XX-R.md`
