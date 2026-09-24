# Volume 07: Environment CRC

**reading and safely editing bootargs**

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

## What You'll Learn

- The U-Boot environment format (a CRC32 followed by `key=value\0` text)
- The real `env.c` implementation and its AArch64 behavior
- How to edit the environment **safely** (recompute the CRC or U-Boot ignores you)
- Why editing the environment is a lab skill

## 1. The Format

The `bootargs` partition (partition 1, 64 KiB) begins with:

```
offset 0x00 : CRC32 (4 bytes, little-endian)
offset 0x04 : key=value\0key=value\0...
```

The CRC covers the `key=value` blob (everything from offset 0x04 to the end). U-Boot
verifies it at boot; if it does not match, U-Boot **falls back to built-in defaults**
and your edit is silently ignored. This is the trap.

## 2. Read It

```bash
strings -n4 CTF-XX-env.img | head
# baudrate=115200
# bootargs=...
# bootcmd=...
```

## 3. The Real Source - `include/env.h`

```c
#define ENV_CRC_SIZE 4u

uint32_t env_compute_crc(const uint8_t *env, size_t len);
bool     env_crc_valid(const uint8_t *env, size_t len);
const char *env_find(const uint8_t *env, size_t len, const char *key);
```

## 4. The Real Source - `src/env.c`

```c
uint32_t env_compute_crc(const uint8_t *env, size_t len) {
    if ((env == NULL) || (len < ENV_CRC_SIZE)) return 0u;
    return crc32_uboot(&env[ENV_CRC_SIZE], len - ENV_CRC_SIZE);
}

bool env_crc_valid(const uint8_t *env, size_t len) {
    if ((env == NULL) || (len < ENV_CRC_SIZE)) return false;
    return env_read32(env) == env_compute_crc(env, len);
}
```

Note the use of the **standard** CRC32 (`crc32_uboot`, `docs/03`), not the JFFS2
`crc32_le`. Using the wrong CRC is the classic environment-editing failure.

## 5. Edit It Safely

The rule: after changing any byte in the blob, **recompute the leading CRC32** before
writing the partition back.

```python
import struct
body = open("env_body.bin", "rb").read()          # the k=v\0 blob
crc = crc32_uboot(body)
open("CTF-XX-env.img", "wb").write(struct.pack("<I", crc) + body)
```

If you skip this step, U-Boot boots with the wrong command line. The `env_crc_valid`
test in the suite exists precisely to catch that.

## 6. Why This Is a Lab Skill

Several labs need an environment change:

- Set `init=/bin/sh` to drop to a root shell early.
- Point `root=` at a different partition.
- Disable a feature by deleting a `key=value`.

All of them require the CRC discipline. It is the same discipline as the JFFS2 patch
(`docs/12`): **change the bytes, then fix the integrity field.**

## 7. Check Values

```bash
$ python3 scripts/run_tests.py | grep env
:test_env_crc_valid:PASS
:test_env_crc_invalid:PASS
:test_env_short:PASS
```

## Labs

1. Read `bootargs` and `bootcmd` from `CTF-XX-env.img`.
2. Modify `bootdelay` to `3`, recompute the CRC, and show `env_crc_valid` passes.
3. Modify a byte **without** fixing the CRC and show it fails.
4. Explain why U-Boot falls back to defaults rather than refusing to boot.

## Reference

- `src/env.c`, `include/env.h`, `src/crc.c`
- `docs/03` (CRC), `docs/06` (U-Boot)
