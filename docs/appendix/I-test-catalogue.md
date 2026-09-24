# Appendix I: Test Catalogue

Every native test, its purpose, and its expected line. Run with `python3 scripts/run_tests.py`.

***
**LEGAL DISCLAIMER:** The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. **IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
***

| # | test | module | asserts |
|---|------|--------|---------|
| 1 | `test_aead_nonce_size_before_init` | | |
| 2 | `test_aead_init_rejects_bad_algo` | | |
| 3 | `test_aead_init_aes` | | |
| 4 | `test_aead_seal_bad_args` | | |
| 5 | `test_aead_open_bad_args` | | |
| 6 | `test_aead_seal_open_aes` | | |
| 7 | `test_aead_open_aes_bad_tag` | | |
| 8 | `test_aead_nonce_reuse` | | |
| 9 | `test_aead_init_xchacha` | | |
| 10 | `test_aead_seal_open_xchacha` | | |
| 11 | `test_aead_open_xchacha_bad_tag` | | |
| 12 | `test_part_desc_for_boot` | | |
| 13 | `test_part_desc_for_unknown` | | |
| 14 | `test_part_identify_boot` | | |
| 15 | `test_part_identify_kernel` | | |
| 16 | `test_part_identify_rootfs` | | |
| 17 | `test_part_identify_env` | | |
| 18 | `test_part_identify_unknown` | | |
| 19 | `test_part_verify_layout` | | |
| 20 | `test_part_carve_ok` | | |
| 21 | `test_part_carve_bad_args` | | |
| 22 | `test_part_carve_too_small` | | |
| 23 | `test_part_carve_unknown_kind` | | |
| 24 | `test_crc32_uboot_check` | | |
| 25 | `test_crc32_le_empty` | | |
| 26 | `test_crc16_check` | | |
| 27 | `test_env_crc_valid` | | |
| 28 | `test_env_crc_invalid` | | |
| 29 | `test_env_short` | | |
| 30 | `test_jffs2_header` | | |
| 31 | `test_jffs2_crc` | | |
| 32 | `test_container_fields` | | |
| 33 | `test_container_invalid` | | |
| 34 | `test_beacon_weak_key` | | |
| 35 | `test_beacon_weak_key_differs` | | |
| 36 | `test_beacon_nonce` | | |
| 37 | `test_beacon_seal` | | |
| 38 | `test_kex_x25519_rfc7748` | | |
| 39 | `test_kex_hkdf_rfc5869` | | |
| 40 | `test_identity_rfc8032` | | |
| 41 | `test_identity_verify_bad` | | |
| 42 | `test_collector_accept` | | |
| 43 | `test_collector_reject` | | |
| 44 | `test_teled_beacon` | | |
| 45 | `test_teled_beacon_fail` | | |
| 46 | `test_camera_rtsp_url` | | |
| 47 | `test_camera_mjpeg_url` | | |
| 48 | `test_camera_url_bad` | | |

## Quick Reference

```
aead      11 cases  both backends, both round trips, both tamper cases, guards
crc        3 cases  the two check values + the empty seed
env        3 cases  valid, invalid, short
jffs2      2 cases  header parse, header CRC
container  2 cases  fields, invalid
beacon     4 cases  determinism, difference, nonce, seal
kex        2 cases  RFC 7748, RFC 5869
identity   2 cases  RFC 8032, tamper
collector  2 cases  accept, reject
teled      2 cases  beacon, failure
camera     3 cases  RTSP, MJPEG, bad buffers
partition 12 cases  3 identify branches + fallback + unknown + carve + errors
--------------------------------------------
total     48 named cases (98 assertions)
```

## Expected Output (abridged)

```
$ python3 scripts/run_tests.py
:test_aead_seal_open_aes:PASS
:test_kex_x25519_rfc7748:PASS
:test_kex_hkdf_rfc5869:PASS
:test_identity_rfc8032:PASS
:test_part_carve_unknown_kind:PASS
98 checks, 0 failures
```

## Exercises

1. Run the suite and confirm 98 checks, 0 failures.
2. For each module, name the case that proves its security property.
3. Add a case and keep the coverage gate green.
