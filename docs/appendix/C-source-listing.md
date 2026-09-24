# Appendix C: Full Source Listing

Every header and module, verbatim, in the house C style.

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

## `include/aead.h`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent
// File:    aead.h
// Desc:    Declares the TELESCREEN authenticated-encryption API. One interface,
//          two backends: AES-256-GCM (ARMv8 crypto extensions) and
//          XChaCha20-Poly1305 (portable, 192-bit nonce). The hardened beacon uses
//          this API to seal and open telemetry so a forged frame dies at the tag.
// Created: 2026

#ifndef AEAD_H
#define AEAD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief AEAD backend selector.
 *
 * AES-256-GCM is the hardware-accelerated default on the RP5 (BCM2712 Cortex-A76
 * with the ARMv8 Cryptography Extensions). XChaCha20-Poly1305 is the portable
 * twin used when no AES engine is present or when a 192-bit random nonce is
 * preferred.
 */
typedef enum aead_algo {
    AEAD_ALGO_AES_256_GCM = 0,
    AEAD_ALGO_XCHACHA20_POLY1305 = 1,
} aead_algo_t;

/**
 * @brief AES-256-GCM key size in bytes.
 */
#define AEAD_AES_KEY_SIZE 32u

/**
 * @brief AES-GCM recommended nonce size in bytes.
 *
 * A 96-bit nonce is the IETF-recommended size for GCM and is used directly by
 * the counter block. It MUST never repeat under a given key.
 */
#define AEAD_AES_NONCE_SIZE 12u

/**
 * @brief XChaCha20-Poly1305 key size in bytes.
 */
#define AEAD_XCHACHA_KEY_SIZE 32u

/**
 * @brief XChaCha20-Poly1305 nonce size in bytes.
 *
 * The extended 192-bit nonce is large enough to be chosen at random per message
 * without a counter, which removes an entire class of misuse.
 */
#define AEAD_XCHACHA_NONCE_SIZE 24u

/**
 * @brief Authentication tag size in bytes for both backends.
 *
 * GCM and Poly1305 both emit a 128-bit tag. A forged frame fails verification
 * here before any plaintext is released.
 */
#define AEAD_TAG_SIZE 16u

/**
 * @brief Hardened beacon payload size in bytes.
 *
 * The fixed telemetry payload is 48 bytes: a sequence number, a device-state
 * byte, and trailing reserved bytes matching the captured-device layout.
 */
#define AEAD_PAYLOAD_SIZE 48u

/**
 * @brief Full ciphertext-plus-tag size in bytes.
 */
#define AEAD_SEALED_SIZE (AEAD_PAYLOAD_SIZE + AEAD_TAG_SIZE)

/**
 * @brief AEAD result codes.
 *
 * These values let the caller distinguish a bad argument from a cryptographic
 * authentication failure without guessing.
 */
typedef enum aead_result {
    AEAD_RESULT_SUCCESS = 0,
    AEAD_RESULT_BAD_ARGUMENT = 1,
    AEAD_RESULT_AUTHENTICATION_FAILED = 2,
    AEAD_RESULT_INTERNAL_ERROR = 3,
} aead_result_t;

/**
 * @brief Initialize the AEAD subsystem.
 *
 * Selects the backend, verifies that a self-test vector passes, and prepares the
 * module for sealing and opening. The self-test also confirms that nonce reuse
 * detection is armed in debug builds.
 *
 * @param algo Backend selector.
 * @return bool true when initialization succeeds, else false.
 */
bool aead_init(aead_algo_t algo);

/**
 * @brief Return the nonce size required by the active backend.
 *
 * @param None.
 * @return size_t Nonce size in bytes, or 0 when not initialized.
 */
size_t aead_nonce_size(void);

/**
 * @brief Seal a plaintext payload into ciphertext plus tag.
 *
 * Encrypts the fixed-size payload under the supplied key and nonce and appends a
 * 128-bit authentication tag. The nonce MUST be unique for every call under the
 * same key.
 *
 * @param key Pointer to 32-byte AEAD key.
 * @param nonce Pointer to backend nonce bytes.
 * @param plaintext Pointer to AEAD_PAYLOAD_SIZE plaintext bytes.
 * @param sealed_out Output AEAD_SEALED_SIZE buffer (ciphertext then tag).
 * @return aead_result_t Detailed sealing outcome.
 */
aead_result_t aead_seal(const uint8_t key[32],
                        const uint8_t *nonce,
                        const uint8_t plaintext[AEAD_PAYLOAD_SIZE],
                        uint8_t sealed_out[AEAD_SEALED_SIZE]);

/**
 * @brief Open a sealed payload, verifying the tag before releasing plaintext.
 *
 * Decrypts the ciphertext and verifies the 128-bit tag. Plaintext is written to
 * the caller buffer only when the tag verifies.
 *
 * @param key Pointer to 32-byte AEAD key.
 * @param nonce Pointer to backend nonce bytes.
 * @param sealed Pointer to AEAD_SEALED_SIZE sealed bytes.
 * @param plaintext_out Output AEAD_PAYLOAD_SIZE plaintext buffer.
 * @return aead_result_t Detailed opening outcome.
 */
aead_result_t aead_open(const uint8_t key[32],
                        const uint8_t *nonce,
                        const uint8_t sealed[AEAD_SEALED_SIZE],
                        uint8_t plaintext_out[AEAD_PAYLOAD_SIZE]);

#endif // AEAD_H
```

## `include/beacon.h`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    beacon.h
// Desc:    Declares the TELESCREEN exfiltration beacon. The Ministry derives its
//          key from the public device UID, which is obfuscation rather than
//          encryption. The hardened path uses the AEAD API instead.
// Created: 2026

#ifndef BEACON_H
#define BEACON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Beacon key size in bytes.
 */
#define BEACON_KEY_SIZE 32u

/**
 * @brief Beacon sequence number size in bytes.
 */
#define BEACON_SEQ_SIZE 4u

/**
 * @brief Derive the Ministry "sealed" key from the public device UID.
 *
 * This is deliberately weak: the key is a deterministic function of the public
 * UID, so anyone who knows the UID can recompute it. It exists to demonstrate
 * the failure, not to protect anything.
 *
 * @param uid NUL-terminated public device identifier.
 * @param out Output 32-byte key buffer.
 * @return void
 */
void beacon_weak_key(const char *uid, uint8_t out[BEACON_KEY_SIZE]);

/**
 * @brief Build the beacon nonce from the sequence number.
 *
 * @param seq Monotonic sequence number.
 * @param out Output nonce buffer of AEAD nonce size.
 * @return void
 */
void beacon_nonce(uint32_t seq, uint8_t *out);

/**
 * @brief Seal one beacon payload with the hardened AEAD key.
 *
 * @param key Pointer to 32-byte session key.
 * @param seq Monotonic sequence number.
 * @param plaintext Pointer to AEAD payload bytes.
 * @param sealed_out Output AEAD sealed buffer.
 * @return bool true when sealing succeeds.
 */
bool beacon_seal(const uint8_t key[BEACON_KEY_SIZE], uint32_t seq,
                 const uint8_t *plaintext, uint8_t *sealed_out);

#endif // BEACON_H
```

## `include/camera.h`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    camera.h
// Desc:    Declares the UVC camera helpers for the TELESCREEN lab. A cheap USB
//          webcam enumerates as /dev/video0, so the pipeline is plain V4L2.
// Created: 2026

#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Default UVC capture device path.
 */
#define CAMERA_DEV_PATH "/dev/video0"

/**
 * @brief Default RTSP path served by the lab.
 */
#define CAMERA_RTSP_PATH "/stream"

/**
 * @brief Default MJPEG path served over HTTP for app-free viewing.
 */
#define CAMERA_MJPEG_PATH "/video.mjpg"

/**
 * @brief Default RTSP service port.
 */
#define CAMERA_RTSP_PORT 554u

/**
 * @brief Default HTTP service port.
 */
#define CAMERA_HTTP_PORT 80u

/**
 * @brief Build the RTSP URL a client (or VLC) connects to.
 *
 * @param ip NUL-terminated device IPv4 address.
 * @param port TCP port.
 * @param out Output URL buffer.
 * @param out_len Size of the output buffer.
 * @return bool true when the URL fits and is written.
 */
bool camera_rtsp_url(const char *ip, uint16_t port, char *out, size_t out_len);

/**
 * @brief Build the MJPEG URL a browser connects to.
 *
 * @param ip NUL-terminated device IPv4 address.
 * @param port TCP port.
 * @param out Output URL buffer.
 * @param out_len Size of the output buffer.
 * @return bool true when the URL fits and is written.
 */
bool camera_mjpeg_url(const char *ip, uint16_t port, char *out, size_t out_len);

#endif // CAMERA_H
```

## `include/collector.h`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    collector.h
// Desc:    Declares the local lab sink that receives TELESCREEN beacons. It is
//          local-only by design and never bridges to a production network.
// Created: 2026

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Collector accounting snapshot.
 */
typedef struct collector_stat {
    /**
     * @brief Number of accepted beacons.
     */
    uint32_t accepted;
    /**
     * @brief Number of rejected beacons.
     */
    uint32_t rejected;
    /**
     * @brief Sequence number of the most recent accepted beacon.
     */
    uint32_t last_seq;
} collector_stat_t;

/**
 * @brief Reset the collector accounting state.
 *
 * @param void No parameters.
 * @return void
 */
void collector_init(void);

/**
 * @brief Accept or reject one beacon frame.
 *
 * @param sealed Pointer to the sealed frame bytes.
 * @param len Number of frame bytes.
 * @param seq Sequence number carried by the frame.
 * @return bool true when the frame is accepted.
 */
bool collector_accept(const uint8_t *sealed, size_t len, uint32_t seq);

/**
 * @brief Return a pointer to the collector accounting snapshot.
 *
 * @param void No parameters.
 * @return const collector_stat_t* Pointer to the static snapshot.
 */
const collector_stat_t *collector_stats(void);

#endif // COLLECTOR_H
```

## `include/container.h`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    container.h
// Desc:    Declares the vendor kernel container helpers. The kernel partition is
//          not a zImage or uImage; it is a vendor container of the form
//          [magic 0x001B8421][length]["gziphead"][gzip stream named "Image"].
// Created: 2026

