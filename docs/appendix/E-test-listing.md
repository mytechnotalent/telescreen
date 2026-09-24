# Appendix E: Test Suite Listing

The native and Python test suites, verbatim.

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

## `test/test_telescreen.c`

```
/**
 * FILE: test_telescreen.c
 *
 * DESCRIPTION:
 * Comprehensive test suite for the TELESCREEN RP5 teaching lab: the AEAD
 * authenticated-encryption API (AES-256-GCM and XChaCha20-Poly1305) and the
 * four-partition model (descriptors, magic identification, layout verification,
 * and carving).
 *
 * BRIEF:
 * Native unit test runner for telescreen-c-rp5.
 *
 * AUTHOR: Kevin Thomas
 * DATE: September 2026
 */

#include "harness.h"
#include "aead.h"
#include "partition.h"
#include "crc.h"
#include "env.h"
#include "jffs2.h"
#include "container.h"
#include "beacon.h"
#include "kex.h"
#include "identity.h"
#include "collector.h"
#include "teled.h"
#include "camera.h"
#include <string.h>

#include "../src/aead.c"
#include "../src/partition.c"
#include "../src/crc.c"
#include "../src/env.c"
#include "../src/jffs2.c"
#include "../src/container.c"
#include "../src/beacon.c"
#include "../src/kex.c"
#include "../src/identity.c"
#include "../src/collector.c"
#include "../src/teled.c"
#include "../src/camera.c"

/**
 * @brief Whole-flash image buffer used by the partition tests.
 */
static uint8_t g_image[PART_IMAGE_SIZE];

/**
 * @brief Shared 32-byte AEAD key for the crypto tests.
 */
static uint8_t g_key[32];

/**
 * @brief Shared 24-byte nonce buffer for the crypto tests.
 */
static uint8_t g_nonce[AEAD_XCHACHA_NONCE_SIZE];

/**
 * @brief Shared plaintext payload buffer for the crypto tests.
 */
static uint8_t g_pt[AEAD_PAYLOAD_SIZE];

/**
 * @brief Shared sealed output buffer for the crypto tests.
 */
static uint8_t g_sealed[AEAD_SEALED_SIZE];

/**
 * @brief Write the ARM reset-vector magic into the boot partition.
 *
 * @param void No parameters.
 * @return void
 */
static void set_boot_magic(void) {
    g_image[PART_BOOT_OFF + 0u] = 0x15u;
    g_image[PART_BOOT_OFF + 1u] = 0x05u;
    g_image[PART_BOOT_OFF + 2u] = 0x00u;
    g_image[PART_BOOT_OFF + 3u] = 0xEAu;
}

/**
 * @brief Write the vendor container magic into the kernel partition.
 *
 * @param void No parameters.
 * @return void
 */
static void set_kernel_magic(void) {
    g_image[PART_KERNEL_OFF + 0u] = 0x21u;
    g_image[PART_KERNEL_OFF + 1u] = 0x84u;
    g_image[PART_KERNEL_OFF + 2u] = 0x1Bu;
    g_image[PART_KERNEL_OFF + 3u] = 0x00u;
}

/**
 * @brief Write the JFFS2 magic into the rootfs partition.
 *
 * @param void No parameters.
 * @return void
 */
static void set_rootfs_magic(void) {
    g_image[PART_ROOTFS_OFF + 0u] = 0x85u;
    g_image[PART_ROOTFS_OFF + 1u] = 0x19u;
    g_image[PART_ROOTFS_OFF + 2u] = 0x03u;
    g_image[PART_ROOTFS_OFF + 3u] = 0x20u;
}

/**
 * @brief Test setup hook executed before each test case.
 *
 * @param void No parameters.
 * @return void
 */
void setUp(void) {
    memset(g_image, 0, sizeof(g_image));
    set_boot_magic();
    set_kernel_magic();
    set_rootfs_magic();
}

/**
 * @brief Test teardown hook executed after each test case.
 *
 * @param void No parameters.
 * @return void
 */
void tearDown(void) {
}

/**
 * @brief Confirm the nonce size is zero before initialization.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_nonce_size_before_init(void) {
    TEST_ASSERT_EQUAL_UINT(0u, aead_nonce_size());
}

/**
 * @brief Confirm initialization rejects an unknown backend.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_init_rejects_bad_algo(void) {
    TEST_ASSERT_FALSE(aead_init((aead_algo_t)99));
}

/**
 * @brief Confirm the AES backend initializes with a 12-byte nonce.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_init_aes(void) {
    TEST_ASSERT_TRUE(aead_init(AEAD_ALGO_AES_256_GCM));
    TEST_ASSERT_EQUAL_UINT(AEAD_AES_NONCE_SIZE, aead_nonce_size());
}

/**
 * @brief Confirm sealing rejects each null argument.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_seal_bad_args(void) {
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_seal(NULL, g_nonce, g_pt, g_sealed));
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_seal(g_key, NULL, g_pt, g_sealed));
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_seal(g_key, g_nonce, NULL, g_sealed));
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_seal(g_key, g_nonce, g_pt, NULL));
}

/**
 * @brief Confirm opening rejects each null argument.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_open_bad_args(void) {
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_open(NULL, g_nonce, g_sealed, g_pt));
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_open(g_key, NULL, g_sealed, g_pt));
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_open(g_key, g_nonce, NULL, g_pt));
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_BAD_ARGUMENT, aead_open(g_key, g_nonce, g_sealed, NULL));
}

/**
 * @brief Confirm an AES-256-GCM round trip recovers the plaintext.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_seal_open_aes(void) {
    uint8_t out[AEAD_PAYLOAD_SIZE];
    aead_init(AEAD_ALGO_AES_256_GCM);
    memset(g_nonce, 0x11, AEAD_AES_NONCE_SIZE);
    memset(g_pt, 0xA5, sizeof(g_pt));
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_SUCCESS, aead_seal(g_key, g_nonce, g_pt, g_sealed));
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_SUCCESS, aead_open(g_key, g_nonce, g_sealed, out));
    TEST_ASSERT_EQUAL_MEMORY(g_pt, out, sizeof(g_pt));
}

/**
 * @brief Confirm a corrupted AES-GCM tag is rejected.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_open_aes_bad_tag(void) {
    uint8_t out[AEAD_PAYLOAD_SIZE];
    aead_init(AEAD_ALGO_AES_256_GCM);
    memset(g_nonce, 0x22, AEAD_AES_NONCE_SIZE);
    aead_seal(g_key, g_nonce, g_pt, g_sealed);
    g_sealed[0] ^= 0xFFu;
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_AUTHENTICATION_FAILED,
                          aead_open(g_key, g_nonce, g_sealed, out));
}

/**
 * @brief Confirm sealing twice with one nonce is detected as reuse.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_nonce_reuse(void) {
    aead_init(AEAD_ALGO_AES_256_GCM);
    memset(g_nonce, 0x33, AEAD_AES_NONCE_SIZE);
    aead_seal(g_key, g_nonce, g_pt, g_sealed);
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_SUCCESS, aead_seal(g_key, g_nonce, g_pt, g_sealed));
}

/**
 * @brief Confirm the XChaCha20 backend initializes with a 24-byte nonce.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_init_xchacha(void) {
    TEST_ASSERT_TRUE(aead_init(AEAD_ALGO_XCHACHA20_POLY1305));
    TEST_ASSERT_EQUAL_UINT(AEAD_XCHACHA_NONCE_SIZE, aead_nonce_size());
}

/**
 * @brief Confirm an XChaCha20-Poly1305 round trip recovers the plaintext.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_seal_open_xchacha(void) {
    uint8_t out[AEAD_PAYLOAD_SIZE];
    aead_init(AEAD_ALGO_XCHACHA20_POLY1305);
    memset(g_nonce, 0x44, AEAD_XCHACHA_NONCE_SIZE);
    memset(g_pt, 0x5A, sizeof(g_pt));
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_SUCCESS, aead_seal(g_key, g_nonce, g_pt, g_sealed));
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_SUCCESS, aead_open(g_key, g_nonce, g_sealed, out));
    TEST_ASSERT_EQUAL_MEMORY(g_pt, out, sizeof(g_pt));
}

/**
 * @brief Confirm a corrupted XChaCha20 tag is rejected.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_open_xchacha_bad_tag(void) {
    uint8_t out[AEAD_PAYLOAD_SIZE];
    aead_init(AEAD_ALGO_XCHACHA20_POLY1305);
    memset(g_nonce, 0x55, AEAD_XCHACHA_NONCE_SIZE);
    aead_seal(g_key, g_nonce, g_pt, g_sealed);
    g_sealed[1] ^= 0xFFu;
    TEST_ASSERT_EQUAL_INT(AEAD_RESULT_AUTHENTICATION_FAILED,
                          aead_open(g_key, g_nonce, g_sealed, out));
}

/**
 * @brief Confirm the boot descriptor reports the locked offset.
 *
 * @param void No parameters.
 * @return void
 */
static void test_part_desc_for_boot(void) {
    const part_desc_t *d = part_desc_for(PART_KIND_BOOT);
    TEST_ASSERT_TRUE(d != NULL);
    TEST_ASSERT_EQUAL_UINT(PART_BOOT_OFF, d->offset);
    TEST_ASSERT_EQUAL_STRING("boot", d->name);
}

/**
 * @brief Confirm an unknown descriptor kind returns NULL.
 *
 * @param void No parameters.
 * @return void
 */
static void test_part_desc_for_unknown(void) {
    TEST_ASSERT_TRUE(part_desc_for((part_kind_t)99) == NULL);
}

/**
 * @brief Confirm the boot partition is identified by its vectors.
 *
 * @param void No parameters.
 * @return void
 */
static void test_part_identify_boot(void) {
    TEST_ASSERT_EQUAL_INT(PART_KIND_BOOT, part_identify(&g_image[PART_BOOT_OFF], 4u));
}

/**
 * @brief Confirm the kernel partition is identified by its container magic.
 *
 * @param void No parameters.
 * @return void
 */
static void test_part_identify_kernel(void) {
    TEST_ASSERT_EQUAL_INT(PART_KIND_KERNEL, part_identify(&g_image[PART_KERNEL_OFF], 4u));
}

/**
 * @brief Confirm the rootfs partition is identified by its JFFS2 magic.
 *
 * @param void No parameters.
 * @return void
 */
static void test_part_identify_rootfs(void) {
    TEST_ASSERT_EQUAL_INT(PART_KIND_ROOTFS, part_identify(&g_image[PART_ROOTFS_OFF], 4u));
}

/**
 * @brief Confirm the environment partition is the fallback identity.
 *
 * @param void No parameters.
 * @return void
 */
static void test_part_identify_env(void) {
    TEST_ASSERT_EQUAL_INT(PART_KIND_ENV, part_identify(&g_image[PART_ENV_OFF], 4u));
}

/**
 * @brief Confirm identification rejects null and short buffers.
 *
 * @param void No parameters.
 * @return void
 */
static void test_part_identify_unknown(void) {
    TEST_ASSERT_EQUAL_INT(PART_KIND_UNKNOWN, part_identify(NULL, 4u));
    TEST_ASSERT_EQUAL_INT(PART_KIND_UNKNOWN, part_identify(g_image, 2u));
}

/**
 * @brief Confirm layout verification accepts full and rejects short images.
 *
 * @param void No parameters.
 * @return void
 */
static void test_part_verify_layout(void) {
    TEST_ASSERT_EQUAL_INT(PART_RESULT_OK, part_verify_layout(PART_IMAGE_SIZE));
    TEST_ASSERT_EQUAL_INT(PART_RESULT_TOO_SMALL, part_verify_layout(16u));
}

/**
 * @brief Confirm carving copies the boot partition bytes.
 *
 * @param void No parameters.
 * @return void
 */
static void test_part_carve_ok(void) {
    uint8_t out[PART_BOOT_SIZE];
    TEST_ASSERT_EQUAL_INT(PART_RESULT_OK,
                          part_carve(g_image, sizeof(g_image), PART_KIND_BOOT,
                                     out, sizeof(out)));
    TEST_ASSERT_EQUAL_UINT(0x15u, out[0]);
}

/**
 * @brief Confirm carving rejects null image and output pointers.
 *
 * @param void No parameters.
 * @return void
 */
static void test_part_carve_bad_args(void) {
    uint8_t out[PART_BOOT_SIZE];
    TEST_ASSERT_EQUAL_INT(PART_RESULT_BAD_ARGUMENT,
                          part_carve(NULL, sizeof(g_image), PART_KIND_BOOT, out, sizeof(out)));
    TEST_ASSERT_EQUAL_INT(PART_RESULT_BAD_ARGUMENT,
                          part_carve(g_image, sizeof(g_image), PART_KIND_BOOT, NULL, sizeof(out)));
}

/**
 * @brief Confirm carving rejects short images and short outputs.
 *
 * @param void No parameters.
 * @return void
 */
static void test_part_carve_too_small(void) {
    uint8_t out[PART_BOOT_SIZE];
    TEST_ASSERT_EQUAL_INT(PART_RESULT_TOO_SMALL,
                          part_carve(g_image, 16u, PART_KIND_BOOT, out, sizeof(out)));
    TEST_ASSERT_EQUAL_INT(PART_RESULT_TOO_SMALL,
                          part_carve(g_image, sizeof(g_image), PART_KIND_BOOT, out, 1u));
}

/**
 * @brief Confirm carving rejects an unknown partition kind.
 *
 * @param void No parameters.
 * @return void
 */
static void test_part_carve_unknown_kind(void) {
    uint8_t out[PART_BOOT_SIZE];
    TEST_ASSERT_EQUAL_INT(PART_RESULT_BAD_ARGUMENT,
                          part_carve(g_image, sizeof(g_image), (part_kind_t)99,
                                     out, sizeof(out)));
}

/**
 * @brief Shared node buffer used by the JFFS2 tests.
 */
static uint8_t g_node[64];

/**
 * @brief Shared ASCII check message used by the CRC tests.
 */
static const char g_msg[] = "123456789";

/**
 * @brief Shared environment buffer used by the env tests.
 */
static uint8_t g_env[64];

/**
 * @brief Write a 32-bit little-endian word into bytes.
 *
 * @param dst Pointer to four writable bytes.
 * @param value Value to serialize.
 * @return void
 */
static void write32_le_test(uint8_t *dst, uint32_t value) {
    dst[0] = (uint8_t)(value & 0xFFu);
    dst[1] = (uint8_t)((value >> 8u) & 0xFFu);
    dst[2] = (uint8_t)((value >> 16u) & 0xFFu);
    dst[3] = (uint8_t)((value >> 24u) & 0xFFu);
}

/**
 * @brief Build an environment blob with a zero CRC field.
 *
 * @param blob NUL-terminated key/value text.
 * @return size_t Total blob length including the CRC field.
 */
static size_t env_build(const char *blob) {
    size_t n = ENV_CRC_SIZE + strlen(blob) + 1u;
    memcpy(&g_env[ENV_CRC_SIZE], blob, strlen(blob) + 1u);
    g_env[0] = 0u; g_env[1] = 0u; g_env[2] = 0u; g_env[3] = 0u;
    return n;
}

/**
 * @brief Store the computed environment CRC into the leading field.
 *
 * @param n Total blob length including the CRC field.
 * @return void
 */
static void env_seal(size_t n) {
    uint32_t c = crc32_uboot(&g_env[ENV_CRC_SIZE], n - ENV_CRC_SIZE);
    write32_le_test(g_env, c);
}

/**
 * @brief Build a JFFS2 CLEANMARKER node with a valid header CRC.
 *
 * @param void No parameters.
 * @return void
 */
static void node_build_cleanmarker(void) {
    memset(g_node, 0, sizeof(g_node));
    g_node[0] = 0x85u; g_node[1] = 0x19u;
    g_node[2] = 0x03u; g_node[3] = 0x20u;
    g_node[4] = 0x0Cu; g_node[5] = 0u; g_node[6] = 0u; g_node[7] = 0u;
    write32_le_test(&g_node[8], crc32_le(0u, g_node, 8u));
}

/**
 * @brief Confirm the standard CRC-32 check value for "123456789".
 *
 * @param void No parameters.
 * @return void
 */
static void test_crc32_uboot_check(void) {
    TEST_ASSERT_EQUAL_UINT(0xCBF43926u, crc32_uboot((const uint8_t *)g_msg,
                                                     sizeof(g_msg) - 1u));
}

/**
 * @brief Confirm crc32_le of an empty buffer returns the seed.
 *
 * @param void No parameters.
 * @return void
 */
static void test_crc32_le_empty(void) {
    TEST_ASSERT_EQUAL_UINT(7u, crc32_le(7u, g_image, 0u));
}

/**
 * @brief Confirm the CRC-16/CCITT-FALSE check value for "123456789".
 *
 * @param void No parameters.
 * @return void
 */
static void test_crc16_check(void) {
    TEST_ASSERT_EQUAL_HEX16(0x29B1u, crc16_ccitt((const uint8_t *)g_msg,
                                                 sizeof(g_msg) - 1u));
}

/**
 * @brief Confirm a sealed environment verifies and yields its values.
 *
 * @param void No parameters.
 * @return void
 */
static void test_env_crc_valid(void) {
    size_t n = env_build("bootargs=console=ttyAMA0");
    env_seal(n);
    TEST_ASSERT_TRUE(env_crc_valid(g_env, n));
    TEST_ASSERT_EQUAL_STRING("console=ttyAMA0", env_find(g_env, n, "bootargs"));
}

/**
 * @brief Confirm an unsealed environment fails verification.
 *
 * @param void No parameters.
 * @return void
 */
static void test_env_crc_invalid(void) {
    size_t n = env_build("bootdelay=1");
    TEST_ASSERT_FALSE(env_crc_valid(g_env, n));
    TEST_ASSERT_TRUE(env_find(g_env, n, "missing") == NULL);
}

/**
 * @brief Confirm the env helpers reject short and null inputs.
 *
 * @param void No parameters.
 * @return void
 */
static void test_env_short(void) {
    TEST_ASSERT_EQUAL_UINT(0u, env_compute_crc(g_image, 2u));
    TEST_ASSERT_FALSE(env_crc_valid(g_image, 2u));
    TEST_ASSERT_TRUE(env_find(NULL, 8u, "x") == NULL);
}

/**
 * @brief Confirm a JFFS2 node header parses its fields.
 *
 * @param void No parameters.
 * @return void
 */
static void test_jffs2_header(void) {
    node_build_cleanmarker();
    TEST_ASSERT_EQUAL_HEX16(JFFS2_MAGIC, jffs2_magic(g_node));
    TEST_ASSERT_EQUAL_HEX16(JFFS2_TYPE_CLEANMARKER, jffs2_type(g_node));
    TEST_ASSERT_EQUAL_UINT(12u, jffs2_totlen(g_node));
}

/**
 * @brief Confirm a JFFS2 header CRC validates and detects tampering.
 *
 * @param void No parameters.
 * @return void
 */
static void test_jffs2_crc(void) {
    node_build_cleanmarker();
    TEST_ASSERT_TRUE(jffs2_hdr_crc_valid(g_node));
    g_node[0] ^= 0xFFu;
    TEST_ASSERT_FALSE(jffs2_hdr_crc_valid(g_node));
}

/**
 * @brief Canonical vendor kernel container header bytes.
 */
static const uint8_t g_cont_init[CONTAINER_HDR_SIZE] = {
    0x21u, 0x84u, 0x1Bu, 0x00u, 0x00u, 0xF0u, 0x2Cu, 0x00u,
    'g', 'z', 'i', 'p', 'h', 'e', 'a', 'd',
};

/**
 * @brief Mutable container header used by the container tests.
 */
static uint8_t g_cont[CONTAINER_HDR_SIZE];

/**
 * @brief Restore the canonical container header bytes.
 *
 * @param void No parameters.
 * @return void
 */
static void cont_reset(void) {
    memcpy(g_cont, g_cont_init, sizeof(g_cont));
}

/**
 * @brief Confirm the container header fields parse correctly.
 *
 * @param void No parameters.
 * @return void
 */
static void test_container_fields(void) {
    cont_reset();
    TEST_ASSERT_EQUAL_UINT(CONTAINER_MAGIC, container_magic(g_cont));
    TEST_ASSERT_EQUAL_UINT(0x002CF000u, container_length(g_cont));
    TEST_ASSERT_TRUE(container_tag_ok(g_cont));
    TEST_ASSERT_TRUE(container_valid(g_cont, sizeof(g_cont)));
}

/**
 * @brief Confirm the container validator rejects bad headers.
 *
 * @param void No parameters.
 * @return void
 */
static void test_container_invalid(void) {
    TEST_ASSERT_FALSE(container_valid(NULL, CONTAINER_HDR_SIZE));
    TEST_ASSERT_FALSE(container_valid(g_cont, 8u));
    cont_reset();
    g_cont[0] ^= 0xFFu;
    TEST_ASSERT_FALSE(container_valid(g_cont, sizeof(g_cont)));
}

/**
 * @brief Confirm the weak key is deterministic for a fixed UID.
 *
 * @param void No parameters.
 * @return void
 */
static void test_beacon_weak_key(void) {
    uint8_t k1[BEACON_KEY_SIZE];
    uint8_t k2[BEACON_KEY_SIZE];
    beacon_weak_key("SSAT-468547-FEEBD", k1);
    beacon_weak_key("SSAT-468547-FEEBD", k2);
    TEST_ASSERT_EQUAL_MEMORY(k1, k2, sizeof(k1));
}

/**
 * @brief Confirm different UIDs derive different weak keys.
 *
 * @param void No parameters.
 * @return void
 */
static void test_beacon_weak_key_differs(void) {
    uint8_t k1[BEACON_KEY_SIZE];
    uint8_t k2[BEACON_KEY_SIZE];
    beacon_weak_key("A", k1);
    beacon_weak_key("B", k2);
    TEST_ASSERT_TRUE(memcmp(k1, k2, sizeof(k1)) != 0);
}

/**
 * @brief Confirm the nonce encodes the sequence number little-endian.
 *
 * @param void No parameters.
 * @return void
 */
static void test_beacon_nonce(void) {
    uint8_t n[AEAD_XCHACHA_NONCE_SIZE];
    beacon_nonce(0x11223344u, n);
    TEST_ASSERT_EQUAL_UINT(0x44u, n[0]);
    TEST_ASSERT_EQUAL_UINT(0x33u, n[1]);
    TEST_ASSERT_EQUAL_UINT(0u, n[4]);
}

/**
 * @brief Confirm the hardened beacon seal succeeds with an AEAD key.
 *
 * @param void No parameters.
 * @return void
 */
static void test_beacon_seal(void) {
    uint8_t key[BEACON_KEY_SIZE];
    uint8_t sealed[AEAD_SEALED_SIZE];
    aead_init(AEAD_ALGO_AES_256_GCM);
    beacon_weak_key("uid", key);
    memset(g_pt, 0x77, sizeof(g_pt));
    TEST_ASSERT_TRUE(beacon_seal(key, 1u, g_pt, sealed));
}


/**
 * @brief RFC 7748 X25519 Alice private scalar.
 */
static const uint8_t s_alice_priv[32] = {
    0x77u, 0x07u, 0x6Du, 0x0Au, 0x73u, 0x18u, 0xA5u, 0x7Du,
    0x3Cu, 0x16u, 0xC1u, 0x72u, 0x51u, 0xB2u, 0x66u, 0x45u,
    0xDFu, 0x4Cu, 0x2Fu, 0x87u, 0xEBu, 0xC0u, 0x99u, 0x2Au,
    0xB1u, 0x77u, 0xFBu, 0xA5u, 0x1Du, 0xB9u, 0x2Cu, 0x2Au,
};

/**
 * @brief RFC 7748 X25519 Alice public key.
 */
static const uint8_t s_alice_pub[32] = {
    0x85u, 0x20u, 0xF0u, 0x09u, 0x89u, 0x30u, 0xA7u, 0x54u,
    0x74u, 0x8Bu, 0x7Du, 0xDCu, 0xB4u, 0x3Eu, 0xF7u, 0x5Au,
    0x0Du, 0xBFu, 0x3Au, 0x0Du, 0x26u, 0x38u, 0x1Au, 0xF4u,
    0xEBu, 0xA4u, 0xA9u, 0x8Eu, 0xAAu, 0x9Bu, 0x4Eu, 0x6Au,
};

/**
 * @brief RFC 7748 X25519 Bob private scalar.
 */
static const uint8_t s_bob_priv[32] = {
    0x5Du, 0xABu, 0x08u, 0x7Eu, 0x62u, 0x4Au, 0x8Au, 0x4Bu,
    0x79u, 0xE1u, 0x7Fu, 0x8Bu, 0x83u, 0x80u, 0x0Eu, 0xE6u,
    0x6Fu, 0x3Bu, 0xB1u, 0x29u, 0x26u, 0x18u, 0xB6u, 0xFDu,
    0x1Cu, 0x2Fu, 0x8Bu, 0x27u, 0xFFu, 0x88u, 0xE0u, 0xEBu,
};

/**
 * @brief RFC 7748 X25519 Bob public key.
 */
static const uint8_t s_bob_pub[32] = {
    0xDEu, 0x9Eu, 0xDBu, 0x7Du, 0x7Bu, 0x7Du, 0xC1u, 0xB4u,
    0xD3u, 0x5Bu, 0x61u, 0xC2u, 0xECu, 0xE4u, 0x35u, 0x37u,
    0x3Fu, 0x83u, 0x43u, 0xC8u, 0x5Bu, 0x78u, 0x67u, 0x4Du,
    0xADu, 0xFCu, 0x7Eu, 0x14u, 0x6Fu, 0x88u, 0x2Bu, 0x4Fu,
};

/**
 * @brief RFC 7748 X25519 expected shared secret.
 */
static const uint8_t s_shared[32] = {
    0x4Au, 0x5Du, 0x9Du, 0x5Bu, 0xA4u, 0xCEu, 0x2Du, 0xE1u,
    0x72u, 0x8Eu, 0x3Bu, 0xF4u, 0x80u, 0x35u, 0x0Fu, 0x25u,
    0xE0u, 0x7Eu, 0x21u, 0xC9u, 0x47u, 0xD1u, 0x9Eu, 0x33u,
    0x76u, 0xF0u, 0x9Bu, 0x3Cu, 0x1Eu, 0x16u, 0x17u, 0x42u,
};

/**
 * @brief RFC 5869 HKDF-SHA256 test-case-1 input key material.
 */
static const uint8_t s_ikm[22] = {
    0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu,
    0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu,
    0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu, 0x0Bu,
};

/**
 * @brief RFC 5869 HKDF-SHA256 test-case-1 salt.
 */
static const uint8_t s_salt[13] = {
    0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u,
    0x07u, 0x08u, 0x09u, 0x0Au, 0x0Bu, 0x0Cu,
};

/**
 * @brief RFC 5869 HKDF-SHA256 test-case-1 info.
 */
static const uint8_t s_info[10] = {
    0xF0u, 0xF1u, 0xF2u, 0xF3u, 0xF4u, 0xF5u, 0xF6u, 0xF7u, 0xF8u, 0xF9u,
};

/**
 * @brief RFC 5869 HKDF-SHA256 test-case-1 expected output.
 */
static const uint8_t s_okm[42] = {
    0x3Cu, 0xB2u, 0x5Fu, 0x25u, 0xFAu, 0xACu, 0xD5u, 0x7Au,
    0x90u, 0x43u, 0x4Fu, 0x64u, 0xD0u, 0x36u, 0x2Fu, 0x2Au,
    0x2Du, 0x2Du, 0x0Au, 0x90u, 0xCFu, 0x1Au, 0x5Au, 0x4Cu,
    0x5Du, 0xB0u, 0x2Du, 0x56u, 0xECu, 0xC4u, 0xC5u, 0xBFu,
    0x34u, 0x00u, 0x72u, 0x08u, 0xD5u, 0xB8u, 0x87u, 0x18u,
    0x58u, 0x65u,
};

/**
 * @brief Confirm X25519 matches the RFC 7748 vector both ways.
 *
 * @param void No parameters.
 * @return void
 */
static void test_kex_x25519_rfc7748(void) {
    uint8_t shared_a[32];
    uint8_t shared_b[32];
    TEST_ASSERT_TRUE(kex_x25519(s_alice_priv, s_bob_pub, shared_a));
    TEST_ASSERT_TRUE(kex_x25519(s_bob_priv, s_alice_pub, shared_b));
    TEST_ASSERT_EQUAL_MEMORY(s_shared, shared_a, 32u);
    TEST_ASSERT_EQUAL_MEMORY(s_shared, shared_b, 32u);
}

/**
 * @brief Confirm HKDF-SHA256 matches the RFC 5869 test case 1.
 *
 * @param void No parameters.
 * @return void
 */
static void test_kex_hkdf_rfc5869(void) {
    uint8_t okm[42];
    TEST_ASSERT_TRUE(kex_hkdf(s_ikm, sizeof(s_ikm), s_salt, sizeof(s_salt),
                              s_info, sizeof(s_info), okm, sizeof(okm)));
    TEST_ASSERT_EQUAL_MEMORY(s_okm, okm, sizeof(okm));
}


/**
 * @brief RFC 8032 Ed25519 test 1 private seed.
 */
static const uint8_t s_ed_priv[32] = {
    0x9Du, 0x61u, 0xB1u, 0x9Du, 0xEFu, 0xFDu, 0x5Au, 0x60u,
    0xBAu, 0x84u, 0x4Au, 0xF4u, 0x92u, 0xECu, 0x2Cu, 0xC4u,
    0x44u, 0x49u, 0xC5u, 0x69u, 0x7Bu, 0x32u, 0x69u, 0x19u,
    0x70u, 0x3Bu, 0xACu, 0x03u, 0x1Cu, 0xAEu, 0x7Fu, 0x60u,
};

/**
 * @brief RFC 8032 Ed25519 test 1 public key.
 */
static const uint8_t s_ed_pub[32] = {
    0xD7u, 0x5Au, 0x98u, 0x01u, 0x82u, 0xB1u, 0x0Au, 0xB7u,
    0xD5u, 0x4Bu, 0xFEu, 0xD3u, 0xC9u, 0x64u, 0x07u, 0x3Au,
    0x0Eu, 0xE1u, 0x72u, 0xF3u, 0xDAu, 0xA6u, 0x23u, 0x25u,
    0xAFu, 0x02u, 0x1Au, 0x68u, 0xF7u, 0x07u, 0x51u, 0x1Au,
};

/**
 * @brief RFC 8032 Ed25519 test 1 empty message buffer.
 */
static const uint8_t s_ed_msg[1] = { 0x00u };

/**
 * @brief RFC 8032 Ed25519 test 1 expected signature.
 */
static const uint8_t s_ed_sig[64] = {
    0xE5u, 0x56u, 0x43u, 0x00u, 0xC3u, 0x60u, 0xACu, 0x72u,
    0x90u, 0x86u, 0xE2u, 0xCCu, 0x80u, 0x6Eu, 0x82u, 0x8Au,
    0x84u, 0x87u, 0x7Fu, 0x1Eu, 0xB8u, 0xE5u, 0xD9u, 0x74u,
    0xD8u, 0x73u, 0xE0u, 0x65u, 0x22u, 0x49u, 0x01u, 0x55u,
    0x5Fu, 0xB8u, 0x82u, 0x15u, 0x90u, 0xA3u, 0x3Bu, 0xACu,
    0xC6u, 0x1Eu, 0x39u, 0x70u, 0x1Cu, 0xF9u, 0xB4u, 0x6Bu,
    0xD2u, 0x5Bu, 0xF5u, 0xF0u, 0x59u, 0x5Bu, 0xBEu, 0x24u,
    0x65u, 0x51u, 0x41u, 0x43u, 0x8Eu, 0x7Au, 0x10u, 0x0Bu,
};

/**
 * @brief Confirm Ed25519 matches the RFC 8032 test 1 vector.
 *
 * @param void No parameters.
 * @return void
 */
static void test_identity_rfc8032(void) {
    uint8_t pub[IDENTITY_KEY_SIZE];
    uint8_t sig[IDENTITY_SIG_SIZE];
    TEST_ASSERT_TRUE(identity_pubkey(s_ed_priv, pub));
    TEST_ASSERT_EQUAL_MEMORY(s_ed_pub, pub, sizeof(pub));
    TEST_ASSERT_TRUE(identity_sign(s_ed_priv, s_ed_msg, 0u, sig));
    TEST_ASSERT_EQUAL_MEMORY(s_ed_sig, sig, sizeof(sig));
    TEST_ASSERT_TRUE(identity_verify(s_ed_pub, s_ed_msg, 0u, sig));
}

/**
 * @brief Confirm Ed25519 rejects a tampered signature.
 *
 * @param void No parameters.
 * @return void
 */
static void test_identity_verify_bad(void) {
    uint8_t sig[IDENTITY_SIG_SIZE];
    memcpy(sig, s_ed_sig, sizeof(sig));
    sig[0] ^= 0xFFu;
    TEST_ASSERT_FALSE(identity_verify(s_ed_pub, s_ed_msg, 0u, sig));
}


/**
 * @brief Confirm the collector accepts a well-formed beacon.
 *
 * @param void No parameters.
 * @return void
 */
static void test_collector_accept(void) {
    uint8_t sealed[AEAD_SEALED_SIZE];
    collector_init();
    TEST_ASSERT_TRUE(collector_accept(sealed, sizeof(sealed), 5u));
    TEST_ASSERT_EQUAL_UINT(1u, collector_stats()->accepted);
    TEST_ASSERT_EQUAL_UINT(5u, collector_stats()->last_seq);
}

/**
 * @brief Confirm the collector rejects null and short frames.
 *
 * @param void No parameters.
 * @return void
 */
static void test_collector_reject(void) {
    uint8_t sealed[AEAD_SEALED_SIZE];
    collector_init();
    TEST_ASSERT_FALSE(collector_accept(NULL, sizeof(sealed), 0u));
    TEST_ASSERT_FALSE(collector_accept(sealed, 1u, 0u));
    TEST_ASSERT_EQUAL_UINT(2u, collector_stats()->rejected);
}


/**
 * @brief Confirm the daemon seals and delivers a beacon.
 *
 * @param void No parameters.
 * @return void
 */
static void test_teled_beacon(void) {
    uint8_t key[32];
    memset(key, 0, sizeof(key));
    aead_init(AEAD_ALGO_AES_256_GCM);
    teled_init("SSAT-468547-FEEBD", key);
    TEST_ASSERT_TRUE(teled_beacon(g_pt));
    TEST_ASSERT_EQUAL_UINT(1u, teled_state()->beacons);
    TEST_ASSERT_EQUAL_STRING("SSAT-468547-FEEBD", teled_state()->uid);
}

/**
 * @brief Confirm the daemon rejects a null payload.
 *
 * @param void No parameters.
 * @return void
 */
static void test_teled_beacon_fail(void) {
    uint8_t key[32];
    memset(key, 0, sizeof(key));
    teled_init("uid", key);
    TEST_ASSERT_FALSE(teled_beacon(NULL));
}


/**
 * @brief Confirm the RTSP URL builder.
 *
 * @param void No parameters.
 * @return void
 */
static void test_camera_rtsp_url(void) {
    char out[64];
    TEST_ASSERT_TRUE(camera_rtsp_url("192.168.1.88", 554u, out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("rtsp://192.168.1.88:554/stream", out);
}

/**
 * @brief Confirm the MJPEG URL builder.
 *
 * @param void No parameters.
 * @return void
 */
static void test_camera_mjpeg_url(void) {
    char out[64];
    TEST_ASSERT_TRUE(camera_mjpeg_url("192.168.1.88", 80u, out, sizeof(out)));
    TEST_ASSERT_EQUAL_STRING("http://192.168.1.88:80/video.mjpg", out);
}

/**
 * @brief Confirm the URL builders reject small and null buffers.
 *
 * @param void No parameters.
 * @return void
 */
static void test_camera_url_bad(void) {
    char out[4];
    TEST_ASSERT_FALSE(camera_rtsp_url("192.168.1.88", 554u, out, sizeof(out)));
    TEST_ASSERT_FALSE(camera_rtsp_url(NULL, 554u, out, sizeof(out)));
}

/**
 * @brief Run the AEAD initialization and argument tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_aead_tests(void) {
    RUN_TEST(test_aead_nonce_size_before_init);
    RUN_TEST(test_aead_init_rejects_bad_algo);
    RUN_TEST(test_aead_init_aes);
    RUN_TEST(test_aead_seal_bad_args);
    RUN_TEST(test_aead_open_bad_args);
    RUN_TEST(test_aead_seal_open_aes);
}

/**
 * @brief Run the remaining AEAD round-trip and tamper tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_aead_extra_tests(void) {
    RUN_TEST(test_aead_open_aes_bad_tag);
    RUN_TEST(test_aead_nonce_reuse);
    RUN_TEST(test_aead_init_xchacha);
    RUN_TEST(test_aead_seal_open_xchacha);
    RUN_TEST(test_aead_open_xchacha_bad_tag);
}

/**
 * @brief Run the four-partition model tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_partition_tests(void) {
    RUN_TEST(test_part_desc_for_boot);
    RUN_TEST(test_part_desc_for_unknown);
    RUN_TEST(test_part_identify_boot);
    RUN_TEST(test_part_identify_kernel);
    RUN_TEST(test_part_identify_rootfs);
    RUN_TEST(test_part_identify_env);
    RUN_TEST(test_part_identify_unknown);
    RUN_TEST(test_part_verify_layout);
}

/**
 * @brief Run the four-partition carving tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_partition_carve_tests(void) {
    RUN_TEST(test_part_carve_ok);
    RUN_TEST(test_part_carve_bad_args);
    RUN_TEST(test_part_carve_too_small);
    RUN_TEST(test_part_carve_unknown_kind);
}

/**
 * @brief Run the integrity primitive tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_crc_tests(void) {
    RUN_TEST(test_crc32_uboot_check);
    RUN_TEST(test_crc32_le_empty);
    RUN_TEST(test_crc16_check);
}

/**
 * @brief Run the U-Boot environment tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_env_tests(void) {
    RUN_TEST(test_env_crc_valid);
    RUN_TEST(test_env_crc_invalid);
    RUN_TEST(test_env_short);
}

/**
 * @brief Run the JFFS2 node header tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_jffs2_tests(void) {
    RUN_TEST(test_jffs2_header);
    RUN_TEST(test_jffs2_crc);
}

/**
 * @brief Run the vendor kernel container tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_container_tests(void) {
    RUN_TEST(test_container_fields);
    RUN_TEST(test_container_invalid);
}

/**
 * @brief Run the exfiltration beacon tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_beacon_tests(void) {
    RUN_TEST(test_beacon_weak_key);
    RUN_TEST(test_beacon_weak_key_differs);
    RUN_TEST(test_beacon_nonce);
    RUN_TEST(test_beacon_seal);
}

/**
 * @brief Run the key agreement and derivation tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_kex_tests(void) {
    RUN_TEST(test_kex_x25519_rfc7748);
    RUN_TEST(test_kex_hkdf_rfc5869);
}

/**
 * @brief Run the Ed25519 identity tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_identity_tests(void) {
    RUN_TEST(test_identity_rfc8032);
    RUN_TEST(test_identity_verify_bad);
}

/**
 * @brief Run the local collector tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_collector_tests(void) {
    RUN_TEST(test_collector_accept);
    RUN_TEST(test_collector_reject);
}

/**
 * @brief Run the TELESCREEN daemon tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_teled_tests(void) {
    RUN_TEST(test_teled_beacon);
    RUN_TEST(test_teled_beacon_fail);
}

/**
 * @brief Run the UVC camera URL tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_camera_tests(void) {
    RUN_TEST(test_camera_rtsp_url);
    RUN_TEST(test_camera_mjpeg_url);
    RUN_TEST(test_camera_url_bad);
}

/**
 * @brief Run every TELESCREEN native test group.
 *
 * @param void No parameters.
 * @return void
 */
static void run_core_groups(void) {
    run_aead_tests();
    run_aead_extra_tests();
    run_crc_tests();
    run_env_tests();
}

/**
 * @brief Run the data-model test groups.
 *
 * @param void No parameters.
 * @return void
 */
static void run_data_groups(void) {
    run_jffs2_tests();
    run_container_tests();
    run_beacon_tests();
    run_kex_tests();
}

/**
 * @brief Run the identity and model test groups.
 *
 * @param void No parameters.
 * @return void
 */
static void run_model_groups(void) {
    run_identity_tests();
    run_collector_tests();
    run_teled_tests();
    run_camera_tests();
    run_partition_tests();
    run_partition_carve_tests();
}

/**
 * @brief Execute the full TELESCREEN native test suite.
 *
 * @param void No parameters.
 * @return int Number of failed checks.
 */
int main(void) {
    TEST_BEGIN();
    run_core_groups();
    run_data_groups();
    run_model_groups();
    return TEST_END();
}
```

