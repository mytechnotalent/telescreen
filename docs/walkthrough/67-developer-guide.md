# Walkthrough 67: The Complete Developer Guide

**everything an engineer needs to build on this lab**

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

A developer who wants to **extend** the TELESCREEN lab: add a module, add a lab, port it, or
ship a variant. It consolidates every rule, tool, and convention in one place.

## 1. The Golden Rules

1. **House C style**: K&R braces, At most eight executable lines per body, **no blank lines
   in a body**, a Doxygen `@brief` on every entity.
2. **100% line coverage** on every owned file.
3. **All three gates green** before a commit.
4. **The legal disclaimer** on every document.
5. **No heap. No `system()`.** Bounded copies, checked lengths.
6. **Prove every claim with a command; label measured/inferred.**

## 2. The Layout

```
telescreen/
  include/    the headers (the API)
  src/        the modules
  test/       the suite + harness + mocks
  scripts/    the tools + the gates
  docs/       the curriculum + modules + appendices + walkthroughs
  CMakeLists.txt  the target build
  .github/    CI
```

## 3. Adding a Module

**Step 1 - header (`include/foo.h`).** The MIT header block, `#ifndef` guard, includes
(`<stdbool.h> <stddef.h> <stdint.h>`), a `@brief` on every macro, and the function
prototypes with `@brief`/`@param`/`@return`.

**Step 2 - source (`src/foo.c`).** The MIT header block, `#include "foo.h"`, file-scope
`static` helpers with `@brief`, and public functions each `<= 8` body lines.

**Step 3 - tests.** Add cases to `test/test_telescreen.c` (`#include "../src/foo.c"`), and a
`run_foo_tests()` group.

**Step 4 - wire the gates.**
```python
# scripts/run_tests.py
def _owned_sources():
    return [..., "src/foo.c"]
```
```python
# scripts/check_coverage.py
if any(src in line for src in (..., "foo.c")):
```

**Step 5 - the gates.**
```bash
python3 scripts/audit_c_standard.py
python3 scripts/audit_python_standard.py
python3 scripts/run_tests.py
python3 scripts/check_coverage.py
```

**Step 6 - a module volume** (`docs/modules/NN-foo.md`) with the header, source,
disassembly, and tests.

## 4. The Style, by Example

```c
/**
 * @brief Fold one byte into a reflected CRC-32 residual.
 *
 * @param crc Current running checksum.
 * @param byte Byte to fold in.
 * @return uint32_t Updated reflected checksum.
 */
static uint32_t crc32_byte(uint32_t crc, uint8_t byte) {
    uint8_t i;
    crc ^= (uint32_t)byte;
    for (i = 0u; i < 8u; ++i) {
        crc = (crc >> 1u) ^ (0xEDB88320u & (uint32_t)(-(int32_t)(crc & 1u)));
    }
    return crc;
}
```

Note: K&R `{` on the signature line; `u` suffixes; explicit casts; no blank lines; a full
`@brief`. That is the whole style in one function.

## 5. The Naming

| element | convention |
| ------- | ---------- |
| files | `snake_case` |
| functions | `snake_case`, module-prefixed |
| macros | `UPPER_SNAKE` |
| types | `snake_case_t` |
| file globals | `g_` prefix |
| statics | `snake_case` |

## 6. The Build

```bash
# host tests
python3 scripts/run_tests.py
# the RP5 target
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
```

## 7. Adding a Lab

Follow `docs/walkthrough/46` (advanced labs): goal, tools, steps, deliverable, points. Add
it to `docs/walkthrough/33` (the lab book) and update the rubric.

## 8. Adding a CTF Defect

1. Add the defect to the lab.
2. Add the finding to `CTF_telescreen/CTF-XX-R.md` and `-S.md`.
3. Build a hardened variant.
4. Update the rubric.

## 9. Adding a Document

Every `.md` starts with a title, then the disclaimer block. Use the house structure: **What
You'll Learn, the concept, commands, labs, reference.** That consistency is what makes the
docs readable.

## 10. The Tools You Have

| tool | use |
| ---- | --- |
| `carve.py` | carve the four partitions |
| `build_images.py` | build the four images |
| `verify_telescreen.py` | hash + magic |
| `weak_decrypt.py` | the weak KDF |
| `run_tests.py` | the suite |
| `check_coverage.py` | the gate |
| `audit_*.py` | the standards |

## 11. The Workflow

```
edit -> run the three gates -> fix -> commit
```

Nothing is "done" until the gates are green. A red gate is a **stop**, not a suggestion.

## 12. The Reviewing Standard

Before you commit:

```
[ ] the C audit is empty
[ ] the Python audit exits 0
[ ] the suite has 0 failures
[ ] coverage is 100.00%
[ ] no blank line in any body
[ ] every function has a @brief
[ ] the doc carries the disclaimer
[ ] the label (measured/inferred) is on every claim
```

## 13. A Worked Addition (mini)

Add a `bitrev` function to `crc`:

```c
/**
 * @brief Reverse the bits of a 32-bit word.
 *
 * @param v Input word.
 * @return uint32_t Bit-reversed word.
 */
uint32_t bitrev32(uint32_t v) {
    uint32_t r = 0u;
    uint8_t i;
    for (i = 0u; i < 32u; ++i) {
        r = (r << 1u) | (v & 1u);
        v >>= 1u;
    }
    return r;
}
```

Add a test (`TEST_ASSERT_EQUAL_UINT(0x80000000u, bitrev32(1u))`), add nothing to the gate
lists (already `crc.c`), run the gates, done.

## 14. Common Mistakes

| mistake | consequence |
| ------- | ----------- |
| a blank line in a body | audit fails |
| a nine-line body | audit fails |
| a missing `@brief` | audit fails |
| an uncovered branch | coverage gate fails |
| a doc without the disclaimer | inconsistent |
| an unlabelled claim | not defensible |
| `system()` from input | a defect, not a feature |

## 15. The Mindset

Build small, tested, documented parts; wire them with explicit rules; prove every claim. That
is what makes this lab **auditable** - and it is the same discipline a real product needs.

## Exercises

1. Add the `bitrev32` example and pass the gates.
2. Add a module of your own and its docs.
3. Add a lab to the lab book.
4. Review your change against the reviewing standard.

## Reference

- `docs/walkthrough/22` (build/CI), `docs/walkthrough/52` (code review)
- `docs/walkthrough/41` (design), `docs/walkthrough/46` (advanced labs)