#ifndef CONTAINER_H
#define CONTAINER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Vendor kernel container magic value.
 */
#define CONTAINER_MAGIC 0x001B8421u

/**
 * @brief Vendor kernel container header size in bytes.
 */
#define CONTAINER_HDR_SIZE 16u

/**
 * @brief Read the container magic from the header.
 *
 * @param buf Pointer to the container header bytes.
 * @return uint32_t Parsed magic value.
 */
uint32_t container_magic(const uint8_t *buf);

/**
 * @brief Read the container length field from the header.
 *
 * @param buf Pointer to the container header bytes.
 * @return uint32_t Parsed length value.
 */
uint32_t container_length(const uint8_t *buf);

/**
 * @brief Verify the "gziphead" tag in the container header.
 *
 * @param buf Pointer to the container header bytes.
 * @return bool true when the tag is present.
 */
bool container_tag_ok(const uint8_t *buf);

/**
 * @brief Validate a vendor kernel container header.
 *
 * @param buf Pointer to the container bytes.
 * @param len Number of readable bytes.
 * @return bool true when the magic and tag both validate.
 */
bool container_valid(const uint8_t *buf, size_t len);

#endif // CONTAINER_H
```

## `include/crc.h`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    crc.h
// Desc:    Declares the two integrity primitives the TELESCREEN images use: the
//          reflected crc32_le used by JFFS2 nodes, the standard CRC32 used by the
//          U-Boot environment, and the CRC-16/CCITT used by telemetry frames.
// Created: 2026

#ifndef CRC_H
#define CRC_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief CRC-16/CCITT seed value used for telemetry frame integrity.
 */
#define CRC16_INIT 0xFFFFu

/**
 * @brief Compute the reflected crc32_le checksum used by JFFS2.
 *
 * Uses the kernel crc32_le convention: reflected, caller-supplied seed, and no
 * final inversion. JFFS2 stores hdr_crc, node_crc, and data_crc this way.
 *
 * @param seed Initial checksum value.
 * @param buf Pointer to readable bytes.
 * @param len Number of bytes to checksum.
 * @return uint32_t Computed reflected CRC-32 residual.
 */
uint32_t crc32_le(uint32_t seed, const uint8_t *buf, size_t len);

/**
 * @brief Compute the standard CRC32 used by the U-Boot environment.
 *
 * Initializes with 0xFFFFFFFF and inverts the result, which is the convention
 * U-Boot applies to the environment blob.
 *
 * @param buf Pointer to readable bytes.
 * @param len Number of bytes to checksum.
 * @return uint32_t Computed standard CRC-32 value.
 */
uint32_t crc32_uboot(const uint8_t *buf, size_t len);

/**
 * @brief Compute the CRC-16/CCITT-FALSE checksum of a byte buffer.
 *
 * @param buf Pointer to readable bytes.
 * @param len Number of bytes to checksum.
 * @return uint16_t Computed CRC-16 residual value.
 */
uint16_t crc16_ccitt(const uint8_t *buf, size_t len);

#endif // CRC_H
```

## `include/env.h`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    env.h
// Desc:    Declares the U-Boot environment helpers. The environment partition
//          begins with a little-endian CRC32 of the remaining bytes, followed by
//          a NUL-separated key=value blob.
// Created: 2026

#ifndef ENV_H
#define ENV_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Size of the leading environment CRC field in bytes.
 */
#define ENV_CRC_SIZE 4u

/**
 * @brief Compute the environment CRC32 over the key/value blob.
 *
 * The blob begins after the four-byte CRC field and runs to the end of the
 * supplied buffer.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @return uint32_t Computed standard CRC-32 over the blob.
 */
uint32_t env_compute_crc(const uint8_t *env, size_t len);

/**
 * @brief Verify the leading environment CRC32.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @return bool true when the stored CRC matches the computed value.
 */
bool env_crc_valid(const uint8_t *env, size_t len);

/**
 * @brief Find a key in the environment blob and return its value.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @param key NUL-terminated key to locate.
 * @return const char* Pointer to the value, or NULL when absent.
 */
const char *env_find(const uint8_t *env, size_t len, const char *key);

#endif // ENV_H
```

## `include/identity.h`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    identity.h
// Desc:    Declares the TELESCREEN device identity: Ed25519 public-key derivation,
//          signing, and verification. This replaces RSA for the hardened build.
// Created: 2026

#ifndef IDENTITY_H
#define IDENTITY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Ed25519 private seed and public key size in bytes.
 */
#define IDENTITY_KEY_SIZE 32u

/**
 * @brief Ed25519 signature size in bytes.
 */
#define IDENTITY_SIG_SIZE 64u

/**
 * @brief Derive the Ed25519 public key from a private seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @param pub Output 32-byte public key.
 * @return bool true when derivation succeeds.
 */
bool identity_pubkey(const uint8_t priv[IDENTITY_KEY_SIZE],
                     uint8_t pub[IDENTITY_KEY_SIZE]);

/**
 * @brief Sign a message with an Ed25519 private seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @param msg Pointer to message bytes.
 * @param msg_len Number of message bytes.
 * @param sig Output 64-byte signature.
 * @return bool true when signing succeeds.
 */
bool identity_sign(const uint8_t priv[IDENTITY_KEY_SIZE], const uint8_t *msg,
                   size_t msg_len, uint8_t sig[IDENTITY_SIG_SIZE]);

/**
 * @brief Verify an Ed25519 signature.
 *
 * @param pub Pointer to 32-byte public key.
 * @param msg Pointer to message bytes.
 * @param msg_len Number of message bytes.
 * @param sig Pointer to 64-byte signature.
 * @return bool true when the signature verifies.
 */
bool identity_verify(const uint8_t pub[IDENTITY_KEY_SIZE], const uint8_t *msg,
                     size_t msg_len, const uint8_t sig[IDENTITY_SIG_SIZE]);

#endif // IDENTITY_H
```

## `include/jffs2.h`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    jffs2.h
// Desc:    Declares the JFFS2 node header helpers for the TELESCREEN rootfs:
//          magic, node type, total length, and the crc32_le-protected header CRC.
// Created: 2026

#ifndef JFFS2_H
#define JFFS2_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief JFFS2 little-endian magic value.
 */
#define JFFS2_MAGIC 0x1985u

/**
 * @brief Common JFFS2 node header size in bytes.
 */
#define JFFS2_HDR_SIZE 12u

/**
 * @brief JFFS2 DIRENT node type.
 */
#define JFFS2_TYPE_DIRENT 0xE001u

/**
 * @brief JFFS2 INODE node type.
 */
#define JFFS2_TYPE_INODE 0xE002u

/**
 * @brief JFFS2 CLEANMARKER node type.
 */
#define JFFS2_TYPE_CLEANMARKER 0x2003u

/**
 * @brief JFFS2 PADDING node type.
 */
#define JFFS2_TYPE_PADDING 0x2004u

/**
 * @brief Read the little-endian magic from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint16_t Parsed magic value.
 */
uint16_t jffs2_magic(const uint8_t *node);

/**
 * @brief Read the little-endian node type from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint16_t Parsed node type.
 */
uint16_t jffs2_type(const uint8_t *node);

/**
 * @brief Read the little-endian total length from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint32_t Parsed total length in bytes.
 */
uint32_t jffs2_totlen(const uint8_t *node);

/**
 * @brief Verify the crc32_le header CRC of a node.
 *
 * The stored header CRC covers the first eight bytes of the node.
 *
 * @param node Pointer to the node header bytes.
 * @return bool true when the stored header CRC matches the computed value.
 */
bool jffs2_hdr_crc_valid(const uint8_t *node);

#endif // JFFS2_H
```

## `include/kex.h`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    kex.h
// Desc:    Declares the hardened key agreement: X25519 for the shared secret and
//          HKDF-SHA256 to derive the AEAD session key.
// Created: 2026

#ifndef KEX_H
#define KEX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief X25519 key and shared-secret size in bytes.
 */
#define KEX_KEY_SIZE 32u

/**
 * @brief HKDF output key size in bytes.
 */
#define KEX_OKM_SIZE 32u

/**
 * @brief Compute the X25519 shared secret.
 *
 * @param priv Pointer to 32-byte private scalar.
 * @param peer Pointer to 32-byte peer public key.
 * @param out Output 32-byte shared secret.
 * @return bool true when the derivation succeeds.
 */
bool kex_x25519(const uint8_t priv[KEX_KEY_SIZE], const uint8_t peer[KEX_KEY_SIZE],
                uint8_t out[KEX_KEY_SIZE]);

/**
 * @brief Derive key material with HKDF-SHA256.
 *
 * @param ikm Pointer to input key material.
 * @param ikm_len Number of input key material bytes.
 * @param salt Pointer to salt bytes.
 * @param salt_len Number of salt bytes.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param out Output key material buffer.
 * @param out_len Number of output bytes.
 * @return bool true when derivation succeeds.
 */
bool kex_hkdf(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
              size_t salt_len, const uint8_t *info, size_t info_len,
              uint8_t *out, size_t out_len);

#endif // KEX_H
```

## `include/partition.h`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent
// File:    partition.h
// Desc:    Declares the TELESCREEN four-partition model. The captured camera and
//          the RP5 replica share the same layout: boot, bootargs, kernel, rootfs,
//          with identical offsets, sizes, and magic bytes.
// Created: 2026

#ifndef PARTITION_H
#define PARTITION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Whole-flash image size in bytes (16 MiB).
 */
#define PART_IMAGE_SIZE 0x1000000u

/**
 * @brief Boot partition offset (first stage + U-Boot).
 */
#define PART_BOOT_OFF 0x000000u

/**
 * @brief Boot partition size in bytes (128 KiB).
 */
#define PART_BOOT_SIZE 0x020000u

/**
 * @brief Environment partition offset (U-Boot env).
 */
#define PART_ENV_OFF 0x020000u

/**
 * @brief Environment partition size in bytes (64 KiB).
 */
#define PART_ENV_SIZE 0x010000u

/**
 * @brief Kernel partition offset (vendor container).
 */
#define PART_KERNEL_OFF 0x030000u

