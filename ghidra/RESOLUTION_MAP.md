# TELESCREEN - Function Resolution Map

Every function Ghidra found in the stripped target, resolved to a
real identity, with the evidence that proves it.  Totals:

- **42** application functions (our firmware code)
- **72** imported-library thunks (OpenSSL / glibc)
- **2** phantom functions (alignment padding)
- **11** C runtime / start-up functions
- **127** total functions in the binary

## Application functions (resolved 1:1 against the unstripped twin)

| address | Ghidra label | resolved name | module | role |
| ------- | ------------ | ------------- | ------ | ---- |
| `0x00401120` | `FUN_00401120` | `quarter_round` | aead | Execute one ChaCha quarter-round. |
| `0x004011a0` | `FUN_004011a0` | `hchacha_rounds` | aead | Run ten HChaCha20 double rounds over the state. |
| `0x004012c0` | `FUN_004012c0` | `hchacha_state` | aead | Build the initial HChaCha20 state from key and nonce. |
| `0x00401320` | `FUN_00401320` | `gcm_dec_final` | aead | Decrypt the payload and verify the GCM tag. |
| `0x004013c0` | `FUN_004013c0` | `aead_init` | aead | Initialize the AEAD subsystem. |
| `0x00401400` | `FUN_00401400` | `aead_nonce_size` | aead | Return the nonce size required by the active backend. |
| `0x0040142c` | `FUN_0040142c` | `aead_seal` | aead | Seal a plaintext payload into ciphertext plus tag. |
| `0x004017e4` | `FUN_004017e4` | `aead_open` | aead | Open a sealed payload, verifying the tag before releasing plaintext. |
| `0x00401ae0` | `FUN_00401ae0` | `beacon_weak_key` | beacon | Derive the Ministry "sealed" key from the public device UID. |
| `0x00401b4c` | `FUN_00401b4c` | `beacon_nonce` | beacon | Build the beacon nonce from the sequence number. |
| `0x00401b60` | `FUN_00401b60` | `beacon_seal` | beacon | Seal one beacon payload with the hardened AEAD key. |
| `0x00401ba0` | `FUN_00401ba0` | `camera_rtsp_url` | camera | Build the RTSP URL a client (or VLC) connects to. |
| `0x00401c08` | `FUN_00401c08` | `camera_mjpeg_url` | camera | Build the MJPEG URL a browser connects to. |
| `0x00401c80` | `FUN_00401c80` | `collector_init` | collector | Reset the collector accounting state. |
| `0x00401ca0` | `FUN_00401ca0` | `collector_accept` | collector | Accept or reject one beacon frame. |
| `0x00401ce4` | `FUN_00401ce4` | `collector_stats` | collector | Return a pointer to the collector accounting snapshot. |
| `0x00401d00` | `FUN_00401d00` | `container_magic` | container | Read the container magic from the header. |
| `0x00401d08` | `FUN_00401d08` | `container_length` | container | Read the container length field from the header. |
| `0x00401d10` | `FUN_00401d10` | `container_tag_ok` | container | Verify the "gziphead" tag in the container header. |
| `0x00401d30` | `FUN_00401d30` | `container_valid` | container | Validate a vendor kernel container header. |
| `0x00401d80` | `FUN_00401d80` | `crc32_le` | crc | Compute the reflected crc32_le checksum used by JFFS2. |
| `0x00401de0` | `FUN_00401de0` | `crc32_uboot` | crc | Compute the standard CRC32 used by the U-Boot environment. |
| `0x00401e40` | `FUN_00401e40` | `crc16_ccitt` | crc | Compute the CRC-16/CCITT-FALSE checksum of a byte buffer. |
| `0x00401ec0` | `FUN_00401ec0` | `env_compute_crc` | env | Compute the environment CRC32 over the key/value blob. |
| `0x00401ee0` | `FUN_00401ee0` | `env_crc_valid` | env | Verify the leading environment CRC32. |
| `0x00401f20` | `FUN_00401f20` | `env_find` | env | Find a key in the environment blob and return its value. |
| `0x00401fe0` | `FUN_00401fe0` | `identity_pubkey` | identity | Derive the Ed25519 public key from a private seed. |
| `0x00402060` | `FUN_00402060` | `identity_sign` | identity | Sign a message with an Ed25519 private seed. |
| `0x00402160` | `FUN_00402160` | `identity_verify` | identity | Verify an Ed25519 signature. |
| `0x00402220` | `FUN_00402220` | `jffs2_magic` | jffs2 | Read the little-endian magic from a node header. |
| `0x00402228` | `FUN_00402228` | `jffs2_type` | jffs2 | Read the little-endian node type from a node header. |
| `0x00402230` | `FUN_00402230` | `jffs2_totlen` | jffs2 | Read the little-endian total length from a node header. |
| `0x00402240` | `FUN_00402240` | `jffs2_hdr_crc_valid` | jffs2 | Verify the crc32_le header CRC of a node. |
| `0x00402280` | `FUN_00402280` | `kex_hkdf` | kex | Derive key material with HKDF-SHA256. |
| `0x004023d0` | `FUN_004023d0` | `kex_x25519` | kex | Compute the X25519 shared secret. |
| `0x00402500` | `FUN_00402500` | `part_desc_for` | partition | Return the descriptor for a partition kind. |
| `0x00402560` | `FUN_00402560` | `part_identify` | partition | Identify a partition by its magic bytes. |
| `0x004025c0` | `FUN_004025c0` | `part_verify_layout` | partition | Verify that a whole-flash image is large enough and consistent. |
| `0x004025e0` | `FUN_004025e0` | `part_carve` | partition | Copy one partition out of a whole-flash image. |
| `0x004026c0` | `FUN_004026c0` | `teled_init` | teled | Initialize the daemon with a device UID and session key. |
| `0x00402728` | `FUN_00402728` | `teled_beacon` | teled | Seal and deliver one beacon to the local collector. |
| `0x0040278c` | `FUN_0040278c` | `teled_state` | teled | Return a pointer to the daemon state snapshot. |

