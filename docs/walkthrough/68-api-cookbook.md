# Walkthrough 68: The API Cookbook

**copy-paste usage for every public function**

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

## How to Use

Each entry: the prototype, a usage snippet, and the failure modes. This is the practical
companion to `docs/appendix/C-source-listing.md`.

## AEAD (`aead.h`)

```c
bool aead_init(aead_algo_t algo);
size_t aead_nonce_size(void);
aead_result_t aead_seal(const uint8_t key[32], const uint8_t *nonce,
                        const uint8_t plaintext[48], uint8_t sealed[64]);
aead_result_t aead_open(const uint8_t key[32], const uint8_t *nonce,
                        const uint8_t sealed[64], uint8_t plaintext[48]);
```

```c
uint8_t key[32] = {0};
uint8_t nonce[24];
uint8_t pt[48] = {0}, sealed[64], out[48];

aead_init(AEAD_ALGO_XCHACHA20_POLY1305);
memset(nonce, 0, aead_nonce_size());
if (aead_seal(key, nonce, pt, sealed) == AEAD_RESULT_SUCCESS) {
    if (aead_open(key, nonce, sealed, out) == AEAD_RESULT_SUCCESS) {
        /* out == pt */
    }
}
```

**Fails when.** not initialised -> `BAD_ARGUMENT`; a null pointer -> `BAD_ARGUMENT`; a
modified frame -> `AUTHENTICATION_FAILED`.

## KEX (`kex.h`)

```c
bool kex_x25519(const uint8_t priv[32], const uint8_t peer[32], uint8_t out[32]);
bool kex_hkdf(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt, size_t salt_len,
              const uint8_t *info, size_t info_len, uint8_t *out, size_t out_len);
```

```c
uint8_t shared[32], key[32];
static const uint8_t info[] = "telescreen-session";
kex_x25519(my_priv, peer_pub, shared);
kex_hkdf(shared, 32, salt, 16, info, sizeof(info) - 1, key, sizeof(key));
```

**Fails when.** a bad key object -> `false`; a wrong length -> `false`.

## Identity (`identity.h`)

```c
bool identity_pubkey(const uint8_t priv[32], uint8_t pub[32]);
bool identity_sign(const uint8_t priv[32], const uint8_t *msg, size_t n, uint8_t sig[64]);
bool identity_verify(const uint8_t pub[32], const uint8_t *msg, size_t n, const uint8_t sig[64]);
```

```c
uint8_t pub[32], sig[64];
identity_pubkey(seed, pub);
if (identity_sign(seed, msg, msg_len, sig)) {
    bool ok = identity_verify(pub, msg, msg_len, sig);   /* true */
}
```

**Fails when.** a tampered message or signature -> `false`.

## CRC (`crc.h`)

```c
uint32_t crc32_le(uint32_t seed, const uint8_t *buf, size_t len);
uint32_t crc32_uboot(const uint8_t *buf, size_t len);
uint16_t crc16_ccitt(const uint8_t *buf, size_t len);
```

```c
uint32_t hdr = crc32_le(0u, node, 8u);          /* JFFS2 */
uint32_t env = crc32_uboot(blob, blob_len);      /* U-Boot env */
uint16_t fr  = crc16_ccitt(frame, frame_len);    /* telemetry */
```

**Note.** `crc32_le` is reflected with a seed; `crc32_uboot` adds init/final; they differ by
one line.

## partition (`partition.h`)

```c
const part_desc_t *part_desc_for(part_kind_t kind);
part_kind_t part_identify(const uint8_t *buf, size_t len);
part_result_t part_verify_layout(size_t image_size);
part_result_t part_carve(const uint8_t *image, size_t image_size, part_kind_t kind,
                         uint8_t *out, size_t out_size);
```

```c
if (part_verify_layout(size) == PART_RESULT_OK) {
    part_kind_t k = part_identify(buf, 16);      /* e.g. PART_KIND_BOOT */
    part_carve(image, size, k, out, sizeof(out));
}
```

