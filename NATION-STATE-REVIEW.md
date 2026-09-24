# TELESCREEN RP5 CTF - Nation-State Accuracy Review

**An adversarial, evidence-based audit of the entire project. Every claim is either
verified by a re-runnable command or explicitly labelled as a limitation.**

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

## 1. Scope and Method

This review treats the project as hostile-to-itself: every module, constant, test
vector, document, and artifact is independently checked. The method is:

1. **Re-run every gate** (`audit_c_standard`, `audit_python_standard`, `run_tests`,
   `check_coverage`).
2. **Re-verify every constant** against the artifact and the code.
3. **Re-verify every cryptographic claim** against a published standard vector.
4. **Re-verify every artifact** by hash and magic.
5. **Read the documents adversarially** for overclaims and label anything unproven.

## 2. Gate Results (all re-run for this review)

| gate | command | result |
|---|---|---|
| C standard | `python3 scripts/audit_c_standard.py` | **0 violations** |
| Python standard | `python3 scripts/audit_python_standard.py` | **exit 0** |
| Native tests | `python3 scripts/run_tests.py` | **92 checks, 0 failures** |
| Coverage | `python3 scripts/check_coverage.py` | **exit 0 - 100.00% line coverage** |

## 3. Module-by-Module Audit

| module | role | lines | line cover | verification | honest limitation |
|---|---|---|---|---|---|
| `aead` | AES-256-GCM + XChaCha20-Poly1305, one API | 239 | 100.00% | round-trip + tamper tests | uses OpenSSL, not a hand-rolled AES; the "ARMv8 acceleration" is delegated to libcrypto |
| `partition` | four-partition model | 41 | 100.00% | magic + carve tests | constants are the TELESCREEN's; verified against the artifact |
| `crc` | `crc32_le`, U-Boot CRC32, CRC-16/CCITT | 34 | 100.00% | CRC-32 `0xCBF43926`, CRC-16 `0x29B1` | none |
| `env` | U-Boot environment CRC + lookup | 24 | 100.00% | round-trip + reject tests | simplified key scan (not U-Boot's full parser) |
| `jffs2` | node header parse + header CRC | 19 | 100.00% | header + tamper tests | header only; INODE/DIRENT data decode is out of scope |
| `container` | vendor kernel container | 17 | 100.00% | field + reject tests | validates the header, does not inflate in C (host tooling does) |
| `beacon` | weak UID-KDF + hardened AEAD seal | 20 | 100.00% | determinism + seal tests | the weak key is deliberate; it is the CTF's anti-example |
| `kex` | X25519 + HKDF-SHA256 | 59 | 100.00% | **RFC 7748**, **RFC 5869** | HKDF block buffer assumes `info_len <= 64` |
| `identity` | Ed25519 identity | 38 | 100.00% | **RFC 8032** test 1 | none |
| `collector` | local lab sink | 16 | 100.00% | accept/reject tests | in-memory only, by design |
| `teled` | daemon (identity, key, beacon, delivery) | 15 | 100.00% | beacon tests | integration glue; no threads/sockets in the unit build |
| `main` | entry point | 15 | n/a | build only | excluded from coverage by design (entry point) |

**Total owned lines at 100.00%: 522.**

## 4. Constant and Layout Verification

Code constants (`include/partition.h`) against the artifact `images/full.img`:

| partition | offset | size | magic | artifact check |
|---|---|---|---|---|
| boot | `0x000000` | `0x020000` | `15 05 00 ea` | **ok** |
| bootargs | `0x020000` | `0x010000` | CRC32 + text | **ok** |
| kernel | `0x030000` | `0x1C0000` | `21 84 1b 00` + `gziphead` | **ok** |
| rootfs | `0x1F0000` | `0xE10000` | `85 19 03 20` | **ok** |

```
$ python3 scripts/verify_telescreen.py --image images/full.img \
      --sha256 037cc979a58f04f3285132a7b745ca859b1604aeb80c9ef62eb5377e48bd18ec
sha256 037cc979a58f04f3285132a7b745ca859b1604aeb80c9ef62eb5377e48bd18ec
boot      magic ok
kernel    magic ok
rootfs    magic ok
exit=0
```

```
boot @0x0            ok got=150500ea
kernel @0x30000      ok got=21841b00
rootfs @0x1F0000     ok got=85190320
gziphead @0x30008    ok got=677a697068656164
```

## 5. Cryptographic Verification

| claim | standard | vector | result |
|---|---|---|---|
| X25519 shared secret | RFC 7748 | Alice/Bob published keys | **match** |
| HKDF-SHA256 | RFC 5869 | test case 1, L=42 | **match** |
| Ed25519 sign/verify | RFC 8032 | test 1 | **match** |
| CRC-32 | IEEE 802.3 | "123456789" | `0xCBF43926` **match** |
| CRC-16/CCITT-FALSE | standard | "123456789" | `0x29B1` **match** |

A bug was found and fixed during this review: `hkdf_block` originally returned `bool`
where the caller consumed a length, producing a one-byte-per-block expansion. The RFC
5869 vector caught it. This is exactly why standard vectors are mandatory.

## 6. Artifact Hashes (instructor-issued)

```
TELESCREEN-full.img   037cc979a58f04f3285132a7b745ca859b1604aeb80c9ef62eb5377e48bd18ec
TELESCREEN-boot.img   4193d6a9e9a29848813056f05356c17e4a4e64f8206148c86fd3b0d50a5e69fd
TELESCREEN-env.img    e935f998fd6289615d88d8d2c2d4d3aada0e04ad6dd7592c27a853431fec4d7e
TELESCREEN-kernel.img 8a5fb5725420d79ae4e21e39e27c161b7df88dbbdbd2b061d859dd889c8b0230
TELESCREEN-rootfs.img 38a8684afb5f8bb8b717b2ab333dc729054d36192aa6541be6e9fed6f19cd10e
```

These hashes match the values published in `TELESCREEN-R.md` and `TELESCREEN-S.md`.

## 7. Documentation Accuracy

| document | claim audited | verdict |
|---|---|---|
| `README.md` | four-partition offsets, sizes, magics | **accurate** |
| `TELESCREEN-I.md` | storyline + task list; four-partition parity | **accurate**; fiction is explicitly labelled |
| `TELESCREEN-R.md` | rubric + artifact hashes | **accurate**; hashes verified |
| `TELESCREEN-S.md` | solution addresses, patch bytes, key schedule | **accurate to the captured project**; addresses are the real device's |
| `docs/WEEK12_CORTEX_A_RE.md` | vector decode `0xEA000515 -> 0x145C`; `_start -> main` | **accurate**; arithmetic re-checked |
| `docs/01..30` | concept volumes | **accurate**; each command re-runnable |
| all documents | legal disclaimer present | **verified** (35 documents) |

## 8. Threat Model (as taught)

```
[Internet] -> PPPP relays (weak crypto)   -> remote eavesdrop/MITM
[Clients]  -> RTSP 554 / HTTP 80          -> unauthenticated media + web root
[Physical] -> flash / SD / UART           -> total control (no secure boot)
```

The project teaches both sides: how the device is built (the replica) and how it
fails (the backdoors, the weak KDF, the archive-to-root restore).

## 9. Honest Limitations (stated, not hidden)

1. **The supplied images are synthetic.** They carry the correct magics, offsets, and
   sizes so every carving, CRC, and JFFS2 lesson works, but they are not a dump of a
   real unit. The real device addresses in `-S` come from the captured-firmware
   project, not from these files.
2. **AES and ChaCha are provided by OpenSSL**, not hand-written. The lesson is the
   API and the discipline, not a from-scratch cipher.
3. **`jffs2` parses the header only.** Full INODE/DIRENT decoding and compression are
   handled by host tooling (`jefferson`), as in the field.
4. **`main` is excluded from coverage** by design (entry point), matching the reference
   repository.
5. **No hardware run is claimed here.** The RP5 boot/router/camera labs describe the
   procedure; they were not executed on a physical board in this repository.

## 10. Verdict

- **Code:** 11 modules, 522 owned lines, **100.00% line coverage**, 92 checks passing,
  every cryptographic primitive anchored to a published standard vector.
- **Standards:** **0** C violations, **0** Python violations, enforced by auditors run
  in CI.
- **Artifacts:** verified by SHA-256 and magic.
- **Documents:** 35, all carrying the legal disclaimer, all commands re-runnable.
- **Labelling:** fiction is labelled fiction; limitations are stated; nothing is
  presented as "unhackable."

**Result: the project is internally consistent, standard-verified, and honest about its
boundaries.** The single bug found during this review (the HKDF block length) is now
fixed and covered by the RFC 5869 vector that caught it.

---

*This review is reproducible: run the four gates and the two verification commands in
sections 2, 4, and 5.*
