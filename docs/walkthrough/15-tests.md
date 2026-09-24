# Walkthrough 15: The Test Suite

**every test in `test/test_telescreen.c`, explained**

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

## The Harness

The suite uses the **in-repo harness** (`test/harness/`), not a vendored framework. It is
~140 lines and does four things:

- `setUp`/`tearDown` per case;
- a `setjmp`/`longjmp` jail so one failed assert abandons only that case;
- `TEST_BEGIN`/`RUN_TEST`/`TEST_END` macros;
- assert macros (`TEST_ASSERT`, `TEST_ASSERT_EQUAL_MEMORY`, ...).

A case prints `:<name>:PASS` or `:<name>:FAIL`; the runner prints `N checks, M failures`.
The Python adapter (`test/test_telescreen.py`) asserts each `:<name>:PASS` is present, so
VS Code's Test Explorer shows 45 cases.

## Why the Tests Are Structured This Way

- **Check values, not round-trips, for crypto.** `test_kex_hkdf_rfc5869` pins the output
  to RFC 5869 - that is what caught the `hkdf_block` bug (`docs/walkthrough/12`).
- **One case per behaviour**, so a failure names the exact broken behaviour.
- **Odd inputs are first-class**: `bad_args`, `too_small`, `unknown`, `bad_tag`. These are
  the branches that reach 100% coverage.

## The Cases, by Module

### AEAD (11)

| test | proves |
| ---- | ------ |
| `test_aead_nonce_size_before_init` | not initialised -> nonce size 0 |
| `test_aead_init_rejects_bad_algo` | an unknown backend is refused |
| `test_aead_init_aes` | AES backend, nonce 12 |
| `test_aead_seal_bad_args` | each null argument -> `BAD_ARGUMENT` |
| `test_aead_open_bad_args` | each null argument -> `BAD_ARGUMENT` |
| `test_aead_seal_open_aes` | AES round trip recovers the plaintext |
| `test_aead_open_aes_bad_tag` | a corrupted tag -> `AUTHENTICATION_FAILED` |
| `test_aead_nonce_reuse` | sealing twice with one nonce is flagged |
| `test_aead_init_xchacha` | XChaCha backend, nonce 24 |
| `test_aead_seal_open_xchacha` | XChaCha round trip |
| `test_aead_open_xchacha_bad_tag` | corrupted XChaCha tag rejected |

The `bad_tag` cases are the security tests: they prove the tag actually gates the
plaintext.

### CRC (3)

| test | proves |
| ---- | ------ |
| `test_crc32_uboot_check` | `0xCBF43926` for `"123456789"` |
| `test_crc32_le_empty` | an empty buffer returns the seed |
| `test_crc16_check` | `0x29B1` for `"123456789"` |

### Environment (3)

| test | proves |
| ---- | ------ |
| `test_env_crc_valid` | a sealed env verifies and its keys are found |
| `test_env_crc_invalid` | an unsealed env fails; a missing key is `NULL` |
| `test_env_short` | short/null inputs are rejected |

### JFFS2 (2)

| test | proves |
| ---- | ------ |
| `test_jffs2_header` | magic, type, and `totlen` parse |
| `test_jffs2_crc` | a valid node verifies; a flipped byte fails |

### Container (2)

| test | proves |
| ---- | ------ |
| `test_container_fields` | magic, length, tag, and `valid` |
| `test_container_invalid` | null, short, and corrupted magic all fail |

### Beacon (4)

| test | proves |
| ---- | ------ |
| `test_beacon_weak_key` | the key is deterministic for a UID |
| `test_beacon_weak_key_differs` | different UIDs -> different keys |
| `test_beacon_nonce` | the sequence lands little-endian, byte 4 zero |
| `test_beacon_seal` | the hardened seal succeeds |

### KEX (2)

| test | proves |
| ---- | ------ |
| `test_kex_x25519_rfc7748` | the shared secret matches RFC 7748 both ways |
| `test_kex_hkdf_rfc5869` | the OKM matches RFC 5869 test case 1 |

### Identity (2)

| test | proves |
| ---- | ------ |
| `test_identity_rfc8032` | public key and signature match RFC 8032 test 1 |
| `test_identity_verify_bad` | a tampered signature is rejected |

### Collector (2)

| test | proves |
| ---- | ------ |
| `test_collector_accept` | a full frame is accepted and `last_seq` recorded |
| `test_collector_reject` | null and short frames are rejected |

### Teled (2)

| test | proves |
| ---- | ------ |
| `test_teled_beacon` | one sealed beacon, `beacons == 1`, UID intact |
| `test_teled_beacon_fail` | a null payload fails and is not counted |

### Camera (3)

| test | proves |
| ---- | ------ |
| `test_camera_rtsp_url` | the exact RTSP URL |
| `test_camera_mjpeg_url` | the exact MJPEG URL |
| `test_camera_url_bad` | a tiny buffer and null ip fail |

### Partition (12)

| test | proves |
| ---- | ------ |
| `test_part_desc_for_boot` | the boot descriptor's offset and name |
| `test_part_desc_for_unknown` | an unknown kind -> `NULL` |
| `test_part_identify_boot` | boot magic |
| `test_part_identify_kernel` | container magic |
| `test_part_identify_rootfs` | JFFS2 magic |
| `test_part_identify_env` | the fallback |
| `test_part_identify_unknown` | null/short -> unknown |
| `test_part_verify_layout` | full vs short image |
| `test_part_carve_ok` | carve copies the bytes |
| `test_part_carve_bad_args` | null args fail |
| `test_part_carve_too_small` | short image and short output fail |
| `test_part_carve_unknown_kind` | unknown kind fails |

## The Coverage Argument

98 cases across 12 modules. The `bad`/`unknown`/`too_small` cases are not padding - each
one reaches a branch that would otherwise be uncovered, which is why the gate can demand
**100.00% line coverage** on every owned file (`docs/walkthrough/14`).

## Run It

```bash
python3 scripts/run_tests.py          # 98 checks, 0 failures
python3 scripts/check_coverage.py     # exit 0
python3 -m unittest test.test_telescreen   # OK (45)
```

## Exercises

1. Add a test for a third AEAD backend stub and watch the gate fail until it is covered.
2. Break `crc32_uboot` (flip the final xor) and confirm two tests fail.
3. Add a `test_env_find_multiple` that finds two keys in one blob.
4. Explain why the `bad_tag` tests are security tests, not format tests.

## Reference

- `test/test_telescreen.c`, `test/harness/`
- `docs/appendix/E-test-listing.md`