**Fails when.** a short image -> `TOO_SMALL`; a small output -> `TOO_SMALL`; unknown -> `BAD_ARGUMENT`.

## env (`env.h`)

```c
uint32_t env_compute_crc(const uint8_t *env, size_t len);
bool env_crc_valid(const uint8_t *env, size_t len);
const char *env_find(const uint8_t *env, size_t len, const char *key);
```

```c
if (env_crc_valid(env, len)) {
    const char *v = env_find(env, len, "bootargs");
}
```

**Fails when.** a short buffer -> `false` / `NULL`.

## container (`container.h`)

```c
uint32_t container_magic(const uint8_t *buf);
uint32_t container_length(const uint8_t *buf);
bool container_tag_ok(const uint8_t *buf);
bool container_valid(const uint8_t *buf, size_t len);
```

```c
if (container_valid(buf, len)) {
    uint32_t n = container_length(buf);
}
```

## jffs2 (`jffs2.h`)

```c
uint16_t jffs2_magic(const uint8_t *node);
uint16_t jffs2_type(const uint8_t *node);
uint32_t jffs2_totlen(const uint8_t *node);
bool jffs2_hdr_crc_valid(const uint8_t *node);
```

```c
if (jffs2_magic(node) == JFFS2_MAGIC && jffs2_hdr_crc_valid(node)) {
    uint32_t next = off + jffs2_totlen(node);
}
```

## beacon (`beacon.h`)

```c
void beacon_weak_key(const char *uid, uint8_t out[32]);
void beacon_nonce(uint32_t seq, uint8_t *out);
bool beacon_seal(const uint8_t key[32], uint32_t seq, const uint8_t *pt, uint8_t *sealed);
```

```c
uint8_t key[32], sealed[64];
beacon_weak_key(uid, key);          /* THE FLAW - use HKDF instead */
beacon_seal(key, seq, payload, sealed);
```

**Note.** `beacon_weak_key` is the anti-example (`docs/17`); use `kex_hkdf` for real keys.

## collector (`collector.h`)

```c
void collector_init(void);
bool collector_accept(const uint8_t *sealed, size_t len, uint32_t seq);
const collector_stat_t *collector_stats(void);
```

```c
collector_init();
collector_accept(sealed, sizeof(sealed), seq);
uint32_t n = collector_stats()->accepted;
```

## camera (`camera.h`)

```c
bool camera_rtsp_url(const char *ip, uint16_t port, char *out, size_t out_len);
bool camera_mjpeg_url(const char *ip, uint16_t port, char *out, size_t out_len);
```

```c
char url[64];
camera_rtsp_url("192.168.1.88", 554, url, sizeof(url));
/* url == "rtsp://192.168.1.88:554/stream" */
```

**Fails when.** the buffer is too small -> `false` (truncation caught).

## teled (`teled.h`)

```c
void teled_init(const char *uid, const uint8_t key[32]);
bool teled_beacon(const uint8_t *payload);
const teled_state_t *teled_state(void);
```

```c
teled_init("SSAT-468547-FEEBD", key);
teled_beacon(payload);
uint32_t seq = teled_state()->seq;
```

## The Patterns to Remember

| pattern | rule |
| ------- | ---- |
| initialise before use | `aead_init`, `teled_init`, `collector_init` |
| check the return | every `bool`/`aead_result_t` |
| bound the output | `snprintf` + the fit check |
| secret keys | never `beacon_weak_key` in production |
| fresh nonces | `beacon_nonce(seq, ...)` with a monotonic `seq` |

## Exercises

1. Write a program that seals and opens a beacon end to end.
2. Write a program that derives a session key with X25519 + HKDF.
3. Write a program that identifies and carves the four partitions.
4. Add one function to a module and its cookbook entry.

## Reference

- `docs/appendix/C-source-listing.md`, `docs/appendix/G-api-reference.md`
- every `docs/modules/` volume