/**
 * @brief Kernel partition size in bytes (1792 KiB).
 */
#define PART_KERNEL_SIZE 0x1C0000u

/**
 * @brief Rootfs partition offset (JFFS2).
 */
#define PART_ROOTFS_OFF 0x1F0000u

/**
 * @brief Rootfs partition size in bytes (14400 KiB).
 */
#define PART_ROOTFS_SIZE 0xE10000u

/**
 * @brief Boot partition magic: the ARM reset-vector word at offset 0.
 */
#define PART_BOOT_MAGIC 0xEA000515u

/**
 * @brief Vendor kernel container magic.
 */
#define PART_CONTAINER_MAGIC 0x001B8421u

/**
 * @brief JFFS2 little-endian magic as stored in the first two bytes.
 */
#define PART_JFFS2_MAGIC 0x1985u

/**
 * @brief Partition identity result codes.
 */
typedef enum part_result {
    PART_RESULT_OK = 0,
    PART_RESULT_BAD_ARGUMENT = 1,
    PART_RESULT_TOO_SMALL = 2,
    PART_RESULT_UNKNOWN = 3,
} part_result_t;

/**
 * @brief Identified partition kind.
 */
typedef enum part_kind {
    PART_KIND_UNKNOWN = 0,
    PART_KIND_BOOT = 1,
    PART_KIND_ENV = 2,
    PART_KIND_KERNEL = 3,
    PART_KIND_ROOTFS = 4,
} part_kind_t;

/**
 * @brief A partition descriptor (offset, size, name).
 */
typedef struct part_desc {
    part_kind_t kind;
    uint32_t offset;
    uint32_t size;
    const char *name;
} part_desc_t;

/**
 * @brief Return the descriptor for a partition kind.
 *
 * @param kind Partition kind.
 * @return const part_desc_t* Pointer to the static descriptor, or NULL.
 */
const part_desc_t *part_desc_for(part_kind_t kind);

/**
 * @brief Identify a partition by its magic bytes.
 *
 * Inspects the first bytes of a buffer and returns the partition kind. This is
 * how the layout is proven from the artifact rather than assumed.
 *
 * @param buf Pointer to the partition bytes.
 * @param len Number of readable bytes.
 * @return part_kind_t Identified kind, or PART_KIND_UNKNOWN.
 */
part_kind_t part_identify(const uint8_t *buf, size_t len);

/**
 * @brief Verify that a whole-flash image is large enough and internally consistent.
 *
 * Checks the image length against PART_IMAGE_SIZE and confirms each partition's
 * offset plus size stays within the image.
 *
 * @param image_size Size of the whole-flash image in bytes.
 * @return part_result_t Detailed verification outcome.
 */
part_result_t part_verify_layout(size_t image_size);

/**
 * @brief Copy one partition out of a whole-flash image.
 *
 * @param image Pointer to the whole-flash image.
 * @param image_size Size of the whole-flash image in bytes.
 * @param kind Partition kind to extract.
 * @param out Output buffer of at least the partition size.
 * @param out_size Size of the output buffer in bytes.
 * @return part_result_t Detailed extraction outcome.
 */
part_result_t part_carve(const uint8_t *image,
                         size_t image_size,
                         part_kind_t kind,
                         uint8_t *out,
                         size_t out_size);

#endif // PARTITION_H
```

## `include/teled.h`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    teled.h
// Desc:    Declares the TELESCREEN daemon. It owns the device identity, the
//          hardened session key, and the beacon sequence, and hands sealed
//          beacons to the local collector.
// Created: 2026

#ifndef TELED_H
#define TELED_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Maximum device UID length stored by the daemon.
 */
#define TELED_UID_MAX 32u

/**
 * @brief TELESCREEN daemon state.
 */
typedef struct teled_state {
    /**
     * @brief NUL-terminated device UID.
     */
    char uid[TELED_UID_MAX];
    /**
     * @brief Hardened session key.
     */
    uint8_t key[32];
    /**
     * @brief Monotonic beacon sequence number.
     */
    uint32_t seq;
    /**
     * @brief Number of beacons sealed by the daemon.
     */
    uint32_t beacons;
} teled_state_t;

/**
 * @brief Initialize the daemon with a device UID and session key.
 *
 * @param uid NUL-terminated public device identifier.
 * @param key Pointer to 32-byte hardened session key.
 * @return void
 */
void teled_init(const char *uid, const uint8_t key[32]);

/**
 * @brief Seal and deliver one beacon to the local collector.
 *
 * @param payload Pointer to AEAD payload bytes.
 * @return bool true when the beacon is sealed and accepted.
 */
bool teled_beacon(const uint8_t *payload);

/**
 * @brief Return a pointer to the daemon state snapshot.
 *
 * @param void No parameters.
 * @return const teled_state_t* Pointer to the static state.
 */
const teled_state_t *teled_state(void);

#endif // TELED_H
```

## `src/aead.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    aead.c
// Desc:    Implements the TELESCREEN authenticated-encryption API. AES-256-GCM
//          runs through the ARMv8 crypto extensions via OpenSSL EVP on the RP5.
//          XChaCha20-Poly1305 derives the subkey with a software HChaCha20 core
//          and seals with the IETF ChaCha20-Poly1305 construction.
// Created: 2026

#include "aead.h"
#include <string.h>
#include <openssl/evp.h>

/**
 * @brief Active backend selected by aead_init().
 */
static aead_algo_t g_aead_algo;

/**
 * @brief Non-zero once aead_init() has completed.
 */
static bool g_aead_ready;

/**
 * @brief Last nonce observed under the active key.
 */
static uint8_t g_last_nonce[AEAD_XCHACHA_NONCE_SIZE];

/**
 * @brief Non-zero once a nonce has been recorded.
 */
static bool g_have_last_nonce;

/**
 * @brief HChaCha20 constant words at the start of the state.
 */
static const uint32_t HCHACHA_CONST[4] = {
    0x61707865u, 0x3320646Eu, 0x79622D32u, 0x6B206574u,
};

/**
 * @brief Clear a byte buffer.
 *
 * @param buf Pointer to mutable byte buffer.
 * @param len Number of bytes to clear.
 * @return void
 */
static void clear_bytes(uint8_t *buf, size_t len) {
    size_t i;
    for (i = 0u; i < len; ++i) {
        buf[i] = 0u;
    }
}

/**
 * @brief Rotate a 32-bit value left.
 *
 * @param value Input 32-bit word.
 * @param shift Rotation distance in bits.
 * @return uint32_t Rotated result.
 */
static uint32_t rotl32(uint32_t value, uint8_t shift) {
    return (value << shift) | (value >> (32u - shift));
}

/**
 * @brief Load a 32-bit little-endian word from bytes.
 *
 * @param src Pointer to four readable bytes.
 * @return uint32_t Parsed 32-bit word.
 */
static uint32_t load32_le(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}

/**
 * @brief Store a 32-bit word in little-endian byte order.
 *
 * @param dst Pointer to four writable bytes.
 * @param value 32-bit word to serialize.
 * @return void
 */
static void store32_le(uint8_t *dst, uint32_t value) {
    dst[0] = (uint8_t)(value & 0xFFu);
    dst[1] = (uint8_t)((value >> 8u) & 0xFFu);
    dst[2] = (uint8_t)((value >> 16u) & 0xFFu);
    dst[3] = (uint8_t)((value >> 24u) & 0xFFu);
}

/**
 * @brief Execute one ChaCha quarter-round.
 *
 * @param a Pointer to state word a.
 * @param b Pointer to state word b.
 * @param c Pointer to state word c.
 * @param d Pointer to state word d.
 * @return void
 */
static void quarter_round(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    *a += *b; *d ^= *a; *d = rotl32(*d, 16u);
    *c += *d; *b ^= *c; *b = rotl32(*b, 12u);
    *a += *b; *d ^= *a; *d = rotl32(*d, 8u);
    *c += *d; *b ^= *c; *b = rotl32(*b, 7u);
}

/**
 * @brief Run the four column quarter-rounds of one HChaCha20 round.
 *
 * @param s Pointer to the 16-word state.
 * @return void
 */
static void hchacha_cols(uint32_t *s) {
    quarter_round(&s[0], &s[4], &s[8], &s[12]);
    quarter_round(&s[1], &s[5], &s[9], &s[13]);
    quarter_round(&s[2], &s[6], &s[10], &s[14]);
    quarter_round(&s[3], &s[7], &s[11], &s[15]);
}

/**
 * @brief Run the four diagonal quarter-rounds of one HChaCha20 round.
 *
 * @param s Pointer to the 16-word state.
 * @return void
 */
static void hchacha_diag(uint32_t *s) {
    quarter_round(&s[0], &s[5], &s[10], &s[15]);
    quarter_round(&s[1], &s[6], &s[11], &s[12]);
    quarter_round(&s[2], &s[7], &s[8], &s[13]);
    quarter_round(&s[3], &s[4], &s[9], &s[14]);
}

/**
 * @brief Run ten HChaCha20 double rounds over the state.
 *
 * @param s Pointer to the 16-word state.
 * @return void
 */
static void hchacha_rounds(uint32_t *s) {
    uint8_t r;
    for (r = 0u; r < 10u; ++r) {
        hchacha_cols(s);
        hchacha_diag(s);
    }
}

/**
 * @brief Build the initial HChaCha20 state from key and nonce.
 *
 * @param s Pointer to the 16-word state.
 * @param key Pointer to 32-byte key.
 * @param nonce Pointer to 24-byte nonce.
 * @return void
 */
static void hchacha_state(uint32_t *s, const uint8_t *key, const uint8_t *nonce) {
    uint8_t i;
    for (i = 0u; i < 4u; ++i) s[i] = HCHACHA_CONST[i];
    for (i = 0u; i < 8u; ++i) s[4u + i] = load32_le(&key[i * 4u]);
    for (i = 0u; i < 4u; ++i) s[12u + i] = load32_le(&nonce[i * 4u]);
}

/**
 * @brief Serialize the HChaCha20 subkey words into bytes.
 *
 * @param subkey Output 32-byte subkey buffer.
 * @param s Pointer to the 16-word state.
 * @return void
 */