## `test/test_telescreen.py`

```
"""Unit test adapter for VS Code Test Explorer."""
import subprocess
import sys
import unittest

_CACHED_OUTPUT = ""


def _get_harness_output() -> str:
    """
    Execute native tests and return stdout.

    Parameters
    ----------
    None

    Returns
    -------
    str
        Standard output from native test suite.
    """
    global _CACHED_OUTPUT
    if not _CACHED_OUTPUT:
        cmd = [sys.executable, "scripts/run_tests.py"]
        res = subprocess.run(cmd, capture_output=True, text=True)
        _CACHED_OUTPUT = res.stdout
    return _CACHED_OUTPUT


def _assert_harness_pass(test_name: str) -> None:
    """
    Assert that a named harness test passed.

    Parameters
    ----------
    test_name : str
        Name of harness test function.

    Returns
    -------
    None
    """
    output = _get_harness_output()
    expected = f":{test_name}:PASS"
    assert expected in output, f"{test_name} did not pass in harness output"


class TestTelescreenFirmware(unittest.TestCase):
    """Test cases for the TELESCREEN RP5 lab modules."""

    def test_01_aead_nonce_size_before_init(self) -> None:
        """
        Verify the aead_nonce_size_before_init native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_nonce_size_before_init")

    def test_02_aead_init_rejects_bad_algo(self) -> None:
        """
        Verify the aead_init_rejects_bad_algo native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_init_rejects_bad_algo")

    def test_03_aead_init_aes(self) -> None:
        """
        Verify the aead_init_aes native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_init_aes")

    def test_04_aead_seal_bad_args(self) -> None:
        """
        Verify the aead_seal_bad_args native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_seal_bad_args")

    def test_05_aead_open_bad_args(self) -> None:
        """
        Verify the aead_open_bad_args native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_open_bad_args")

    def test_06_aead_seal_open_aes(self) -> None:
        """
        Verify the aead_seal_open_aes native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_seal_open_aes")

    def test_07_aead_open_aes_bad_tag(self) -> None:
        """
        Verify the aead_open_aes_bad_tag native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_open_aes_bad_tag")

    def test_08_aead_nonce_reuse(self) -> None:
        """
        Verify the aead_nonce_reuse native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_nonce_reuse")

    def test_09_aead_init_xchacha(self) -> None:
        """
        Verify the aead_init_xchacha native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_init_xchacha")

    def test_10_aead_seal_open_xchacha(self) -> None:
        """
        Verify the aead_seal_open_xchacha native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_seal_open_xchacha")

    def test_11_aead_open_xchacha_bad_tag(self) -> None:
        """
        Verify the aead_open_xchacha_bad_tag native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_open_xchacha_bad_tag")

    def test_12_crc32_uboot_check(self) -> None:
        """
        Verify the crc32_uboot_check native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_crc32_uboot_check")

    def test_13_crc32_le_empty(self) -> None:
        """
        Verify the crc32_le_empty native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_crc32_le_empty")

    def test_14_crc16_check(self) -> None:
        """
        Verify the crc16_check native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_crc16_check")

    def test_15_env_crc_valid(self) -> None:
        """
        Verify the env_crc_valid native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_env_crc_valid")

    def test_16_env_crc_invalid(self) -> None:
        """
        Verify the env_crc_invalid native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_env_crc_invalid")

    def test_17_env_short(self) -> None:
        """
        Verify the env_short native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_env_short")

    def test_18_jffs2_header(self) -> None:
        """
        Verify the jffs2_header native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_jffs2_header")

    def test_19_jffs2_crc(self) -> None:
        """
        Verify the jffs2_crc native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_jffs2_crc")

    def test_20_container_fields(self) -> None:
        """
        Verify the container_fields native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_container_fields")

    def test_21_container_invalid(self) -> None:
        """
        Verify the container_invalid native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_container_invalid")

    def test_22_beacon_weak_key(self) -> None:
        """
        Verify the beacon_weak_key native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_beacon_weak_key")

    def test_23_beacon_weak_key_differs(self) -> None:
        """
        Verify the beacon_weak_key_differs native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_beacon_weak_key_differs")

    def test_24_beacon_nonce(self) -> None:
        """
        Verify the beacon_nonce native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_beacon_nonce")

    def test_25_beacon_seal(self) -> None:
        """
        Verify the beacon_seal native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_beacon_seal")

    def test_26_kex_x25519_rfc7748(self) -> None:
        """
        Verify the kex_x25519_rfc7748 native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_kex_x25519_rfc7748")

    def test_27_kex_hkdf_rfc5869(self) -> None:
        """
        Verify the kex_hkdf_rfc5869 native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_kex_hkdf_rfc5869")

    def test_28_identity_rfc8032(self) -> None:
        """
        Verify the identity_rfc8032 native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_identity_rfc8032")

    def test_29_identity_verify_bad(self) -> None:
        """
        Verify the identity_verify_bad native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_identity_verify_bad")

    def test_30_collector_accept(self) -> None:
        """
        Verify the collector_accept native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_collector_accept")

    def test_31_collector_reject(self) -> None:
        """
        Verify the collector_reject native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_collector_reject")

    def test_32_teled_beacon(self) -> None:
        """
        Verify the teled_beacon native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_teled_beacon")

    def test_33_teled_beacon_fail(self) -> None:
        """
        Verify the teled_beacon_fail native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_teled_beacon_fail")

    def test_34_part_desc_for_boot(self) -> None:
        """
        Verify the part_desc_for_boot native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_desc_for_boot")

    def test_35_part_desc_for_unknown(self) -> None:
        """
        Verify the part_desc_for_unknown native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_desc_for_unknown")

    def test_36_part_identify_boot(self) -> None:
        """
        Verify the part_identify_boot native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_identify_boot")

    def test_37_part_identify_kernel(self) -> None:
        """
        Verify the part_identify_kernel native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_identify_kernel")

    def test_38_part_identify_rootfs(self) -> None:
        """
        Verify the part_identify_rootfs native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_identify_rootfs")

    def test_39_part_identify_env(self) -> None:
        """
        Verify the part_identify_env native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_identify_env")

    def test_40_part_identify_unknown(self) -> None:
        """
        Verify the part_identify_unknown native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_identify_unknown")

    def test_41_part_verify_layout(self) -> None:
        """
        Verify the part_verify_layout native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_verify_layout")

    def test_42_part_carve_ok(self) -> None:
        """
        Verify the part_carve_ok native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_carve_ok")

    def test_43_part_carve_bad_args(self) -> None:
        """
        Verify the part_carve_bad_args native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_carve_bad_args")

    def test_44_part_carve_too_small(self) -> None:
        """
        Verify the part_carve_too_small native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_carve_too_small")

    def test_45_part_carve_unknown_kind(self) -> None:
        """
        Verify the part_carve_unknown_kind native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_carve_unknown_kind")

    def test_46_camera_rtsp_url(self) -> None:
        """
        Verify the camera_rtsp_url native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_camera_rtsp_url")

    def test_47_camera_mjpeg_url(self) -> None:
        """
        Verify the camera_mjpeg_url native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_camera_mjpeg_url")

    def test_48_camera_url_bad(self) -> None:
        """
        Verify the camera_url_bad native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_camera_url_bad")
```

