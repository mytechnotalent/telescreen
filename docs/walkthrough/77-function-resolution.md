# Walkthrough 77 - Resolve All 42 Functions

***
**LEGAL DISCLAIMER:** The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with. **IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
***

**Full teaching text:** `docs/33-ghidra-nation-state-re.md`. **Model answer:**
`ghidra/RESOLUTION_MAP.md` and `docs/appendix/J-ghidra-function-resolution.md`.

## Goal

Give a real name and a cited rule to every one of the **42** application
functions in `firmware/teled.stripped`.

## Method

1. Start at `main` (Walkthrough 76). Build the call graph with `X`.
2. Group functions by the imports and constants they use:

   | evidence | family |
   | -------- | ------ |
   | `EVP_Encrypt*` | aead seal |
   | `EVP_Decrypt*` | aead open |
   | `EVP_PKEY_new_raw_*` | identity / kex |
   | `0xEDB88320` constant | crc |
   | `0x1985` / `0x001b8421` | jffs2 / container |
   | `strlen` over a UID | beacon key derivation |

3. Name each function (`L`) and comment your rule (`;`).
4. Check yourself against `ghidra/RESOLUTION_MAP.md` **after** deciding.

## The 42, grouped by module

| module | functions |
| ------ | --------- |
| aead | `quarter_round`, `hchacha_rounds`, `hchacha_state`, `gcm_dec_final`, `aead_init`, `aead_nonce_size`, `aead_seal`, `aead_open` |
| beacon | `beacon_weak_key`, `beacon_nonce`, `beacon_seal` |
| camera | `camera_rtsp_url`, `camera_mjpeg_url` |
| collector | `collector_init`, `collector_accept`, `collector_stats` |
| container | `container_magic`, `container_length`, `container_tag_ok`, `container_valid` |
| crc | `crc32_le`, `crc32_uboot`, `crc16_ccitt` |
| env | `env_compute_crc`, `env_crc_valid`, `env_find` |
| identity | `identity_pubkey`, `identity_sign`, `identity_verify` |
| jffs2 | `jffs2_magic`, `jffs2_type`, `jffs2_totlen`, `jffs2_hdr_crc_valid` |
| kex | `kex_hkdf`, `kex_x25519` |
| partition | `part_desc_for`, `part_identify`, `part_verify_layout`, `part_carve` |
| teled | `teled_init`, `teled_beacon`, `teled_state` |

## Watch out for

- **ICF:** `gcm_dec_final` and `chacha_dec_final` share one address (`0x401320`).
- **Phantoms:** `0x401b5c` (of `beacon_seal`) and `0x40205c` (of `identity_sign`).
- **Dual PLT:** each import appears twice; do not double count.
- **Runtime:** `_start`, `_init`, `_fini`, the `*_tm_clones`, `frame_dummy` are
  toolchain code, not yours.

## Done when

- [ ] all 42 application functions are named in your Ghidra project
- [ ] each name cites R1-R4
- [ ] you can explain the ICF pair and the two phantoms
- [ ] your list matches `ghidra/RESOLUTION_MAP.md`
