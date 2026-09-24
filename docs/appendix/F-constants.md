# Appendix F: Constants, Configs, and Check Values

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

## Partition constants

#define PART_IMAGE_SIZE 0x1000000u
#define PART_BOOT_OFF 0x000000u
#define PART_BOOT_SIZE 0x020000u
#define PART_ENV_OFF 0x020000u
#define PART_ENV_SIZE 0x010000u
#define PART_KERNEL_OFF 0x030000u
#define PART_KERNEL_SIZE 0x1C0000u
#define PART_ROOTFS_OFF 0x1F0000u
#define PART_ROOTFS_SIZE 0xE10000u
#define PART_BOOT_MAGIC 0xEA000515u
#define PART_CONTAINER_MAGIC 0x001B8421u
#define PART_JFFS2_MAGIC 0x1985u

## AEAD constants

#define AEAD_H
#define AEAD_AES_KEY_SIZE 32u
#define AEAD_AES_NONCE_SIZE 12u
#define AEAD_XCHACHA_KEY_SIZE 32u
#define AEAD_XCHACHA_NONCE_SIZE 24u
#define AEAD_TAG_SIZE 16u
#define AEAD_PAYLOAD_SIZE 48u
#define AEAD_SEALED_SIZE (AEAD_PAYLOAD_SIZE + AEAD_TAG_SIZE)

## KEX / identity constants

#define KEX_H
#define KEX_KEY_SIZE 32u
#define KEX_OKM_SIZE 32u
#define IDENTITY_H
#define IDENTITY_KEY_SIZE 32u
#define IDENTITY_SIG_SIZE 64u

## JFFS2 / container constants

#define JFFS2_H
#define JFFS2_MAGIC 0x1985u
#define JFFS2_HDR_SIZE 12u
#define JFFS2_TYPE_DIRENT 0xE001u
#define JFFS2_TYPE_INODE 0xE002u
#define JFFS2_TYPE_CLEANMARKER 0x2003u
#define JFFS2_TYPE_PADDING 0x2004u
#define CONTAINER_H
#define CONTAINER_MAGIC 0x001B8421u
#define CONTAINER_HDR_SIZE 16u

## Published check values

```
crc32_uboot("123456789") = 0xCBF43926
crc16_ccitt("123456789") = 0x29B1
X25519 shared (RFC 7748)  = 4a5d9d5ba4ce2de1728e3bf480350f25...
HKDF-SHA256 OKM (RFC 5869)= 3cb25f25faacd57a90434f64d0362f2a...
Ed25519 sig  (RFC 8032)   = e5564300c360ac729086e2cc806e828a...
```