## Imported-library thunks

| address | resolved name |
| ------- | ------------- |
| `0x00400d40` | `EVP_DigestSign` |
| `0x00400d50` | `memcpy` |
| `0x00400d60` | `EVP_EncryptUpdate` |
| `0x00400d70` | `EVP_DecryptInit_ex` |
| `0x00400d80` | `EVP_PKEY_derive_set_peer` |
| `0x00400d90` | `EVP_DigestVerify` |
| `0x00400da0` | `strlen` |
| `0x00400db0` | `EVP_DecryptFinal_ex` |
| `0x00400dc0` | `memcmp` |
| `0x00400dd0` | `EVP_DigestSignInit` |
| `0x00400de0` | `EVP_PKEY_CTX_new` |
| `0x00400df0` | `EVP_EncryptInit_ex` |
| `0x00400e00` | `EVP_DigestVerifyInit` |
| `0x00400e10` | `EVP_DecryptUpdate` |
| `0x00400e20` | `EVP_PKEY_free` |
| `0x00400e30` | `EVP_CIPHER_CTX_new` |
| `0x00400e40` | `EVP_PKEY_new_raw_public_key` |
| `0x00400e50` | `__libc_start_main` |
| `0x00400e60` | `EVP_PKEY_new_raw_private_key` |
| `0x00400e70` | `EVP_PKEY_derive` |
| `0x00400e80` | `HMAC` |
| `0x00400e90` | `EVP_CIPHER_CTX_free` |
| `0x00400ea0` | `EVP_MD_CTX_new` |
| `0x00400eb0` | `snprintf` |
| `0x00400ec0` | `EVP_chacha20_poly1305` |
| `0x00400ed0` | `EVP_CIPHER_CTX_ctrl` |
| `0x00400ee0` | `EVP_aes_256_gcm` |
| `0x00400ef0` | `EVP_PKEY_get_raw_public_key` |
| `0x00400f00` | `EVP_EncryptFinal_ex` |
| `0x00400f10` | `abort` |
| `0x00400f20` | `EVP_PKEY_CTX_free` |
| `0x00400f30` | `EVP_sha256` |
| `0x00400f40` | `EVP_MD_CTX_free` |
| `0x00400f50` | `__gmon_start__` |
| `0x00400f60` | `EVP_PKEY_derive_init` |
| `0x00421000` | `EVP_DigestSign` |
| `0x00421008` | `memcpy` |
| `0x00421010` | `EVP_EncryptUpdate` |
| `0x00421018` | `EVP_DecryptInit_ex` |
| `0x00421020` | `EVP_PKEY_derive_set_peer` |
| `0x00421028` | `EVP_DigestVerify` |
| `0x00421030` | `strlen` |
| `0x00421038` | `EVP_DecryptFinal_ex` |
| `0x00421040` | `memcmp` |
| `0x00421048` | `EVP_DigestSignInit` |
| `0x00421050` | `EVP_PKEY_CTX_new` |
| `0x00421058` | `EVP_EncryptInit_ex` |
| `0x00421060` | `EVP_DigestVerifyInit` |
| `0x00421068` | `EVP_DecryptUpdate` |
| `0x00421070` | `EVP_PKEY_free` |
| `0x00421078` | `EVP_CIPHER_CTX_new` |
| `0x00421080` | `EVP_PKEY_new_raw_public_key` |
| `0x00421088` | `__libc_start_main` |
| `0x00421090` | `EVP_PKEY_new_raw_private_key` |
| `0x00421098` | `EVP_PKEY_derive` |
| `0x004210a0` | `HMAC` |
| `0x004210a8` | `EVP_CIPHER_CTX_free` |
| `0x004210b0` | `EVP_MD_CTX_new` |
| `0x004210b8` | `snprintf` |
| `0x004210c0` | `EVP_chacha20_poly1305` |
| `0x004210c8` | `EVP_CIPHER_CTX_ctrl` |
| `0x004210d0` | `EVP_aes_256_gcm` |
| `0x004210d8` | `EVP_PKEY_get_raw_public_key` |
| `0x004210e0` | `EVP_EncryptFinal_ex` |
| `0x004210e8` | `abort` |
| `0x004210f0` | `EVP_PKEY_CTX_free` |
| `0x004210f8` | `EVP_sha256` |
| `0x00421100` | `_ITM_deregisterTMCloneTable` |
| `0x00421108` | `EVP_MD_CTX_free` |
| `0x00421110` | `__gmon_start__` |
| `0x00421118` | `_ITM_registerTMCloneTable` |
| `0x00421120` | `EVP_PKEY_derive_init` |

## Phantom functions (Ghidra artefacts - NOT real functions)

- `0x00401b5c` (`FUN_00401b5c`): R4: identical body to FUN_00401b60; alignment padding
- `0x0040205c` (`FUN_0040205c`): R4: identical body to FUN_00402060; alignment padding

## C runtime / start-up

- `0x00400d00` (`_DT_INIT`) -> `_init`
- `0x00400d20` (`FUN_00400d20`) -> `<.plt> PLT0 resolver`
- `0x00400f80` (`FUN_00400f80`) -> `main`
- `0x00401000` (`entry`) -> `_start`
- `0x00401040` (`FUN_00401040`) -> `_dl_relocate_static_pie`
- `0x00401048` (`FUN_00401048`) -> `call_weak_fn`
- `0x00401060` (`FUN_00401060`) -> `deregister_tm_clones`
- `0x00401090` (`FUN_00401090`) -> `register_tm_clones`
- `0x004010cc` (`_FINI_0`) -> `__do_global_dtors_aux`
- `0x00401100` (`_INIT_0`) -> `frame_dummy`
- `0x00402798` (`_DT_FINI`) -> `_fini`

