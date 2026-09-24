# Walkthrough 64: Project FAQ

**questions about the repo itself**

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

## The Repo

**Q: What is the difference between `telescreen` and `CTF_telescreen`?**
`telescreen` is the **defended** lab (code, tests, tools, curriculum). `CTF_telescreen` is the
**compromised** challenge (images, docs, a self-contained code copy). Two repos, per the
reference format.

**Q: Why is the course so large?**
Because it is meant to be a **self-contained textbook**. Every concept has a volume, every
module has a walkthrough, every claim has a command.

**Q: Where do I start?**
`README.md`, then `docs/01`, then the labs (`docs/walkthrough/33`).

**Q: Where are the answers?**
`CTF_telescreen/CTF-XX-S.md` (instructor) and `docs/walkthrough/16` (the solve path).

## The Code

**Q: How many modules?**
Thirteen: `aead, partition, crc, env, jffs2, container, beacon, kex, identity, collector,
camera, teled, main`.

**Q: How is correctness proven?**
98 native checks, 45 Python cases, RFC vectors, and 100% line coverage, all enforced by the
three gates.

**Q: Why the eight-line rule?**
It forces decomposition. A body over eight lines is usually doing too much.

**Q: Why no blank lines in bodies?**
House convention, enforced by the audit.

**Q: Is there any heap?**
No. All state is static or on the stack.

**Q: Any `system()` in the code?**
No. The lab's app does not shell out. (The **CTF device's** app does - that is a defect.)

## The Crypto

**Q: Which AEAD?**
AES-256-GCM on the RP5; XChaCha20-Poly1305 as the portable twin. One API, two backends.

**Q: Which KDF?**
X25519 + HKDF-SHA256 for session keys; Argon2id for passwords.

**Q: Is anything "unhackable"?**
No. The point is to handle keys and nonces correctly, not to pick a magic cipher.

**Q: Why is there a deliberately weak KDF?**
As the anti-example. `beacon_weak_key` shows *why* key management matters (`docs/17`).

## The Hardware

**Q: Do I need a Pi 5?**
The lab targets it. A Pi 4 runs most of it.

**Q: Do I need a camera?**
No, unless you do the camera lab. A cheap USB webcam is the default.

**Q: Do I need the serial adapter?**
Yes. It is the only console.

## The Docs

**Q: How is the documentation organized?**
`docs/01-30` (the curriculum), `docs/modules/` (per-module), `docs/appendix/` (data),
`docs/walkthrough/` (line-by-line, labs, references).

**Q: Why a disclaimer on every file?**
The material is dual-use. Stating the terms up front is the responsible default.

**Q: Are the images real?**
They are **synthetic** - correct magics, offsets, and sizes, but not a dump of a real unit.
Stated plainly in `NATION-STATE-REVIEW.md`.

## The Method

**Q: What is the one discipline?**
Trust the bytes; prove every claim with a command; label what you infer.

**Q: How do I find `main` / the reset handler?**
`docs/WEEK12_CORTEX_A_RE.md`, `docs/05`.

**Q: How do I analyse a device I have never seen?**
`docs/walkthrough/25` (real-world comparison) and `docs/walkthrough/36` (methodology).

## Contributing

**Q: How do I add a module?**
House style, eight-line bodies, tests to 100%, the three gates green, a disclaimer on any
doc. Then wire it in (`docs/walkthrough/34`).

**Q: How do I add a lab?**
`docs/walkthrough/46` (advanced labs) has the template.

**Q: How do I extend the CTF?**
`docs/walkthrough/45` (future work) and Lab D in `docs/walkthrough/46`.

## The Meta

**Q: Why does this exist?**
To teach **how a comprehensive embedded-security project is built**: the model, the code, the
tests, the docs, and the honesty about limits.

**Q: What is the single best exercise?**
Do the CTF (`docs/walkthrough/16`) end to end, then harden and rebuild
(`docs/walkthrough/30`).

**Q: What next?**
`docs/walkthrough/44` (reading list) and `docs/walkthrough/45` (future work).

## Exercises

1. Answer any question you could not, with citations.
2. Add five questions a new student would ask.
3. Turn the FAQ into a one-page quick-start.

## Reference

- `README.md`, `NATION-STATE-REVIEW.md`, `PARTS.md`
- every `docs/` volume