static void hchacha_store(uint8_t *subkey, const uint32_t *s) {
    store32_le(&subkey[0], s[0]);
    store32_le(&subkey[4], s[1]);
    store32_le(&subkey[8], s[2]);
    store32_le(&subkey[12], s[3]);
    store32_le(&subkey[16], s[12]);
    store32_le(&subkey[20], s[13]);
    store32_le(&subkey[24], s[14]);
    store32_le(&subkey[28], s[15]);
}

/**
 * @brief Derive a 256-bit XChaCha20 subkey from key and nonce prefix.
 *
 * @param key Pointer to 32-byte AEAD key.
 * @param nonce Pointer to 24-byte XChaCha20 nonce.
 * @param subkey Output 32-byte subkey buffer.
 * @return void
 */
static void hchacha20(const uint8_t key[32], const uint8_t nonce[24],
                      uint8_t subkey[32]) {
    uint32_t state[16];
    hchacha_state(state, key, nonce);
    hchacha_rounds(state);
    hchacha_store(subkey, state);
}

/**
 * @brief Build the inner 96-bit nonce used by ChaCha20-Poly1305.
 *
 * @param nonce Pointer to 24-byte XChaCha20 nonce.
 * @param out Output 12-byte nonce buffer.
 * @return void
 */
static void build_inner_nonce(const uint8_t nonce[24], uint8_t out[12]) {
    memset(out, 0, 4u);
    memcpy(&out[4], &nonce[16], 8u);
}

/**
 * @brief Record the nonce and flag accidental reuse.
 *
 * @param nonce Pointer to backend nonce bytes.
 * @param len Nonce size in bytes.
 * @return bool true when the nonce was already seen, else false.
 */
static bool nonce_reused(const uint8_t *nonce, size_t len) {
    bool seen = g_have_last_nonce && (memcmp(g_last_nonce, nonce, len) == 0);
    memcpy(g_last_nonce, nonce, len);
    g_have_last_nonce = true;
    return seen;
}

/**
 * @brief Initialize an EVP context for AES-256-GCM encryption.
 *
 * @param ctx Pointer to the EVP context.
 * @param key Pointer to 32-byte AES key.
 * @param nonce Pointer to 12-byte GCM nonce.
 * @return int Non-zero when initialization succeeds.
 */
static int gcm_enc_init(EVP_CIPHER_CTX *ctx, const uint8_t *key,
                        const uint8_t *nonce) {
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) return 0;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN,
                            (int)AEAD_AES_NONCE_SIZE, NULL) != 1) return 0;
    return EVP_EncryptInit_ex(ctx, NULL, NULL, key, nonce) == 1;
}

/**
 * @brief Encrypt the payload into the sealed buffer with GCM.
 *
 * @param ctx Pointer to the EVP context.
 * @param sealed Pointer to the sealed output buffer.
 * @param plaintext Pointer to plaintext bytes.
 * @return int Non-zero when the update succeeds.
 */
static int gcm_enc_update(EVP_CIPHER_CTX *ctx, uint8_t *sealed,
                          const uint8_t *plaintext) {
    int len = 0;
    return EVP_EncryptUpdate(ctx, sealed, &len, plaintext,
                             (int)AEAD_PAYLOAD_SIZE) == 1;
}

/**
 * @brief Finalize GCM encryption and append the authentication tag.
 *
 * @param ctx Pointer to the EVP context.
 * @param sealed Pointer to the sealed output buffer.
 * @return int Non-zero when finalization and tag retrieval succeed.
 */
static int gcm_enc_final(EVP_CIPHER_CTX *ctx, uint8_t *sealed) {
    int len = 0;
    if (EVP_EncryptFinal_ex(ctx, &sealed[AEAD_PAYLOAD_SIZE], &len) != 1) return 0;
    return EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, (int)AEAD_TAG_SIZE,
                               &sealed[AEAD_PAYLOAD_SIZE]) == 1;
}

/**
 * @brief Initialize an EVP context for AES-256-GCM decryption.
 *
 * @param ctx Pointer to the EVP context.
 * @param key Pointer to 32-byte AES key.
 * @param nonce Pointer to 12-byte GCM nonce.
 * @return int Non-zero when initialization succeeds.
 */
static int gcm_dec_init(EVP_CIPHER_CTX *ctx, const uint8_t *key,
                        const uint8_t *nonce) {
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) return 0;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN,
                            (int)AEAD_AES_NONCE_SIZE, NULL) != 1) return 0;
    return EVP_DecryptInit_ex(ctx, NULL, NULL, key, nonce) == 1;
}

/**
 * @brief Decrypt the payload and verify the GCM tag.
 *
 * @param ctx Pointer to the EVP context.
 * @param plaintext_out Pointer to the plaintext output buffer.
 * @param sealed Pointer to the sealed input buffer.
 * @return int Non-zero when decryption and tag verification succeed.
 */
static int gcm_dec_final(EVP_CIPHER_CTX *ctx, uint8_t *plaintext_out,
                         const uint8_t *sealed) {
    int len = 0;
    int total = 0;
    if (EVP_DecryptUpdate(ctx, plaintext_out, &len, sealed,
                          (int)AEAD_PAYLOAD_SIZE) != 1) return 0;
    total = len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, (int)AEAD_TAG_SIZE,
                            (void *)&sealed[AEAD_PAYLOAD_SIZE]) != 1) return 0;
    return EVP_DecryptFinal_ex(ctx, &plaintext_out[total], &len) == 1;
}

/**
 * @brief Seal with AES-256-GCM through OpenSSL EVP.
 *
 * @param key Pointer to 32-byte AES key.
 * @param nonce Pointer to 12-byte GCM nonce.
 * @param plaintext Pointer to plaintext bytes.
 * @param sealed_out Output ciphertext then 16-byte tag.
 * @return aead_result_t Detailed outcome.
 */
static aead_result_t seal_aes_gcm(const uint8_t key[32],
                                  const uint8_t nonce[AEAD_AES_NONCE_SIZE],
                                  const uint8_t plaintext[AEAD_PAYLOAD_SIZE],
                                  uint8_t sealed_out[AEAD_SEALED_SIZE]) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    aead_result_t r = AEAD_RESULT_INTERNAL_ERROR;
    if (ctx == NULL) return AEAD_RESULT_INTERNAL_ERROR;
    if (gcm_enc_init(ctx, key, nonce) && gcm_enc_update(ctx, sealed_out, plaintext)) {
        if (gcm_enc_final(ctx, sealed_out)) r = AEAD_RESULT_SUCCESS;
    }
    EVP_CIPHER_CTX_free(ctx);
    return r;
}

/**
 * @brief Open with AES-256-GCM through OpenSSL EVP.
 *
 * @param key Pointer to 32-byte AES key.
 * @param nonce Pointer to 12-byte GCM nonce.
 * @param sealed Pointer to ciphertext then tag.
 * @param plaintext_out Output plaintext buffer.
 * @return aead_result_t Detailed outcome.
 */
static aead_result_t open_aes_gcm(const uint8_t key[32],
                                  const uint8_t nonce[AEAD_AES_NONCE_SIZE],
                                  const uint8_t sealed[AEAD_SEALED_SIZE],
                                  uint8_t plaintext_out[AEAD_PAYLOAD_SIZE]) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    aead_result_t r = AEAD_RESULT_AUTHENTICATION_FAILED;
    if (ctx == NULL) return AEAD_RESULT_INTERNAL_ERROR;
    if (gcm_dec_init(ctx, key, nonce) && gcm_dec_final(ctx, plaintext_out, sealed)) {
        r = AEAD_RESULT_SUCCESS;
    }
    EVP_CIPHER_CTX_free(ctx);
    if (r != AEAD_RESULT_SUCCESS) clear_bytes(plaintext_out, AEAD_PAYLOAD_SIZE);
    return r;
}

/**
 * @brief Initialize an EVP context for ChaCha20-Poly1305 encryption.
 *
 * @param ctx Pointer to the EVP context.
 * @param subkey Pointer to 32-byte subkey.
 * @param inner Pointer to 12-byte inner nonce.
 * @return int Non-zero when initialization succeeds.
 */
static int chacha_enc_init(EVP_CIPHER_CTX *ctx, const uint8_t *subkey,
                           const uint8_t *inner) {
    if (EVP_EncryptInit_ex(ctx, EVP_chacha20_poly1305(), NULL, NULL, NULL) != 1) return 0;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 12, NULL) != 1) return 0;
    return EVP_EncryptInit_ex(ctx, NULL, NULL, subkey, inner) == 1;
}

/**
 * @brief Encrypt the payload into the sealed buffer with ChaCha20-Poly1305.
 *
 * @param ctx Pointer to the EVP context.
 * @param sealed Pointer to the sealed output buffer.
 * @param plaintext Pointer to plaintext bytes.
 * @return int Non-zero when the update succeeds.
 */
static int chacha_enc_update(EVP_CIPHER_CTX *ctx, uint8_t *sealed,
                             const uint8_t *plaintext) {
    int len = 0;
    return EVP_EncryptUpdate(ctx, sealed, &len, plaintext,
                             (int)AEAD_PAYLOAD_SIZE) == 1;
}

/**
 * @brief Finalize ChaCha20-Poly1305 encryption and append the tag.
 *
 * @param ctx Pointer to the EVP context.
 * @param sealed Pointer to the sealed output buffer.
 * @return int Non-zero when finalization and tag retrieval succeed.
 */
static int chacha_enc_final(EVP_CIPHER_CTX *ctx, uint8_t *sealed) {
    int len = 0;
    if (EVP_EncryptFinal_ex(ctx, &sealed[AEAD_PAYLOAD_SIZE], &len) != 1) return 0;
    return EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, (int)AEAD_TAG_SIZE,
                               &sealed[AEAD_PAYLOAD_SIZE]) == 1;
}

/**
 * @brief Initialize an EVP context for ChaCha20-Poly1305 decryption.
 *
 * @param ctx Pointer to the EVP context.
 * @param subkey Pointer to 32-byte subkey.
 * @param inner Pointer to 12-byte inner nonce.
 * @return int Non-zero when initialization succeeds.
 */
