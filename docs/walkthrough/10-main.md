# Walkthrough 10: `main`

**every line of `src/main.c`, explained**

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

## Why This Module

`main.c` is the **entry point**: it selects the AEAD backend, initialises the daemon, and
emits one beacon. It is where the boot chain (`docs/05`-`docs/13`) meets the code. It is
deliberately short so the wiring is obvious.

## 1. The Whole File

```c
#include "aead.h"
#include "teled.h"
#include <string.h>

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

## 2. Line by Line

| line | what it does |
| ---- | ------------ |
| `#include "aead.h"` | the AEAD API (backend selection + seal) |
| `#include "teled.h"` | the daemon |
| `#include <string.h>` | `memset` |
| `uint8_t key[32];` | a 32-byte key buffer on the stack |
| `uint8_t payload[AEAD_PAYLOAD_SIZE];` | the beacon payload buffer |
| `memset(key, 0, ...)` | zero the key |
| `memset(payload, 0, ...)` | zero the payload |
| `aead_init(AEAD_ALGO_AES_256_GCM)` | select AES-256-GCM (`docs/19`) |
| `teled_init("SSAT-468547-FEEBD", key)` | set the UID and key; reset the collector |
| `return teled_beacon(payload) ? 0 : 1;` | seal one beacon, return success/fail |

## 3. Why These Choices

**Why zero the key?** This is a **lab stub**. In a real device the key would be an
HKDF-derived session key (`docs/21`) or a provisioned secret (`docs/17` discusses why a
public-derived one is wrong). A zero key here would make the beacon trivially
decryptable - which is honest for a stub that demonstrates the *mechanism*, and is why
`main` is excluded from coverage.

**Why one beacon, not a loop?** The lab runs one cycle per invocation and tests the
seal/open path. A production daemon loops on a timer; the loop is scheduling, not
crypto, and lives in the daemon (`docs/walkthrough/09`).

**Why AES-256-GCM?** The RP5 has the ARMv8 crypto extensions, so AES-GCM is the
hardware-accelerated default (`docs/19`). Switching to
`AEAD_ALGO_XCHACHA20_POLY1305` is a one-line change (`docs/20`).

## 4. The Exit Code

`return teled_beacon(payload) ? 0 : 1;` - `main` returns `0` on success and `1` on
failure, the C convention. The shell and CI can check it.

## 5. Where It Sits in the Boot Chain

```
VideoCore -> U-Boot -> Linux Image -> /sbin/init -> rcS -> S80network
    -> /etc/starts -> run -> teled
        -> main()
```

`main` is the end of the boot chain (`docs/10`, `docs/13`) and the start of the
application.

## 6. Not Covered, on Purpose

`main` is **excluded from the coverage gate** (the reference repo excludes its entry
point too). Rationale: an entry point is exercised by the operating system, not by the
unit suite; testing it in-process would mean calling `main` recursively. Its *logic* -
the seal and the daemon - is covered in `teled` and `aead`.

## 7. AArch64

Compiled for AArch64, `main` is a small frame: it reserves the stack buffers, zeroes
them, calls `aead_init`, `teled_init`, and `teled_beacon`, and returns. The `bl` targets
are the interesting part: `main` is a **caller**, so its disassembly is a map of the
whole application.

## Exercises

1. Change the backend to XChaCha20-Poly1305 and confirm the beacon still seals.
2. Replace the zero key with an HKDF key and explain the security difference.
3. Add a loop that emits five beacons and observe `seq`.
4. Explain why `main` is excluded from coverage and what covers its logic instead.

## Reference

- `src/main.c`, `src/teled.c`, `src/aead.c`
- `docs/13` (userland boot), `docs/18`-`docs/21` (crypto)
