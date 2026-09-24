# Walkthrough 03: `env`

**every line of `src/env.c`, explained**

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

`env.c` reads the U-Boot environment (`docs/07`): a CRC32 followed by `key=value\0` text.
Three functions:

```
env_read32      (private)   little-endian word load
env_compute_crc (public)    CRC32 over the key/value blob
env_crc_valid   (public)    compare stored vs computed
env_find        (public)    locate a key, return its value
```

## 1. The Format, Restated

```
offset 0x00 : CRC32 (4 bytes LE)
offset 0x04 : key=value\0key=value\0...
```

`ENV_CRC_SIZE` is `4u` (`include/env.h`).

## 2. `env_read32`

```c
static uint32_t env_read32(const uint8_t *src) {
    return (uint32_t)src[0] | ((uint32_t)src[1] << 8u) |
           ((uint32_t)src[2] << 16u) | ((uint32_t)src[3] << 24u);
}
```

Same byte-wise little-endian load as `partition.c` (`docs/walkthrough/02`), for the same
reason: no alignment trap.

## 3. `env_compute_crc`

```c
uint32_t env_compute_crc(const uint8_t *env, size_t len) {
    if ((env == NULL) || (len < ENV_CRC_SIZE)) return 0u;
    return crc32_uboot(&env[ENV_CRC_SIZE], len - ENV_CRC_SIZE);
}
```

| line | what it does |
| ---- | ------------ |
| guard | reject null / too-short |
| `&env[4]` | skip the CRC field |
| `len - 4` | cover the rest of the blob |
| `crc32_uboot` | the **standard** CRC32 (`docs/03`) |

The guard returns `0u` for a bad input. Note that `0u` is a valid CRC value, so callers
must distinguish "bad input" from "CRC is zero" themselves - `env_crc_valid` does the
length check first.

## 4. `env_crc_valid`

```c
bool env_crc_valid(const uint8_t *env, size_t len) {
    if ((env == NULL) || (len < ENV_CRC_SIZE)) return false;
    return env_read32(env) == env_compute_crc(env, len);
}
```

Compares the **stored** CRC (first word) with the **computed** CRC. This is the function
that decides whether U-Boot will honour your edit (`docs/07`).

## 5. `env_find` - a small parser

```c
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

| line | what it does |
| ---- | ------------ |
| guard | reject null / too-short |
| `klen = strlen(key)` | the key length |
| `for (i = 4; i + klen + 1 < len; ++i)` | slide a window over the blob |
| `env[i + klen] == '='` | the char after the key must be `=` |
| `memcmp(env + i, key, klen) == 0` | and the key must match |
| `return &env[i + klen + 1]` | return the value (after the `=`) |

The loop bound `(i + klen + 1u) < len` guarantees both the `=` at `i+klen` and at least
one value byte at `i+klen+1` are in range - no out-of-bounds read.

**Simplification.** This is a scan, not a full U-Boot parser: it does not require the
match to start at a `key` boundary (after a `\0`). For the lab that is fine; note it so
you do not reuse it where boundaries matter.

## 6. AArch64

The compiler turns `env_find`'s loop into a byte compare plus a `memcmp` call. The CRC
path tail-calls `crc32_uboot`, which is inlined with the `0xEDB88320` constant
(`docs/walkthrough/01`).

## 7. Tests

```bash
$ python3 scripts/run_tests.py | grep env
:test_env_crc_valid:PASS
:test_env_crc_invalid:PASS
:test_env_short:PASS
```

- `valid` builds a blob, seals it, and expects `env_crc_valid == true` **and**
  `env_find("bootargs")` to return the value.
- `invalid` builds a blob **without** sealing and expects `false`, and a missing key to
  return `NULL`.
- `short` exercises the `len < 4` guards on both the CRC and the find paths.

## 8. The CRC Discipline (the point)

The entire module exists so that a lab can **edit the environment safely**:
change the bytes, call `env_compute_crc`, write it back, verify with `env_crc_valid`.
Skip it and U-Boot ignores you (`docs/07`).

## Exercises

1. Extend `env_find` to require the match to start at a key boundary.
2. Explain why `env_compute_crc` returning `0u` on a bad input is a mild footgun.
3. Build a two-key blob, seal it, and find both keys.
4. Flip one blob byte and show `env_crc_valid` fails.

## Reference

- `src/env.c`, `include/env.h`, `src/crc.c`
- `docs/07` (environment CRC)