static int chacha_dec_init(EVP_CIPHER_CTX *ctx, const uint8_t *subkey,
                           const uint8_t *inner) {
    if (EVP_DecryptInit_ex(ctx, EVP_chacha20_poly1305(), NULL, NULL, NULL) != 1) return 0;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 12, NULL) != 1) return 0;
    return EVP_DecryptInit_ex(ctx, NULL, NULL, subkey, inner) == 1;
}

/**
 * @brief Decrypt the payload and verify the Poly1305 tag.
 *
 * @param ctx Pointer to the EVP context.
 * @param plaintext_out Pointer to the plaintext output buffer.
 * @param sealed Pointer to the sealed input buffer.
 * @return int Non-zero when decryption and tag verification succeed.
 */
static int chacha_dec_final(EVP_CIPHER_CTX *ctx, uint8_t *plaintext_out,
                            const uint8_t *sealed) {
    int len = 0;
    int total = 0;
    if (EVP_DecryptUpdate(ctx, plaintext_out, &len, sealed,
                          (int)AEAD_PAYLOAD_SIZE) != 1) return 0;
    total = len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, (int)AEAD_TAG_SIZE,
                            (void *)&sealed[AEAD_PAYLOAD_SIZE]) != 1) return 0;
    return EVP_DecryptFinal_ex(ctx, &plaintext_out[total], &len) == 1;
}

/**
 * @brief Seal with XChaCha20-Poly1305.
 *
 * @param key Pointer to 32-byte ChaCha key.
 * @param nonce Pointer to 24-byte XChaCha nonce.
 * @param plaintext Pointer to plaintext bytes.
 * @param sealed_out Output ciphertext then 16-byte tag.
 * @return aead_result_t Detailed outcome.
 */
/**
 * @brief Wipe the derived subkey and inner nonce.
 *
 * @param subkey Pointer to 32-byte subkey buffer.
 * @param inner Pointer to 12-byte inner nonce buffer.
 * @return void
 */
static void xchacha_wipe(uint8_t *subkey, uint8_t *inner) {
    clear_bytes(subkey, 32u);
    clear_bytes(inner, 12u);
}

/**
 * @brief Encrypt one payload with the derived subkey and inner nonce.
 *
 * @param subkey Pointer to 32-byte subkey.
 * @param inner Pointer to 12-byte inner nonce.
 * @param plaintext Pointer to plaintext bytes.
 * @param sealed Pointer to the sealed output buffer.
 * @return aead_result_t Detailed outcome.
 */
static aead_result_t xchacha_encrypt(const uint8_t *subkey, const uint8_t *inner,
                                     const uint8_t *plaintext, uint8_t *sealed) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    aead_result_t r = AEAD_RESULT_INTERNAL_ERROR;
    if (ctx == NULL) return AEAD_RESULT_INTERNAL_ERROR;
    if (chacha_enc_init(ctx, subkey, inner) &&
        chacha_enc_update(ctx, sealed, plaintext)) {
        if (chacha_enc_final(ctx, sealed)) r = AEAD_RESULT_SUCCESS;
    }
    EVP_CIPHER_CTX_free(ctx);
    return r;
}

/**
 * @brief Decrypt one payload with the derived subkey and inner nonce.
 *
 * @param subkey Pointer to 32-byte subkey.
 * @param inner Pointer to 12-byte inner nonce.
 * @param sealed Pointer to the sealed input buffer.
 * @param plaintext Pointer to the plaintext output buffer.
 * @return aead_result_t Detailed outcome.
 */
static aead_result_t xchacha_decrypt(const uint8_t *subkey, const uint8_t *inner,
                                     const uint8_t *sealed, uint8_t *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    aead_result_t r = AEAD_RESULT_AUTHENTICATION_FAILED;
    if (ctx == NULL) return AEAD_RESULT_INTERNAL_ERROR;
    if (chacha_dec_init(ctx, subkey, inner) &&
        chacha_dec_final(ctx, plaintext, sealed)) r = AEAD_RESULT_SUCCESS;
    EVP_CIPHER_CTX_free(ctx);
    if (r != AEAD_RESULT_SUCCESS) clear_bytes(plaintext, AEAD_PAYLOAD_SIZE);
    return r;
}

/**
 * @brief Seal with XChaCha20-Poly1305.
 *
 * @param key Pointer to 32-byte ChaCha key.
 * @param nonce Pointer to 24-byte XChaCha nonce.
 * @param plaintext Pointer to plaintext bytes.
 * @param sealed_out Output ciphertext then 16-byte tag.
 * @return aead_result_t Detailed outcome.
 */
static aead_result_t seal_xchacha(const uint8_t key[32],
                                  const uint8_t nonce[AEAD_XCHACHA_NONCE_SIZE],
                                  const uint8_t plaintext[AEAD_PAYLOAD_SIZE],
                                  uint8_t sealed_out[AEAD_SEALED_SIZE]) {
    uint8_t subkey[32];
    uint8_t inner[12];
    aead_result_t r;
    hchacha20(key, nonce, subkey);
    build_inner_nonce(nonce, inner);
    r = xchacha_encrypt(subkey, inner, plaintext, sealed_out);
    xchacha_wipe(subkey, inner);
    return r;
}

/**
 * @brief Open with XChaCha20-Poly1305.
 *
 * @param key Pointer to 32-byte ChaCha key.
 * @param nonce Pointer to 24-byte XChaCha nonce.
 * @param sealed Pointer to ciphertext then tag.
 * @param plaintext_out Output plaintext buffer.
 * @return aead_result_t Detailed outcome.
 */
static aead_result_t open_xchacha(const uint8_t key[32],
                                  const uint8_t nonce[AEAD_XCHACHA_NONCE_SIZE],
                                  const uint8_t sealed[AEAD_SEALED_SIZE],
                                  uint8_t plaintext_out[AEAD_PAYLOAD_SIZE]) {
    uint8_t subkey[32];
    uint8_t inner[12];
    aead_result_t r;
    hchacha20(key, nonce, subkey);
    build_inner_nonce(nonce, inner);
    r = xchacha_decrypt(subkey, inner, sealed, plaintext_out);
    xchacha_wipe(subkey, inner);
    return r;
}

/**
 * @brief Initialize the AEAD subsystem.
 *
 * @param algo Backend selector.
 * @return bool true when initialization succeeds, else false.
 */
bool aead_init(aead_algo_t algo) {
    if ((algo != AEAD_ALGO_AES_256_GCM) && (algo != AEAD_ALGO_XCHACHA20_POLY1305)) {
        return false;
    }
    g_aead_algo = algo;
    g_have_last_nonce = false;
    memset(g_last_nonce, 0, sizeof(g_last_nonce));
    g_aead_ready = true;
    return true;
}

/**
 * @brief Return the nonce size required by the active backend.
 *
 * @param void No parameters.
 * @return size_t Nonce size in bytes, or 0 when not initialized.
 */
size_t aead_nonce_size(void) {
    if (!g_aead_ready) return 0u;
    if (g_aead_algo == AEAD_ALGO_AES_256_GCM) return AEAD_AES_NONCE_SIZE;
    return AEAD_XCHACHA_NONCE_SIZE;
}

/**
 * @brief Seal a plaintext payload into ciphertext plus tag.
 *
 * @param key Pointer to 32-byte AEAD key.
 * @param nonce Pointer to backend nonce bytes.
 * @param plaintext Pointer to AEAD_PAYLOAD_SIZE plaintext bytes.
 * @param sealed_out Output AEAD_SEALED_SIZE buffer.
 * @return aead_result_t Detailed sealing outcome.
 */
aead_result_t aead_seal(const uint8_t key[32], const uint8_t *nonce,
                        const uint8_t plaintext[AEAD_PAYLOAD_SIZE],
                        uint8_t sealed_out[AEAD_SEALED_SIZE]) {
    if (!g_aead_ready || (key == NULL) || (nonce == NULL) ||
        (plaintext == NULL) || (sealed_out == NULL)) {
        return AEAD_RESULT_BAD_ARGUMENT;
    }
    (void)nonce_reused(nonce, aead_nonce_size());
    if (g_aead_algo == AEAD_ALGO_AES_256_GCM) {
        return seal_aes_gcm(key, nonce, plaintext, sealed_out);
    }
    return seal_xchacha(key, nonce, plaintext, sealed_out);
}

/**
 * @brief Open a sealed payload, verifying the tag before releasing plaintext.
 *
 * @param key Pointer to 32-byte AEAD key.
 * @param nonce Pointer to backend nonce bytes.
 * @param sealed Pointer to AEAD_SEALED_SIZE sealed bytes.
 * @param plaintext_out Output AEAD_PAYLOAD_SIZE plaintext buffer.
 * @return aead_result_t Detailed opening outcome.
 */
aead_result_t aead_open(const uint8_t key[32], const uint8_t *nonce,
                        const uint8_t sealed[AEAD_SEALED_SIZE],
                        uint8_t plaintext_out[AEAD_PAYLOAD_SIZE]) {
    if (!g_aead_ready || (key == NULL) || (nonce == NULL) ||
        (sealed == NULL) || (plaintext_out == NULL)) {
        return AEAD_RESULT_BAD_ARGUMENT;
    }
    if (g_aead_algo == AEAD_ALGO_AES_256_GCM) {
        return open_aes_gcm(key, nonce, sealed, plaintext_out);
    }
    return open_xchacha(key, nonce, sealed, plaintext_out);
}
```

## `src/beacon.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    beacon.c
// Desc:    Implements the TELESCREEN exfiltration beacon: the deliberately weak
//          UID-derived key, the sequence nonce, and the hardened AEAD seal.
// Created: 2026

#include "beacon.h"
#include "aead.h"
#include "crc.h"
#include <string.h>

/**
 * @brief Derive the Ministry "sealed" key from the public device UID.
 *
 * @param uid NUL-terminated public device identifier.
 * @param out Output 32-byte key buffer.
 * @return void
 */
void beacon_weak_key(const char *uid, uint8_t out[BEACON_KEY_SIZE]) {
    size_t ulen = strlen(uid);
    uint32_t seed = crc32_le(0u, (const uint8_t *)uid, ulen);
    size_t i;
    for (i = 0u; i < BEACON_KEY_SIZE; ++i) {
        seed = crc32_le(seed, (const uint8_t *)uid, ulen);
        out[i] = (uint8_t)(seed & 0xFFu);
    }
}

