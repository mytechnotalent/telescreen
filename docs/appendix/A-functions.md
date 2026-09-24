# Appendix A: Function Catalogue

Every function in every owned module, with file and body length.

***
**LEGAL DISCLAIMER:** The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with. **IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
***

| module | function | body lines |
| ------ | -------- | ---------- |
| aead | `clear_bytes` | 4 |
| aead | `rotl32` | 1 |
| aead | `load32_le` | 2 |
| aead | `store32_le` | 4 |
| aead | `quarter_round` | 4 |
| aead | `hchacha_cols` | 4 |
| aead | `hchacha_diag` | 4 |
| aead | `hchacha_rounds` | 5 |
| aead | `hchacha_state` | 4 |
| aead | `hchacha_store` | 8 |
| aead | `build_inner_nonce` | 2 |
| aead | `nonce_reused` | 4 |
| aead | `gcm_enc_final` | 4 |
| aead | `chacha_enc_final` | 4 |
| aead | `xchacha_wipe` | 2 |
| aead | `aead_init` | 8 |
| beacon | `beacon_weak_key` | 7 |
| camera | `camera_rtsp_url` | 1 |
| collector | `collector_init` | 3 |
| container | `cont_read32` | 2 |
| container | `container_magic` | 1 |
| container | `container_length` | 1 |
| container | `container_tag_ok` | 1 |
| crc | `crc32_byte` | 6 |
| crc | `crc16_byte` | 9 |
| crc | `crc32_le` | 4 |
| crc | `crc32_uboot` | 1 |
| env | `env_read32` | 2 |
| env | `env_compute_crc` | 2 |
| identity | `ed_sign_init` | 1 |
| jffs2 | `jffs2_read16` | 1 |
| jffs2 | `jffs2_read32` | 2 |
| jffs2 | `jffs2_magic` | 1 |
| jffs2 | `jffs2_type` | 1 |
| jffs2 | `jffs2_totlen` | 1 |
| kex | `x25519_derive` | 4 |
| partition | `part_read32` | 2 |
| partition | `part_read16` | 1 |
| partition | `part_end` | 1 |
| partition | `part_identify` | 5 |
| teled | `teled_init` | 4 |

**Total functions: 54.**
