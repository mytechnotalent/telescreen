# Walkthrough 73: Acknowledgments and Provenance

**where the ideas and the material came from**

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

## The Tools We Stand On

This lab is built on the work of others:

| tool | what it gives |
| ---- | ------------- |
| **Ghidra** (NSA) | decompilation and cross-references |
| **radare2 / r2ghidra** | CLI analysis |
| **binutils** | `objdump`, `readelf` |
| **binwalk** | format identification |
| **jefferson** | JFFS2 extraction |
| **OpenSSL** | the crypto primitives |
| **mtd-utils** | `mkfs.jffs2`, the MTD tools |
| **pandoc / weasyprint** | the PDFs |
| **dtc** | the device tree |
| **Docker** | the ARM environment |

The C standard (`c-standard`) and the coverage gate (`check_coverage`) follow the reference
course's convention, shared across the series.

## The References

The crypto is pinned to standards, not to tribal knowledge:

```
RFC 7748  X25519
RFC 5869  HKDF
RFC 8032  Ed25519
RFC 8439  ChaCha20-Poly1305
RFC 9106  Argon2
SP 800-38D  AES-GCM
```

The methodology (sink/taint, evidence labelling) follows the published security-engineering
literature (`docs/walkthrough/44`).

## The Credits

- **The builders** of the devices this class of product runs on - the craft is the lesson.
- **The open-source community** whose tools make this work possible.
- **The students** whose questions shaped these volumes.

## The Provenance of the Lab

The TELESCREEN is fiction. The **formats, the flaws, and the method are real**, drawn from
the class of cheap, unsigned, backdoored cameras and routers that ship by the million. The
lab's images are **synthetic** - the correct magics, offsets, and sizes, but not a dump of a
real unit. That limitation is stated in `NATION-STATE-REVIEW.md`.

## The License

MIT (see `LICENSE`). Use it, teach it, extend it - with the disclaimer intact, and only on
hardware you own or are authorised to test.

## Exercises

1. List the tools you used and one thing each gave you.
2. Trace one claim in the docs to its standard.
3. Add an acknowledgment for a tool you relied on.

## Reference

- `LICENSE`, `docs/walkthrough/44`, `NATION-STATE-REVIEW.md`