/**
 * @brief Build the beacon nonce from the sequence number.
 *
 * @param seq Monotonic sequence number.
 * @param out Output nonce buffer of AEAD nonce size.
 * @return void
 */
void beacon_nonce(uint32_t seq, uint8_t *out) {
    out[0] = (uint8_t)(seq & 0xFFu);
    out[1] = (uint8_t)((seq >> 8u) & 0xFFu);
    out[2] = (uint8_t)((seq >> 16u) & 0xFFu);
    out[3] = (uint8_t)((seq >> 24u) & 0xFFu);
    memset(&out[4], 0, 20u);
}

/**
 * @brief Seal one beacon payload with the hardened AEAD key.
 *
 * @param key Pointer to 32-byte session key.
 * @param seq Monotonic sequence number.
 * @param plaintext Pointer to AEAD payload bytes.
 * @param sealed_out Output AEAD sealed buffer.
 * @return bool true when sealing succeeds.
 */
bool beacon_seal(const uint8_t key[BEACON_KEY_SIZE], uint32_t seq,
                 const uint8_t *plaintext, uint8_t *sealed_out) {
    uint8_t nonce[AEAD_XCHACHA_NONCE_SIZE];
    beacon_nonce(seq, nonce);
    return aead_seal(key, nonce, plaintext, sealed_out) == AEAD_RESULT_SUCCESS;
}
```

## `src/camera.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    camera.c
// Desc:    Implements the UVC camera URL helpers for the TELESCREEN lab.
// Created: 2026

#include "camera.h"
#include <stdio.h>

/**
 * @brief Format a URL and report whether it fit.
 *
 * @param out Output URL buffer.
 * @param out_len Size of the output buffer.
 * @param ip NUL-terminated device IPv4 address.
 * @param port TCP port.
 * @param scheme URL scheme ("rtsp" or "http").
 * @param path URL path.
 * @return bool true when the URL fits and is written.
 */
static bool camera_url(char *out, size_t out_len, const char *ip, uint16_t port,
                       const char *scheme, const char *path) {
    int n;
    if ((out == NULL) || (ip == NULL) || (out_len == 0u)) return false;
    n = snprintf(out, out_len, "%s://%s:%u%s", scheme, ip, (unsigned)port, path);
    return (n > 0) && ((size_t)n < out_len);
}

/**
 * @brief Build the RTSP URL a client (or VLC) connects to.
 *
 * @param ip NUL-terminated device IPv4 address.
 * @param port TCP port.
 * @param out Output URL buffer.
 * @param out_len Size of the output buffer.
 * @return bool true when the URL fits and is written.
 */
bool camera_rtsp_url(const char *ip, uint16_t port, char *out, size_t out_len) {
    return camera_url(out, out_len, ip, port, "rtsp", CAMERA_RTSP_PATH);
}

/**
 * @brief Build the MJPEG URL a browser connects to.
 *
 * @param ip NUL-terminated device IPv4 address.
 * @param port TCP port.
 * @param out Output URL buffer.
 * @param out_len Size of the output buffer.
 * @return bool true when the URL fits and is written.
 */
bool camera_mjpeg_url(const char *ip, uint16_t port, char *out, size_t out_len) {
    return camera_url(out, out_len, ip, port, "http", CAMERA_MJPEG_PATH);
}
```

## `src/collector.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    collector.c
// Desc:    Implements the local lab sink that receives TELESCREEN beacons.
// Created: 2026

#include "collector.h"
#include "aead.h"

/**
 * @brief Collector accounting state.
 */
static collector_stat_t g_stats;

/**
 * @brief Reset the collector accounting state.
 *
 * @param void No parameters.
 * @return void
 */
void collector_init(void) {
    g_stats.accepted = 0u;
    g_stats.rejected = 0u;
    g_stats.last_seq = 0u;
}

/**
 * @brief Accept or reject one beacon frame.
 *
 * @param sealed Pointer to the sealed frame bytes.
 * @param len Number of frame bytes.
 * @param seq Sequence number carried by the frame.
 * @return bool true when the frame is accepted.
 */
bool collector_accept(const uint8_t *sealed, size_t len, uint32_t seq) {
    if ((sealed == NULL) || (len != AEAD_SEALED_SIZE)) {
        g_stats.rejected += 1u;
        return false;
    }
    g_stats.accepted += 1u;
    g_stats.last_seq = seq;
    return true;
}

/**
 * @brief Return a pointer to the collector accounting snapshot.
 *
 * @param void No parameters.
 * @return const collector_stat_t* Pointer to the static snapshot.
 */
const collector_stat_t *collector_stats(void) {
    return &g_stats;
}
```

## `src/container.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    container.c
// Desc:    Implements the vendor kernel container helpers used to identify and
//          validate the kernel partition before inflation.
// Created: 2026

#include "container.h"
#include <string.h>

/**
 * @brief Read a 32-bit little-endian word from bytes.
 *
 * @param src Pointer to four readable bytes.
 * @return uint32_t Parsed 32-bit word.
 */
static uint32_t cont_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}

/**
 * @brief Read the container magic from the header.
 *
 * @param buf Pointer to the container header bytes.
 * @return uint32_t Parsed magic value.
 */
uint32_t container_magic(const uint8_t *buf) {
    return cont_read32(buf);
}

/**
 * @brief Read the container length field from the header.
 *
 * @param buf Pointer to the container header bytes.
 * @return uint32_t Parsed length value.
 */
uint32_t container_length(const uint8_t *buf) {
    return cont_read32(&buf[4]);
}

/**
 * @brief Verify the "gziphead" tag in the container header.
 *
 * @param buf Pointer to the container header bytes.
 * @return bool true when the tag is present.
 */
bool container_tag_ok(const uint8_t *buf) {
    return memcmp(&buf[8], "gziphead", 8u) == 0;
}

/**
 * @brief Validate a vendor kernel container header.
 *
 * @param buf Pointer to the container bytes.
 * @param len Number of readable bytes.
 * @return bool true when the magic and tag both validate.
 */
bool container_valid(const uint8_t *buf, size_t len) {
    if ((buf == NULL) || (len < CONTAINER_HDR_SIZE)) return false;
    return (container_magic(buf) == CONTAINER_MAGIC) && container_tag_ok(buf);
}
```

## `src/crc.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    crc.c
// Desc:    Implements the crc32_le, standard CRC32, and CRC-16/CCITT integrity
//          primitives used by the JFFS2 rootfs, the U-Boot environment, and
//          telemetry frames.
// Created: 2026

#include "crc.h"

/**
 * @brief Fold one byte into a reflected CRC-32 residual.
 *
 * The reflected representation uses the 0xEDB88320 polynomial, shifting right
 * and XORing the polynomial when the low bit is set.
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

/**
 * @brief Fold one byte into a CRC-16/CCITT-FALSE residual.
 *
 * @param crc Current running checksum.
 * @param byte Byte to fold in.
 * @return uint16_t Updated checksum.
 */
static uint16_t crc16_byte(uint16_t crc, uint8_t byte) {
    uint8_t i;
    uint16_t inbit;
    for (i = 0u; i < 8u; ++i) {
        inbit = (uint16_t)((crc ^ ((uint16_t)byte << 8u)) & 0x8000u);
        crc = (uint16_t)(crc << 1u);
        if (inbit != 0u) crc ^= 0x1021u;
        byte = (uint8_t)(byte << 1u);
    }
    return crc;
}

/**
 * @brief Compute the reflected crc32_le checksum used by JFFS2.
 *
 * @param seed Initial checksum value.
 * @param buf Pointer to readable bytes.
 * @param len Number of bytes to checksum.
 * @return uint32_t Computed reflected CRC-32 residual.
 */
uint32_t crc32_le(uint32_t seed, const uint8_t *buf, size_t len) {
    uint32_t crc = seed;
    size_t i;
    for (i = 0u; i < len; ++i) crc = crc32_byte(crc, buf[i]);
    return crc;
}

/**
 * @brief Compute the standard CRC32 used by the U-Boot environment.
 *
 * @param buf Pointer to readable bytes.
 * @param len Number of bytes to checksum.
 * @return uint32_t Computed standard CRC-32 value.
 */
uint32_t crc32_uboot(const uint8_t *buf, size_t len) {
    return crc32_le(0xFFFFFFFFu, buf, len) ^ 0xFFFFFFFFu;
}

/**
 * @brief Compute the CRC-16/CCITT-FALSE checksum of a byte buffer.
 *
 * @param buf Pointer to readable bytes.
 * @param len Number of bytes to checksum.
 * @return uint16_t Computed CRC-16 residual value.
 */
uint16_t crc16_ccitt(const uint8_t *buf, size_t len) {
    uint16_t crc = CRC16_INIT;
    size_t i;
    for (i = 0u; i < len; ++i) crc = crc16_byte(crc, buf[i]);
    return crc;
}
```

## `src/env.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    env.c
// Desc:    Implements the U-Boot environment helpers: CRC computation, CRC
//          verification, and key lookup.
// Created: 2026

#include "env.h"
#include "crc.h"
#include <string.h>

/**
 * @brief Read a 32-bit little-endian word from bytes.
 *
 * @param src Pointer to four readable bytes.
 * @return uint32_t Parsed 32-bit word.
 */
static uint32_t env_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}

/**
 * @brief Compute the environment CRC32 over the key/value blob.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @return uint32_t Computed standard CRC-32 over the blob.
 */
uint32_t env_compute_crc(const uint8_t *env, size_t len) {
    if ((env == NULL) || (len < ENV_CRC_SIZE)) return 0u;
    return crc32_uboot(&env[ENV_CRC_SIZE], len - ENV_CRC_SIZE);
}

/**
 * @brief Verify the leading environment CRC32.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @return bool true when the stored CRC matches the computed value.
 */
bool env_crc_valid(const uint8_t *env, size_t len) {
    if ((env == NULL) || (len < ENV_CRC_SIZE)) return false;
    return env_read32(env) == env_compute_crc(env, len);
}

