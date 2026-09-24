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
#include "faults.h"
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

void setUp(void) {
    memset(g_image, 0, sizeof(g_image));
    set_boot_magic();
    set_kernel_magic();
    set_rootfs_magic();
}

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
 * @brief Confirm X25519 rejects null scalar and peer pointers.
 *
 * @param void No parameters.
 * @return void
 */
static void test_kex_bad_args(void) {
    uint8_t out[KEX_KEY_SIZE];
    uint8_t scalar[KEX_KEY_SIZE] = { 0u };
    TEST_ASSERT_FALSE(kex_x25519(NULL, scalar, out));
    TEST_ASSERT_FALSE(kex_x25519(scalar, NULL, out));
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
 * @brief Confirm the identity API rejects null key pointers.
 *
 * @param void No parameters.
 * @return void
 */
static void test_identity_bad_args(void) {
    uint8_t buf[IDENTITY_KEY_SIZE];
    TEST_ASSERT_FALSE(identity_pubkey(NULL, buf));
    TEST_ASSERT_FALSE(identity_sign(NULL, s_ed_msg, 0u, buf));
    TEST_ASSERT_FALSE(identity_verify(NULL, s_ed_msg, 0u, buf));
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
 * @brief Confirm the URL builders reject a null output and a zero length.
 *
 * @param void No parameters.
 * @return void
 */
static void test_camera_url_null(void) {
    char out[64];
    TEST_ASSERT_FALSE(camera_rtsp_url("192.168.1.88", 554u, NULL, sizeof(out)));
    TEST_ASSERT_FALSE(camera_rtsp_url("192.168.1.88", 554u, out, 0u));
}

/**
 * @brief Confirm aead_seal and aead_open reject calls before initialization.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_before_init(void) {
    TEST_ASSERT_TRUE(aead_seal(g_key, g_nonce, g_pt, g_sealed) == AEAD_RESULT_BAD_ARGUMENT);
    TEST_ASSERT_TRUE(aead_open(g_key, g_nonce, g_sealed, g_pt) == AEAD_RESULT_BAD_ARGUMENT);
}

/**
 * @brief Confirm aead_seal rejects every null argument.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_seal_null_args(void) {
    aead_init(AEAD_ALGO_AES_256_GCM);
    TEST_ASSERT_TRUE(aead_seal(NULL, g_nonce, g_pt, g_sealed) == AEAD_RESULT_BAD_ARGUMENT);
    TEST_ASSERT_TRUE(aead_seal(g_key, NULL, g_pt, g_sealed) == AEAD_RESULT_BAD_ARGUMENT);
    TEST_ASSERT_TRUE(aead_seal(g_key, g_nonce, NULL, g_sealed) == AEAD_RESULT_BAD_ARGUMENT);
    TEST_ASSERT_TRUE(aead_seal(g_key, g_nonce, g_pt, NULL) == AEAD_RESULT_BAD_ARGUMENT);
}

/**
 * @brief Confirm aead_open rejects every null argument.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_open_null_args(void) {
    aead_init(AEAD_ALGO_AES_256_GCM);
    TEST_ASSERT_TRUE(aead_open(NULL, g_nonce, g_sealed, g_pt) == AEAD_RESULT_BAD_ARGUMENT);
    TEST_ASSERT_TRUE(aead_open(g_key, NULL, g_sealed, g_pt) == AEAD_RESULT_BAD_ARGUMENT);
    TEST_ASSERT_TRUE(aead_open(g_key, g_nonce, NULL, g_pt) == AEAD_RESULT_BAD_ARGUMENT);
    TEST_ASSERT_TRUE(aead_open(g_key, g_nonce, g_sealed, NULL) == AEAD_RESULT_BAD_ARGUMENT);
}

/**
 * @brief Confirm nonce tracking distinguishes a changed nonce.
 *
 * @param void No parameters.
 * @return void
 */
static void test_aead_nonce_varied(void) {
    uint8_t n2[AEAD_XCHACHA_NONCE_SIZE];
    aead_init(AEAD_ALGO_AES_256_GCM);
    memcpy(n2, g_nonce, sizeof(n2));
    n2[0] ^= 0xFFu;
    TEST_ASSERT_TRUE(aead_seal(g_key, g_nonce, g_pt, g_sealed) == AEAD_RESULT_SUCCESS);
    TEST_ASSERT_TRUE(aead_seal(g_key, n2, g_pt, g_sealed) == AEAD_RESULT_SUCCESS);
}

/**
 * @brief Confirm aead_seal fails under injected EVP faults (AES-256-GCM).
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_aead_seal_gcm(void) {
    static const int faults[] = { FAULT_CTX_NEW, FAULT_ENC_INIT,
                                  FAULT_ENC_UPDATE, FAULT_ENC_FINAL, FAULT_CTRL };
    aead_init(AEAD_ALGO_AES_256_GCM);
    for (size_t i = 0u; i < sizeof(faults) / sizeof(faults[0]); ++i) {
        g_fault = faults[i];
        TEST_ASSERT_TRUE(aead_seal(g_key, g_nonce, g_pt, g_sealed) != AEAD_RESULT_SUCCESS);
    }
    g_fault = FAULT_NONE;
}

/**
 * @brief Confirm aead_open fails under injected EVP faults (AES-256-GCM).
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_aead_open_gcm(void) {
    static const int faults[] = { FAULT_CTX_NEW, FAULT_DEC_INIT,
                                  FAULT_DEC_UPDATE, FAULT_DEC_FINAL, FAULT_CTRL };
    aead_init(AEAD_ALGO_AES_256_GCM);
    for (size_t i = 0u; i < sizeof(faults) / sizeof(faults[0]); ++i) {
        g_fault = faults[i];
        TEST_ASSERT_TRUE(aead_open(g_key, g_nonce, g_sealed, g_pt) != AEAD_RESULT_SUCCESS);
    }
    g_fault = FAULT_NONE;
}

/**
 * @brief Confirm aead_seal fails under injected EVP faults (XChaCha20).
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_aead_seal_xchacha(void) {
    static const int faults[] = { FAULT_CTX_NEW, FAULT_ENC_INIT,
                                  FAULT_ENC_UPDATE, FAULT_ENC_FINAL, FAULT_CTRL };
    aead_init(AEAD_ALGO_XCHACHA20_POLY1305);
    for (size_t i = 0u; i < sizeof(faults) / sizeof(faults[0]); ++i) {
        g_fault = faults[i];
        TEST_ASSERT_TRUE(aead_seal(g_key, g_nonce, g_pt, g_sealed) != AEAD_RESULT_SUCCESS);
    }
    g_fault = FAULT_NONE;
}

/**
 * @brief Confirm aead_open fails under injected EVP faults (XChaCha20).
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_aead_open_xchacha(void) {
    static const int faults[] = { FAULT_CTX_NEW, FAULT_DEC_INIT,
                                  FAULT_DEC_UPDATE, FAULT_DEC_FINAL, FAULT_CTRL };
    aead_init(AEAD_ALGO_XCHACHA20_POLY1305);
    for (size_t i = 0u; i < sizeof(faults) / sizeof(faults[0]); ++i) {
        g_fault = faults[i];
        TEST_ASSERT_TRUE(aead_open(g_key, g_nonce, g_sealed, g_pt) != AEAD_RESULT_SUCCESS);
    }
    g_fault = FAULT_NONE;
}

/**
 * @brief Confirm the HKDF error paths trigger under an injected HMAC fault.
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_kex_hkdf(void) {
    uint8_t out[32];
    g_fault = FAULT_HMAC;
    TEST_ASSERT_FALSE(kex_hkdf(g_key, sizeof(g_key), g_nonce, 4u, g_nonce, 4u, out, sizeof(out)));
    TEST_ASSERT_FALSE(hkdf_expand(g_key, g_nonce, 4u, out, sizeof(out)));
    g_fault = FAULT_NONE;
}

/**
 * @brief Confirm the X25519 derive error paths trigger under injected faults.
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_kex_derive(void) {
    uint8_t out[KEX_KEY_SIZE];
    g_fault = FAULT_DERIVE_INIT;
    TEST_ASSERT_FALSE(kex_x25519(g_key, g_nonce, out));
    g_fault = FAULT_DERIVE_PEER;
    TEST_ASSERT_FALSE(kex_x25519(g_key, g_nonce, out));
    g_fault = FAULT_DERIVE;
    TEST_ASSERT_FALSE(kex_x25519(g_key, g_nonce, out));
    g_fault = FAULT_NONE;
}

/**
 * @brief Confirm identity_pubkey fails under an injected fault.
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_identity_pub(void) {
    uint8_t pub[IDENTITY_KEY_SIZE];
    g_fault = FAULT_GET_PUB;
    TEST_ASSERT_FALSE(identity_pubkey(g_key, pub));
    g_fault = FAULT_NONE;
}

/**
 * @brief Confirm identity_sign fails under injected faults.
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_identity_sign(void) {
    uint8_t sig[IDENTITY_SIG_SIZE];
    g_fault = FAULT_MD_CTX_NEW;
    TEST_ASSERT_FALSE(identity_sign(g_key, g_pt, 1u, sig));
    g_fault = FAULT_SIGN_INIT;
    TEST_ASSERT_FALSE(identity_sign(g_key, g_pt, 1u, sig));
    g_fault = FAULT_SIGN;
    TEST_ASSERT_FALSE(identity_sign(g_key, g_pt, 1u, sig));
    g_fault = FAULT_NONE;
}

/**
 * @brief Confirm identity_verify fails under injected faults.
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_identity_verify(void) {
    uint8_t sig[IDENTITY_SIG_SIZE];
    g_fault = FAULT_MD_CTX_NEW;
    TEST_ASSERT_FALSE(identity_verify(g_key, g_pt, 1u, sig));
    g_fault = FAULT_VERIFY_INIT;
    TEST_ASSERT_FALSE(identity_verify(g_key, g_pt, 1u, sig));
    g_fault = FAULT_VERIFY;
    TEST_ASSERT_FALSE(identity_verify(g_key, g_pt, 1u, sig));
    g_fault = FAULT_NONE;
}

/**
 * @brief Confirm identity reports failure on a wrong output length.
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_identity_len(void) {
    uint8_t pub[IDENTITY_KEY_SIZE];
    uint8_t sig[IDENTITY_SIG_SIZE];
    g_fault = FAULT_GET_PUB_LEN;
    TEST_ASSERT_FALSE(identity_pubkey(g_key, pub));
    g_fault = FAULT_SIGN_LEN;
    TEST_ASSERT_FALSE(identity_sign(g_key, g_pt, 1u, sig));
    g_fault = FAULT_NONE;
}

/**
 * @brief Confirm X25519 reports failure on a wrong output length.
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_kex_len(void) {
    uint8_t out[KEX_KEY_SIZE];
    g_fault = FAULT_DERIVE_LEN;
    TEST_ASSERT_FALSE(kex_x25519(g_key, g_nonce, out));
    g_fault = FAULT_NONE;
}

/**
 * @brief Confirm the AES-256-GCM SET_TAG control failure path is handled.
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_aead_settag_gcm(void) {
    EVP_CIPHER_CTX *c = EVP_CIPHER_CTX_new();
    uint8_t out[AEAD_PAYLOAD_SIZE];
    TEST_ASSERT_TRUE(gcm_dec_init(c, g_key, g_nonce));
    g_fault = FAULT_CTRL;
    TEST_ASSERT_EQUAL_INT(0, gcm_dec_final(c, out, g_sealed));
    g_fault = FAULT_NONE;
    EVP_CIPHER_CTX_free(c);
}

/**
 * @brief Confirm the XChaCha20 SET_TAG control failure path is handled.
 *
 * @param void No parameters.
 * @return void
 */
static void test_fault_aead_settag_chacha(void) {
    EVP_CIPHER_CTX *c = EVP_CIPHER_CTX_new();
    uint8_t out[AEAD_PAYLOAD_SIZE];
    TEST_ASSERT_TRUE(chacha_dec_init(c, g_key, g_nonce));
    g_fault = FAULT_CTRL;
    TEST_ASSERT_EQUAL_INT(0, chacha_dec_final(c, out, g_sealed));
    g_fault = FAULT_NONE;
    EVP_CIPHER_CTX_free(c);
}

/**
 * @brief Confirm a UID longer than the buffer is truncated safely.
 *
 * @param void No parameters.
 * @return void
 */
static void test_teled_init_long_uid(void) {
    char uid[TELED_UID_MAX + 8u];
    uint8_t key[32];
    memset(uid, 'x', sizeof(uid) - 1u);
    uid[sizeof(uid) - 1u] = '\0';
    memset(key, 0, sizeof(key));
    teled_init(uid, key);
    TEST_ASSERT_EQUAL_UINT8(0u, (uint8_t)teled_state()->uid[TELED_UID_MAX - 1u]);
}

/**
 * @brief Confirm the environment helpers reject null arguments.
 *
 * @param void No parameters.
 * @return void
 */
static void test_env_null_args(void) {
    TEST_ASSERT_EQUAL_UINT(0u, env_compute_crc(NULL, 64u));
    TEST_ASSERT_FALSE(env_crc_valid(NULL, 64u));
    TEST_ASSERT_TRUE(env_find(NULL, 64u, "k") == NULL);
    TEST_ASSERT_TRUE(env_find(g_env, 64u, NULL) == NULL);
    TEST_ASSERT_TRUE(env_find(g_env, 2u, "k") == NULL);
}

/**
 * @brief Confirm a valid container magic with a bad tag is rejected.
 *
 * @param void No parameters.
 * @return void
 */
static void test_container_bad_tag(void) {
    uint8_t buf[CONTAINER_HDR_SIZE] = { 0x21u, 0x84u, 0x1Bu, 0x00u };
    TEST_ASSERT_FALSE(container_valid(buf, sizeof(buf)));
}

/**
 * @brief Run the AEAD initialization and argument tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_aead_tests(void) {
    RUN_TEST(test_aead_before_init);
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
    RUN_TEST(test_env_null_args);
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
    RUN_TEST(test_container_bad_tag);
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
    RUN_TEST(test_kex_bad_args);
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
    RUN_TEST(test_identity_bad_args);
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
    RUN_TEST(test_camera_url_null);
}

/**
 * @brief Run the AEAD argument and injected-fault tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_fault_crypto_tests(void) {
    RUN_TEST(test_aead_nonce_varied);
    RUN_TEST(test_aead_seal_null_args);
    RUN_TEST(test_aead_open_null_args);
    RUN_TEST(test_fault_aead_seal_gcm);
    RUN_TEST(test_fault_aead_open_gcm);
    RUN_TEST(test_fault_aead_seal_xchacha);
    RUN_TEST(test_fault_aead_open_xchacha);
}

/**
 * @brief Run the KEX and identity injected-fault tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_fault_identity_tests(void) {
    RUN_TEST(test_fault_kex_hkdf);
    RUN_TEST(test_fault_kex_derive);
    RUN_TEST(test_fault_identity_pub);
    RUN_TEST(test_fault_identity_sign);
    RUN_TEST(test_fault_identity_verify);
}

/**
 * @brief Run the remaining length and control fault tests.
 *
 * @param void No parameters.
 * @return void
 */
static void run_fault_extra_tests(void) {
    RUN_TEST(test_fault_identity_len);
    RUN_TEST(test_fault_kex_len);
    RUN_TEST(test_fault_aead_settag_gcm);
    RUN_TEST(test_fault_aead_settag_chacha);
    RUN_TEST(test_teled_init_long_uid);
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

int main(void) {
    TEST_BEGIN();
    run_core_groups();
    run_data_groups();
    run_model_groups();
    run_fault_crypto_tests();
    run_fault_identity_tests();
    run_fault_extra_tests();
    return TEST_END();
}
