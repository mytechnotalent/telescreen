# Walkthrough 71: Series Notes

**how this lab sits in the CTF series, and how to keep the family consistent**

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

## The Family

This lab is one entry in a **series of CTFs**, each a pair of repositories:

```
<task>-c-rpXXXX/          the defended device (firmware/lab)
CTF_<task>/               the compromised device (the challenge)
```

The series shares a **convention**, so a student who learns one can navigate any:

| convention | value |
| ---------- | ----- |
| the doc set | `-I` (Instructions), `-R` (Requirements/Grading), `-S` (Solution) |
| the artifact prefix | `CTF-XX-` (this challenge's id) |
| the repo pair | `<task>` + `CTF_<task>` |
| the disclaimer | on every document |
| the code standard | house C, 8-line bodies, `@brief` everywhere |
| the coverage gate | 100% line coverage |
| the legal theme | a fictional scenario, a real technique |

## What Is Shared vs Unique

| shared | unique to this entry |
| ------ | -------------------- |
| the doc format | the four-partition model |
| the code standard | the RP5 (application-class) |
| the gate scripts | Linux + JFFS2 (not bare metal) |
| the ethics framing | the crypto track (X25519/AEAD) |
| the finding format | the router + camera labs |

## Why This Entry Is Different

The reference entries are **microcontroller** CTFs (Pico 2, bare metal, `.uf2`). This entry is
an **application-class** CTF:

| | reference | this |
| - | --------- | ---- |
| silicon | Cortex-M33 | Cortex-A76 (RP5) |
| OS | bare metal | Linux |
| image | one `.bin`/`.uf2` | four partitions |
| filesystem | none | JFFS2 |
| crypto | on-device | OpenSSL + the ARMv8 extensions |

It teaches the **same discipline** with a **different technology class**, which is the point
of adding it to the series.

## The Wider Saga: this is the finale after OPERATION COLD IRON

TELESCREEN is not a standalone CTF. It is the **surveillance backbone** of the
same fictional world as OPERATION COLD IRON, and it comes **after** its ten acts.
The whole saga is **ARM**: the acts are bare-metal ARM (RP2350, Cortex-M33), and
TELESCREEN is application-class ARM (RP5, Cortex-A76) - the same architecture,
one level up.

- [OPERATION COLD IRON](https://github.com/mytechnotalent/cold-chain-monitor) -
  the Ministry's cold-chain edge, Act I of a planned ten-act saga (still in
  development).
- **TELESCREEN (this entry)** - the application-class ARM backbone that watches
  that edge.

The cold-chain README states the link directly: *"This act is one node of the
Ministry's industrial edge. TELESCREEN, the surveillance backbone that watches
it, comes after the ten."* TELESCREEN is that backbone, built on the Raspberry
Pi 5, and this series is its CTF.

## Keeping the Family Consistent

If you add another entry:

1. **Copy the doc format** (`-I/-R/-S` + PDFs + `DESIGN.md`).
2. **Copy the code standard** and the gate scripts.
3. **Use an artifact prefix** unique to the entry (`CTF-YY-...`).
4. **Put the disclaimer** on every document.
5. **State the limitations** truthfully (synthetic artifacts, etc.).

Consistency is what makes the series usable: a student learns the shape once.

## The Naming

| element | this entry |
| ------- | ---------- |
| task | `telescreen` |
| CTF repo | `CTF_telescreen` |
| artifact prefix | `CTF-XX-` |
| operation name | OPERATION TELESCREEN |

## The Reviewer's Note

Every entry must be **accurate and honest**: the formats right, the vectors published, the
limitations stated, the provenance labelled. A series is only as good as its weakest entry's
honesty.

## Exercises

1. Compare this entry's doc set with a reference entry's.
2. List what you would copy for a new entry.
3. Draft the `-I/-R/-S` skeleton for a new task.
4. Write the disclaimer block into a template.

## Reference

- `docs/walkthrough/41` (design), `docs/walkthrough/66` (index)
- the reference CTF repositories