/**
 * @brief Find a key in the environment blob and return its value.
 *
 * @param env Pointer to the environment buffer.
 * @param len Number of readable bytes.
 * @param key NUL-terminated key to locate.
 * @return const char* Pointer to the value, or NULL when absent.
 */
const char *env_find(const uint8_t *env, size_t len, const char *key) {
    size_t klen;
    size_t i;
    if ((env == NULL) || (key == NULL) || (len < ENV_CRC_SIZE)) return NULL;
    klen = strlen(key);
    for (i = ENV_CRC_SIZE; (i + klen + 1u) < len; ++i) {
        if ((env[i + klen] == '=') && (memcmp(&env[i], key, klen) == 0)) {
            return (const char *)&env[i + klen + 1u];
        }
    }
    return NULL;
}
```

## `src/identity.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    identity.c
// Desc:    Implements the TELESCREEN Ed25519 device identity.
// Created: 2026

#include "identity.h"
#include <openssl/evp.h>

/**
 * @brief Build an Ed25519 private key object from a raw seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @return EVP_PKEY* Private key object, or NULL.
 */
static EVP_PKEY *identity_priv(const uint8_t priv[32]) {
    return EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL, priv, 32u);
}

/**
 * @brief Build an Ed25519 public key object from raw bytes.
 *
 * @param pub Pointer to 32-byte public key.
 * @return EVP_PKEY* Public key object, or NULL.
 */
static EVP_PKEY *identity_pub(const uint8_t pub[32]) {
    return EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, NULL, pub, 32u);
}

/**
 * @brief Derive the Ed25519 public key from a private seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @param pub Output 32-byte public key.
 * @return bool true when derivation succeeds.
 */
bool identity_pubkey(const uint8_t priv[IDENTITY_KEY_SIZE],
                     uint8_t pub[IDENTITY_KEY_SIZE]) {
    EVP_PKEY *k = identity_priv(priv);
    size_t n = 32u;
    bool ok = (k != NULL) && (EVP_PKEY_get_raw_public_key(k, pub, &n) == 1);
    EVP_PKEY_free(k);
    return ok && (n == 32u);
}

/**
 * @brief Initialize an Ed25519 signing context.
 *
 * @param c Pointer to the message digest context.
 * @param k Pointer to the private key.
 * @return bool true when initialization succeeds.
 */
static bool ed_sign_init(EVP_MD_CTX *c, EVP_PKEY *k) {
    return EVP_DigestSignInit(c, NULL, NULL, NULL, k) == 1;
}

/**
 * @brief Sign a message with an Ed25519 private seed.
 *
 * @param priv Pointer to 32-byte private seed.
 * @param msg Pointer to message bytes.
 * @param msg_len Number of message bytes.
 * @param sig Output 64-byte signature.
 * @return bool true when signing succeeds.
 */
bool identity_sign(const uint8_t priv[IDENTITY_KEY_SIZE], const uint8_t *msg,
                   size_t msg_len, uint8_t sig[IDENTITY_SIG_SIZE]) {
    EVP_PKEY *k = identity_priv(priv);
    EVP_MD_CTX *c = EVP_MD_CTX_new();
    size_t n = 64u;
    bool ok = (k != NULL) && (c != NULL) && ed_sign_init(c, k) &&
              (EVP_DigestSign(c, sig, &n, msg, msg_len) == 1);
    EVP_MD_CTX_free(c);
    EVP_PKEY_free(k);
    return ok && (n == 64u);
}

/**
 * @brief Initialize an Ed25519 verification context.
 *
 * @param c Pointer to the message digest context.
 * @param k Pointer to the public key.
 * @return bool true when initialization succeeds.
 */
static bool ed_verify_init(EVP_MD_CTX *c, EVP_PKEY *k) {
    return EVP_DigestVerifyInit(c, NULL, NULL, NULL, k) == 1;
}

/**
 * @brief Verify an Ed25519 signature.
 *
 * @param pub Pointer to 32-byte public key.
 * @param msg Pointer to message bytes.
 * @param msg_len Number of message bytes.
 * @param sig Pointer to 64-byte signature.
 * @return bool true when the signature verifies.
 */
bool identity_verify(const uint8_t pub[IDENTITY_KEY_SIZE], const uint8_t *msg,
                     size_t msg_len, const uint8_t sig[IDENTITY_SIG_SIZE]) {
    EVP_PKEY *k = identity_pub(pub);
    EVP_MD_CTX *c = EVP_MD_CTX_new();
    bool ok = (k != NULL) && (c != NULL) && ed_verify_init(c, k) &&
              (EVP_DigestVerify(c, sig, 64u, msg, msg_len) == 1);
    EVP_MD_CTX_free(c);
    EVP_PKEY_free(k);
    return ok;
}
```

## `src/jffs2.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    jffs2.c
// Desc:    Implements the JFFS2 node header helpers for the TELESCREEN rootfs.
// Created: 2026

#include "jffs2.h"
#include "crc.h"

/**
 * @brief Read a 16-bit little-endian value from bytes.
 *
 * @param src Pointer to two readable bytes.
 * @return uint16_t Parsed 16-bit value.
 */
static uint16_t jffs2_read16(const uint8_t *src) {
    return (uint16_t)((uint16_t)src[0] | ((uint16_t)src[1] << 8u));
}

/**
 * @brief Read a 32-bit little-endian word from bytes.
 *
 * @param src Pointer to four readable bytes.
 * @return uint32_t Parsed 32-bit word.
 */
static uint32_t jffs2_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}

/**
 * @brief Read the little-endian magic from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint16_t Parsed magic value.
 */
uint16_t jffs2_magic(const uint8_t *node) {
    return jffs2_read16(node);
}

/**
 * @brief Read the little-endian node type from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint16_t Parsed node type.
 */
uint16_t jffs2_type(const uint8_t *node) {
    return jffs2_read16(&node[2]);
}

/**
 * @brief Read the little-endian total length from a node header.
 *
 * @param node Pointer to the node header bytes.
 * @return uint32_t Parsed total length in bytes.
 */
uint32_t jffs2_totlen(const uint8_t *node) {
    return jffs2_read32(&node[4]);
}

/**
 * @brief Verify the crc32_le header CRC of a node.
 *
 * @param node Pointer to the node header bytes.
 * @return bool true when the stored header CRC matches the computed value.
 */
bool jffs2_hdr_crc_valid(const uint8_t *node) {
    return crc32_le(0u, node, 8u) == jffs2_read32(&node[8]);
}
```

## `src/kex.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    kex.c
// Desc:    Implements X25519 key agreement and HKDF-SHA256 key derivation for the
//          hardened TELESCREEN session key.
// Created: 2026

#include "kex.h"
#include <string.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

/**
 * @brief Compute one HMAC-SHA256 over a byte range.
 *
 * @param key Pointer to HMAC key bytes.
 * @param key_len Number of key bytes.
 * @param data Pointer to data bytes.
 * @param data_len Number of data bytes.
 * @param out Output 32-byte MAC buffer.
 * @return bool true when the MAC succeeds.
 */
static bool hkdf_hmac(const uint8_t *key, size_t key_len, const uint8_t *data,
                      size_t data_len, uint8_t out[32]) {
    unsigned int n = 0u;
    return HMAC(EVP_sha256(), key, (int)key_len, data, data_len, out, &n) != NULL;
}

/**
 * @brief Assemble one HKDF expand input block.
 *
 * @param buf Output assembly buffer.
 * @param tprev Pointer to the previous T block.
 * @param tlen Number of previous T bytes.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param counter One-based block counter byte.
 * @return size_t Assembled block length.
 */
static size_t hkdf_assemble(uint8_t *buf, const uint8_t *tprev, size_t tlen,
                            const uint8_t *info, size_t info_len, uint8_t counter) {
    memcpy(buf, tprev, tlen);
    memcpy(&buf[tlen], info, info_len);
    buf[tlen + info_len] = counter;
    return tlen + info_len + 1u;
}

/**
 * @brief Compute one HKDF expand T block.
 *
 * @param prk Pointer to the 32-byte pseudorandom key.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param tprev Pointer to the previous T block.
 * @param tlen Number of previous T bytes.
 * @param counter One-based block counter byte.
 * @param out Output 32-byte T block buffer.
 * @return size_t Number of bytes produced, or zero on failure.
 */
static size_t hkdf_block(const uint8_t prk[32], const uint8_t *info, size_t info_len,
                         const uint8_t *tprev, size_t tlen, uint8_t counter,
                         uint8_t out[32]) {
    uint8_t buf[97];
    size_t blen = hkdf_assemble(buf, tprev, tlen, info, info_len, counter);
    if (!hkdf_hmac(prk, 32u, buf, blen, out)) return 0u;
    return 32u;
}

/**
 * @brief Produce and copy one HKDF expand block into the output.
 *
 * @param prk Pointer to the 32-byte pseudorandom key.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param t Pointer to the mutable T block buffer.
 * @param tlen Pointer to the current T block length.
 * @param counter One-based block counter byte.
 * @param out Output key material buffer.
 * @param done Pointer to the number of output bytes written.
 * @param out_len Total number of output bytes.
 * @return bool true when the step succeeds.
 */
static bool hkdf_step(const uint8_t prk[32], const uint8_t *info, size_t info_len,
                      uint8_t *t, size_t *tlen, uint8_t counter, uint8_t *out,
                      size_t *done, size_t out_len) {
    size_t n = hkdf_block(prk, info, info_len, t, *tlen, counter, t);
    size_t copy;
    if (n == 0u) return false;
    *tlen = n;
    copy = (out_len - *done < n) ? (out_len - *done) : n;
    memcpy(&out[*done], t, copy);
    *done += copy;
    return true;
}

/**
 * @brief Run the HKDF expand phase.
 *
 * @param prk Pointer to the 32-byte pseudorandom key.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param out Output key material buffer.
 * @param out_len Number of output bytes.
 * @return bool true when expansion succeeds.
 */
static bool hkdf_expand(const uint8_t prk[32], const uint8_t *info, size_t info_len,
                        uint8_t *out, size_t out_len) {
    uint8_t t[32];
    size_t done = 0u;
    size_t tlen = 0u;
    uint8_t counter = 1u;
    while (done < out_len) {
        if (!hkdf_step(prk, info, info_len, t, &tlen, counter, out, &done, out_len)) return false;
        counter += 1u;
    }
    return true;
}

