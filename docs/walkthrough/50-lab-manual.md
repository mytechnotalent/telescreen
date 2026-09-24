# Walkthrough 50: Instructor Lab Manual

**running the course, session by session**

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

## Who This Is For

Instructors running the TELESCREEN lab as a course. It gives the session plan, the prep,
and the common failure modes.

## Prep (before the first session)

1. **Bench per student/team:** RP5 + PSU + microSD + USB-UART adapter + webcam + a client
   device. (`PARTS.md`)
2. **Image the cards** with the four images (`docs/27`).
3. **Verify the toolchain** on the teaching machines: `python3`, `clang`/`gcc`,
   `openssl@3`, `binwalk`, `jefferson`, and the gate scripts.
4. **Run the gates** once to confirm the repo is green.
5. **Distribute the CTF repo** (separate) with the compromised images.

## Session Plan

| session | topic | volumes | lab |
| ------- | ----- | ------- | --- |
| 1 | the world, the device, the threat | `docs/01` | bring-up (Lab 0) |
| 2 | the four partitions | `docs/02`, `docs/03` | carve (Lab 1) |
| 3 | the boot chain | `docs/05`, `docs/06` | boot trace (Lab 2) |
| 4 | env + kernel + DT | `docs/07`-`docs/09` | kernel/DT (Lab 3) |
| 5 | the rootfs | `docs/11`, `docs/12` | extract + CRCs (Lab 4) |
| 6 | the app + sinks | `docs/13`-`docs/15` | backdoors (Lab 5) |
| 7 | crypto | `docs/17`-`docs/23` | break the KDF (Lab 6) |
| 8 | the router | `docs/24` | routing (Lab 7) |
| 9 | the camera | `docs/25` | streaming (Lab 8) |
| 10 | harden + rebuild | `docs/26`-`docs/28` | harden (Lab 9) |
| 11 | defense + ethics | `docs/28`, `docs/29` | playbook (Lab 10) |
| 12 | review | `docs/walkthrough/39` | the exam |

## Prep per Session

- Have the **volume printed** or open.
- Pre-flight the **lab** so no time is lost to a bad card or a dead adapter.
- Keep a **known-good golden card** to swap in when a student bricks one.

## Common Failure Modes (and fixes)

| symptom | cause | fix |
| ------- | ----- | --- |
| no console output | TX/RX not crossed | swap the two wires |
| board throttles | PSU under 5V/5A | use the official PSU |
| card will not boot | bad image write | re-image and verify (`docs/27`) |
| tool missing | environment not set up | run the prep checks |
| gate fails | a standard violation | read the auditor output |
| student overclaims | not labelling evidence | use the finding format |

## Teaching the Hard Points

| hard point | how to land it |
| ---------- | -------------- |
| Cortex-A vs Cortex-M vector table | decode `0xEA000515` by hand, twice |
| the container format | hex-dump it; find `gziphead` |
| `crc32_le` vs zlib | compute both on the same bytes |
| JFFS2 in-place patch | do the CRC order on the board |
| public-ID key | recover it live with `weak_decrypt.py` |
| secure boot | "what if U-Boot checked a signature?" |

## Grading

Use `CTF_telescreen/CTF-XX-R.md` for the formal rubric, `docs/walkthrough/39` for the exam,
and `docs/walkthrough/33` for the lab book.

## Safety and Ethics (cover in session 1)

- isolated bench only; local collector;
- own hardware or written authorisation;
- responsible disclosure (`docs/29`);
- the disclaimer on every document.

## A Minimal One-Day Version

If you have one day, keep: `docs/01`, `docs/02`, `docs/04`, `docs/05`, `docs/11`,
`docs/16`, `docs/25`. That is the shape, the boot, the filesystem, the bugs, and a view.

## Exercises (for instructors)

1. Run the 12-session plan once on your own bench.
2. Time each lab; adjust the plan.
3. Add a session for a topic your class needs.
4. Build a golden card and a recovery kit.

## Reference

- `PARTS.md`, `CTF_telescreen/CTF-XX-R.md`, `docs/walkthrough/33`, `docs/walkthrough/39`