## `test/harness/harness.c`

```
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
// GitHub:  https://github.com/mytechnotalent/cold-chain-monitor-c-rp2350
// File:    harness.c
// Desc:    Minimal in-repo test harness implementation replacing the vendored
//          framework with a strictly C-standard-compliant runner.
// Created: 2026

#include "harness.h"
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief Longjmp target used to abandon a failing test case.
 */
static jmp_buf g_jmp;

/**
 * @brief Number of failed checks in the current run.
 */
static int g_failures;

/**
 * @brief Number of checks executed in the current run.
 */
static int g_checks;

/**
 * @brief Name of the currently running test case.
 */
static const char *g_case;

/**
 * @brief Record a failure, report it, and abandon the current test case.
 *
 * @param file Source file of the failing check.
 * @param line Source line of the failing check.
 * @param msg Short failure description.
 * @return void
 */
static void harness_fail(const char *file, int line, const char *msg) {
    g_failures += 1;
    printf("%s:%d:%s:FAIL:%s\n", file, line, g_case, msg);
    longjmp(g_jmp, 1);
}

/**
 * @brief Initialize the runner counters.
 *
 * @param void No parameters.
 * @return void
 */
void harness_begin(void) {
    g_failures = 0;
    g_checks = 0;
}

/**
 * @brief Print the run summary and return the failure count.
 *
 * @param void No parameters.
 * @return int Number of failed checks.
 */
int harness_finish(void) {
    printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures;
}

/**
 * @brief Run one test case with setup, teardown, and failure isolation.
 *
 * @param name Test case name.
 * @param fn Test case function pointer.
 * @return void
 */
void harness_run(const char *name, void (*fn)(void)) {
    g_case = name;
    if (setjmp(g_jmp) == 0) {
        setUp();
        fn();
        tearDown();
        printf(":%s:PASS\n", name);
    } else {
        printf(":%s:FAIL\n", name);
    }
}

/**
 * @brief Record and report a failing boolean-true check.
 *
 * @param ok Nonzero when the check passed.
 * @param file Source file of the check.
 * @param line Source line of the check.
 * @return void
 */
void harness_assert_true(int ok, const char *file, int line) {
    g_checks += 1;
    if (!ok) harness_fail(file, line, "expected true");
}

/**
 * @brief Record and report a failing boolean-false check.
 *
 * @param ok Nonzero when the negated check passed.
 * @param file Source file of the check.
 * @param line Source line of the check.
 * @return void
 */
void harness_assert_false(int ok, const char *file, int line) {
    g_checks += 1;
    if (ok) harness_fail(file, line, "expected false");
}

/**
 * @brief Record and report a failing signed integer comparison.
 *
 * @param expected Expected value.
 * @param actual Actual value.
 * @param file Source file of the check.
 * @param line Source line of the check.
 * @return void
 */
void harness_assert_int(long expected, long actual, const char *file, int line) {
    g_checks += 1;
    if (expected != actual) harness_fail(file, line, "int mismatch");
}

/**
 * @brief Record and report a failing unsigned integer comparison.
 *
 * @param expected Expected value.
 * @param actual Actual value.
 * @param file Source file of the check.
 * @param line Source line of the check.
 * @return void
 */
void harness_assert_uint(unsigned long expected, unsigned long actual,
                       const char *file, int line) {
    g_checks += 1;
    if (expected != actual) harness_fail(file, line, "uint mismatch");
}

/**
 * @brief Record and report a failing string comparison.
 *
 * @param expected Expected string.
 * @param actual Actual string.
 * @param file Source file of the check.
 * @param line Source line of the check.
 * @return void
 */
void harness_assert_str(const char *expected, const char *actual, const char *file,
                      int line) {
    g_checks += 1;
    if ((expected == NULL) || (actual == NULL) || (strcmp(expected, actual) != 0)) {
        harness_fail(file, line, "string mismatch");
    }
}

/**
 * @brief Record and report a failing memory comparison.
 *
 * @param expected Expected buffer.
 * @param actual Actual buffer.
 * @param len Number of bytes to compare.
 * @param file Source file of the check.
 * @param line Source line of the check.
 * @return void
 */
void harness_assert_mem(const void *expected, const void *actual, size_t len,
                      const char *file, int line) {
    g_checks += 1;
    if ((expected == NULL) || (actual == NULL) || (memcmp(expected, actual, len) != 0)) {
        harness_fail(file, line, "memory mismatch");
    }
}
```