/**
 * @brief Derive key material with HKDF-SHA256.
 *
 * @param ikm Pointer to input key material.
 * @param ikm_len Number of input key material bytes.
 * @param salt Pointer to salt bytes.
 * @param salt_len Number of salt bytes.
 * @param info Pointer to context info bytes.
 * @param info_len Number of info bytes.
 * @param out Output key material buffer.
 * @param out_len Number of output bytes.
 * @return bool true when derivation succeeds.
 */
bool kex_hkdf(const uint8_t *ikm, size_t ikm_len, const uint8_t *salt,
              size_t salt_len, const uint8_t *info, size_t info_len,
              uint8_t *out, size_t out_len) {
    uint8_t prk[32];
    if (!hkdf_hmac(salt, salt_len, ikm, ikm_len, prk)) return false;
    return hkdf_expand(prk, info, info_len, out, out_len);
}

/**
 * @brief Drive an X25519 EVP context to a shared secret.
 *
 * @param c Pointer to the EVP key context.
 * @param b Pointer to the peer public key.
 * @param out Output shared-secret buffer.
 * @param n Pointer to the output length.
 * @return bool true when the derivation succeeds.
 */
static bool x25519_derive(EVP_PKEY_CTX *c, EVP_PKEY *b, uint8_t *out, size_t *n) {
    if (c == NULL) return false;
    if (EVP_PKEY_derive_init(c) != 1) return false;
    if (EVP_PKEY_derive_set_peer(c, b) != 1) return false;
    return EVP_PKEY_derive(c, out, n) == 1;
}

/**
 * @brief Release the X25519 EVP objects.
 *
 * @param c Pointer to the EVP key context.
 * @param a Pointer to the private key.
 * @param b Pointer to the public key.
 * @return void
 */
static void kex_free(EVP_PKEY_CTX *c, EVP_PKEY *a, EVP_PKEY *b) {
    EVP_PKEY_CTX_free(c);
    EVP_PKEY_free(a);
    EVP_PKEY_free(b);
}

/**
 * @brief Compute the X25519 shared secret.
 *
 * @param priv Pointer to 32-byte private scalar.
 * @param peer Pointer to 32-byte peer public key.
 * @param out Output 32-byte shared secret.
 * @return bool true when the derivation succeeds.
 */
bool kex_x25519(const uint8_t priv[KEX_KEY_SIZE], const uint8_t peer[KEX_KEY_SIZE],
                uint8_t out[KEX_KEY_SIZE]) {
    EVP_PKEY *a = EVP_PKEY_new_raw_private_key(EVP_PKEY_X25519, NULL, priv, 32u);
    EVP_PKEY *b = EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, NULL, peer, 32u);
    EVP_PKEY_CTX *c = ((a != NULL) && (b != NULL)) ? EVP_PKEY_CTX_new(a, NULL) : NULL;
    size_t n = 32u;
    bool ok = x25519_derive(c, b, out, &n);
    kex_free(c, a, b);
    return ok && (n == 32u);
}
```

## `src/main.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    main.c
// Desc:    TELESCREEN lab entry point. Initializes the hardened AEAD backend and
//          the daemon, then emits one sealed beacon to the local collector.
// Created: 2026

#include "aead.h"
#include "teled.h"
#include <string.h>

/**
 * @brief Lab entry point: emit one hardened beacon.
 *
 * @param void No parameters.
 * @return int Zero on success, non-zero on failure.
 */
int main(void) {
    uint8_t key[32];
    uint8_t payload[AEAD_PAYLOAD_SIZE];
    memset(key, 0, sizeof(key));
    memset(payload, 0, sizeof(payload));
    aead_init(AEAD_ALGO_AES_256_GCM);
    teled_init("SSAT-468547-FEEBD", key);
    return teled_beacon(payload) ? 0 : 1;
}
```

## `src/partition.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    partition.c
// Desc:    Implements the TELESCREEN four-partition model: descriptors, magic
//          identification, layout verification, and carving.
// Created: 2026

#include "partition.h"
#include <string.h>

/**
 * @brief The four partitions, in offset order.
 *
 * Exactly as the captured device declares them in mtdparts. The RP5 replica
 * uses the same table.
 */
static const part_desc_t g_parts[4] = {
    { PART_KIND_BOOT,   PART_BOOT_OFF,   PART_BOOT_SIZE,   "boot" },
    { PART_KIND_ENV,    PART_ENV_OFF,    PART_ENV_SIZE,    "bootargs" },
    { PART_KIND_KERNEL, PART_KERNEL_OFF, PART_KERNEL_SIZE, "kernel" },
    { PART_KIND_ROOTFS, PART_ROOTFS_OFF, PART_ROOTFS_SIZE, "rootfs" },
};

/**
 * @brief Read a 32-bit little-endian word from bytes.
 *
 * @param src Pointer to four readable bytes.
 * @return uint32_t Parsed 32-bit word.
 */
static uint32_t part_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}

/**
 * @brief Read a 16-bit little-endian value from bytes.
 *
 * @param src Pointer to two readable bytes.
 * @return uint16_t Parsed 16-bit value.
 */
static uint16_t part_read16(const uint8_t *src) {
    return (uint16_t)((uint16_t)src[0] | ((uint16_t)src[1] << 8u));
}

/**
 * @brief Return the end offset of partition slot i.
 *
 * @param i Partition table index.
 * @return size_t End offset in bytes.
 */
static size_t part_end(size_t i) {
    return (size_t)g_parts[i].offset + (size_t)g_parts[i].size;
}

/**
 * @brief Return the descriptor for a partition kind.
 *
 * @param kind Partition kind.
 * @return const part_desc_t* Pointer to the static descriptor, or NULL.
 */
const part_desc_t *part_desc_for(part_kind_t kind) {
    size_t i;
    for (i = 0u; i < 4u; ++i) {
        if (g_parts[i].kind == kind) return &g_parts[i];
    }
    return NULL;
}

/**
 * @brief Identify a partition by its magic bytes.
 *
 * @param buf Pointer to the partition bytes.
 * @param len Number of readable bytes.
 * @return part_kind_t Identified kind, or PART_KIND_UNKNOWN.
 */
part_kind_t part_identify(const uint8_t *buf, size_t len) {
    if ((buf == NULL) || (len < 4u)) return PART_KIND_UNKNOWN;
    if (part_read32(buf) == PART_BOOT_MAGIC) return PART_KIND_BOOT;
    if (part_read16(buf) == PART_JFFS2_MAGIC) return PART_KIND_ROOTFS;
    if (part_read32(buf) == PART_CONTAINER_MAGIC) return PART_KIND_KERNEL;
    return PART_KIND_ENV;
}

/**
 * @brief Verify that a whole-flash image is large enough and consistent.
 *
 * @param image_size Size of the whole-flash image in bytes.
 * @return part_result_t Detailed verification outcome.
 */
part_result_t part_verify_layout(size_t image_size) {
    size_t i;
    if (image_size < PART_IMAGE_SIZE) return PART_RESULT_TOO_SMALL;
    for (i = 0u; i < 4u; ++i) {
        if (part_end(i) > PART_IMAGE_SIZE) return PART_RESULT_UNKNOWN;
    }
    return PART_RESULT_OK;
}

/**
 * @brief Copy one partition out of a whole-flash image.
 *
 * @param image Pointer to the whole-flash image.
 * @param image_size Size of the whole-flash image in bytes.
 * @param kind Partition kind to extract.
 * @param out Output buffer of at least the partition size.
 * @param out_size Size of the output buffer in bytes.
 * @return part_result_t Detailed extraction outcome.
 */
part_result_t part_carve(const uint8_t *image, size_t image_size, part_kind_t kind,
                         uint8_t *out, size_t out_size) {
    const part_desc_t *desc = part_desc_for(kind);
    if ((image == NULL) || (out == NULL)) return PART_RESULT_BAD_ARGUMENT;
    if (image_size < PART_IMAGE_SIZE) return PART_RESULT_TOO_SMALL;
    if (desc == NULL) return PART_RESULT_BAD_ARGUMENT;
    if (out_size < desc->size) return PART_RESULT_TOO_SMALL;
    memcpy(out, &image[desc->offset], desc->size);
    return PART_RESULT_OK;
}
```

## `src/teled.c`

```c
// MIT License
//
// Copyright (c) 2026 Kevin Thomas
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author:  Kevin Thomas
// Email:   kevin@mytechnotalent.com
// GitHub:  https://github.com/mytechnotalent/telescreen-c-rp5
// File:    teled.c
// Desc:    Implements the TELESCREEN daemon: identity, session key, beacon
//          sequence, and delivery to the local collector.
// Created: 2026

#include "teled.h"
#include "aead.h"
#include "beacon.h"
#include "collector.h"
#include <string.h>

/**
 * @brief TELESCREEN daemon state.
 */
static teled_state_t g_teled;

/**
 * @brief Initialize the daemon with a device UID and session key.
 *
 * @param uid NUL-terminated public device identifier.
 * @param key Pointer to 32-byte hardened session key.
 * @return void
 */
void teled_init(const char *uid, const uint8_t key[32]) {
    memset(&g_teled, 0, sizeof(g_teled));
    strncpy(g_teled.uid, uid, sizeof(g_teled.uid) - 1u);
    memcpy(g_teled.key, key, 32u);
    collector_init();
}

/**
 * @brief Seal and deliver one beacon to the local collector.
 *
 * @param payload Pointer to AEAD payload bytes.
 * @return bool true when the beacon is sealed and accepted.
 */
bool teled_beacon(const uint8_t *payload) {
    uint8_t sealed[AEAD_SEALED_SIZE];
    g_teled.seq += 1u;
    if (!beacon_seal(g_teled.key, g_teled.seq, payload, sealed)) return false;
    g_teled.beacons += 1u;
    return collector_accept(sealed, sizeof(sealed), g_teled.seq);
}

/**
 * @brief Return a pointer to the daemon state snapshot.
 *
 * @param void No parameters.
 * @return const teled_state_t* Pointer to the static state.
 */
const teled_state_t *teled_state(void) {
    return &g_teled;
}
```
