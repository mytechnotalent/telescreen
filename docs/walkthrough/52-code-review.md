# Walkthrough 52: Code Review

**a reviewer's pass over every module**

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

## How This Review Works

For each module: **what it must do, what could go wrong, what the code does, and the
verdict.** This is the kind of pass a second engineer does before a release.

## `crc`

**Must do.** Compute the correct CRC for each consumer.
**Could go wrong.** Reflected vs non-reflected; wrong poly; wrong init/final.
**Does.** Reflected core (`0xEDB88320`), `crc32_le` (seed, no final), `crc32_uboot`
(init/final), CRC-16/CCITT.
**Verdict.** Correct; pinned to the two published check values. `crc32_byte` is branchless
(no timing leak). **Pass.**

## `partition`

**Must do.** Identify and carve the four regions from bytes.
**Could go wrong.** Off-by-one in offsets; unaligned loads; missing bounds on `memcpy`.
**Does.** Byte-wise LE loads; magic-ordered identification; `out_size` checked before
`memcpy`.
**Verdict.** Correct; every branch tested to 100%. The `part_end` cast avoids 32-bit
overflow. **Pass.**

## `env`

**Must do.** Verify and read the U-Boot environment.
**Could go wrong.** Wrong CRC variant; reading past the buffer in the scan.
**Does.** `crc32_uboot`; the scan bound `(i + klen + 1) < len` guarantees in-range reads.
**Verdict.** Correct for the lab. **Note:** the scan does not require a key boundary; fine
here, flagged in the walkthrough. **Pass with note.**

## `container`

**Must do.** Validate the vendor kernel container.
**Could go wrong.** Trusting one field; OOB on short input.
**Does.** Magic **and** tag; a 16-byte length guard.
**Verdict.** Two independent fields make a false positive negligible. **Pass.**

## `jffs2`

**Must do.** Parse a node header and verify its CRC.
**Could go wrong.** Wrong CRC variant; wrong coverage.
**Does.** `crc32_le(0, node, 8)` compared to `node[8]`.
**Verdict.** Correct; 8-byte coverage, seed 0. **Pass.**

## `beacon`

**Must do.** Build a nonce and seal; hold the weak KDF as an anti-example.
**Could go wrong.** Nonce reuse; a weak key used as if secure.
**Does.** A monotonic sequence nonce; the weak key is clearly named `weak`.
**Verdict.** Correct. The weak key is intentional and labelled. **Pass (intentional flaw).**

## `collector`

**Must do.** Accept a well-formed frame; reject the rest.
**Could go wrong.** Length confusion; accepting short frames.
**Does.** Exact-length check; counters; no decrypt.
**Verdict.** Correct; no heap; local sink. **Pass.**

## `camera`

**Must do.** Build URLs safely.
**Could go wrong.** Buffer overflow on the URL.
**Does.** `snprintf` + fit check (truncation -> `false`).
**Verdict.** Correct and safe; the pattern to reuse. **Pass.**

## `kex`

**Must do.** X25519 + HKDF-SHA256.
**Could go wrong.** Wrong HKDF block length (a real bug); uninitialised PRK.
**Does.** Extract then expand; returns a **length** from `hkdf_block` (the fix).
**Verdict.** Correct; RFC 7748 and RFC 5869 vectors pass. **Pass (bug found and fixed).**

## `identity`

**Must do.** Ed25519 sign/verify/derive.
**Could go wrong.** Wrong digest handling; missing key/context checks.
**Does.** One-shot `EVP_DigestSign`/`Verify`; short-circuit guards.
**Verdict.** Correct; RFC 8032 vector passes; tamper rejected. **Pass.**

## `aead`

**Must do.** One API, two AEADs, tag-first open, nonce discipline.
**Could go wrong.** Tag checked after plaintext release; nonce reuse; subkey not wiped.
**Does.** Tag verified before success; plaintext cleared on failure; subkey/nonce wiped;
nonce-reuse guard.
**Verdict.** Correct; both round trips and both tamper cases pass. **Pass.**

## `teled`

**Must do.** Compose the modules; monotonic sequence; counted emission.
**Could go wrong.** Nonce reuse from a non-advancing sequence; counting failures.
**Does.** `seq++` before seal; count only on success.
**Verdict.** Correct. **Pass.**

## `main`

**Must do.** Wire it up; return a status.
**Could go wrong.** Uninitialised buffers; wrong backend.
**Does.** Zeroed buffers; AES-256-GCM; a status return.
**Verdict.** Correct (a lab stub; excluded from coverage). **Pass.**

## The Cross-Module Review

| question | answer |
| -------- | ------ |
| Any heap? | **No** |
| Any unbounded copy? | **No** (`snprintf`, checked `memcpy`) |
| Any `system()`? | **No** (the lab's app does not shell out) |
| Uninitialised secrets? | No; buffers zeroed, subkeys wiped |
| Nonce discipline? | Monotonic sequence |
| Tag-first open? | Yes |
| Threads? | No (single-threaded in the unit build) |

## What a Real Review Would Add

- run under **ASAN/UBSAN** and a fuzzer (`docs/walkthrough/46`, Lab H);
- a **static analyzer** pass (clang-tidy);
- a **second implementer** for the crypto usage.

## Exercises

1. Add `-fsanitize=address,undefined` to the test build and run the suite.
2. Fuzz `env_find` and `part_identify` with a mutation harness.
3. Review one module against its assembly and confirm the C.
4. Add a review note for any module you would change.

## Reference

- every `docs/modules/` volume; `docs/walkthrough/11`, `docs/walkthrough/12`