## `test/harness/harness.h`

```
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
// GitHub:  https://github.com/mytechnotalent/cold-chain-monitor-c-rp2350
// File:    harness.h
// Desc:    Minimal in-repo test harness API replacing the vendored framework.
// Created: 2026

#ifndef HARNESS_H
#define HARNESS_H

#include <stddef.h>

/**
 * @brief Test setup hook executed before each test case.
 *
 * @param void No parameters.
 * @return void
 */
void setUp(void);

/**
 * @brief Test teardown hook executed after each test case.
 *
 * @param void No parameters.
 * @return void
 */
void tearDown(void);

/**
 * @brief Initialize the runner counters.
 *
 * @param void No parameters.
 * @return void
 */
void harness_begin(void);

/**
 * @brief Print the run summary and return the failure count.
 *
 * @param void No parameters.
 * @return int Number of failed checks.
 */
int harness_finish(void);

/**
 * @brief Run one test case with setup, teardown, and failure isolation.
 *
 * @param name Test case name.
 * @param fn Test case function pointer.
 * @return void
 */
void harness_run(const char *name, void (*fn)(void));

/**
 * @brief Record and report a failing boolean-true check.
 *
 * @param ok Nonzero when the check passed.
 * @param file Source file of the check.
 * @param line Source line of the check.
 * @return void
 */
void harness_assert_true(int ok, const char *file, int line);

/**
 * @brief Record and report a failing boolean-false check.
 *
 * @param ok Nonzero when the negated check passed.
 * @param file Source file of the check.
 * @param line Source line of the check.
 * @return void
 */
void harness_assert_false(int ok, const char *file, int line);

/**
 * @brief Record and report a failing signed integer comparison.
 *
 * @param expected Expected value.
 * @param actual Actual value.
 * @param file Source file of the check.
 * @param line Source line of the check.
 * @return void
 */
void harness_assert_int(long expected, long actual, const char *file, int line);

/**
 * @brief Record and report a failing unsigned integer comparison.
 *
 * @param expected Expected value.
 * @param actual Actual value.
 * @param file Source file of the check.
 * @param line Source line of the check.
 * @return void
 */
void harness_assert_uint(unsigned long expected, unsigned long actual,
                       const char *file, int line);

/**
 * @brief Record and report a failing string comparison.
 *
 * @param expected Expected string.
 * @param actual Actual string.
 * @param file Source file of the check.
 * @param line Source line of the check.
 * @return void
 */
void harness_assert_str(const char *expected, const char *actual, const char *file,
                      int line);

/**
 * @brief Record and report a failing memory comparison.
 *
 * @param expected Expected buffer.
 * @param actual Actual buffer.
 * @param len Number of bytes to compare.
 * @param file Source file of the check.
 * @param line Source line of the check.
 * @return void
 */
void harness_assert_mem(const void *expected, const void *actual, size_t len,
                      const char *file, int line);

/**
 * @brief Open a harness test run.
 */
#define TEST_BEGIN() harness_begin()

/**
 * @brief Close a harness test run and return the failure count.
 */
#define TEST_END() harness_finish()

/**
 * @brief Register and run one test case by name.
 */
#define RUN_TEST(fn) harness_run(#fn, fn)

/**
 * @brief Assert that a condition is nonzero.
 */
#define TEST_ASSERT(cond) \
    harness_assert_true((cond) ? 1 : 0, __FILE__, __LINE__)

/**
 * @brief Assert that a condition is nonzero.
 */
#define TEST_ASSERT_TRUE(cond) \
    harness_assert_true((cond) ? 1 : 0, __FILE__, __LINE__)

/**
 * @brief Assert that a condition is zero.
 */
#define TEST_ASSERT_FALSE(cond) \
    harness_assert_false((cond) ? 1 : 0, __FILE__, __LINE__)

/**
 * @brief Assert two signed integers are equal.
 */
#define TEST_ASSERT_EQUAL(a, b) \
    harness_assert_int((long)(a), (long)(b), __FILE__, __LINE__)

/**
 * @brief Assert two signed integers are equal.
 */
#define TEST_ASSERT_EQUAL_INT(a, b) \
    harness_assert_int((long)(a), (long)(b), __FILE__, __LINE__)

/**
 * @brief Assert two unsigned integers are equal.
 */
#define TEST_ASSERT_EQUAL_UINT(a, b) \
    harness_assert_uint((unsigned long)(a), (unsigned long)(b), __FILE__, __LINE__)

/**
 * @brief Assert two unsigned 8-bit integers are equal.
 */
#define TEST_ASSERT_EQUAL_UINT8(a, b) \
    harness_assert_uint((unsigned long)(a), (unsigned long)(b), __FILE__, __LINE__)

/**
 * @brief Assert two unsigned 16-bit integers are equal.
 */
#define TEST_ASSERT_EQUAL_HEX16(a, b) \
    harness_assert_uint((unsigned long)(a), (unsigned long)(b), __FILE__, __LINE__)

/**
 * @brief Assert two NUL-terminated strings are equal.
 */
#define TEST_ASSERT_EQUAL_STRING(a, b) \
    harness_assert_str((a), (b), __FILE__, __LINE__)

/**
 * @brief Assert two memory regions are equal.
 */
#define TEST_ASSERT_EQUAL_MEMORY(a, b, n) \
    harness_assert_mem((a), (b), (n), __FILE__, __LINE__)

#endif // HARNESS_H
```
