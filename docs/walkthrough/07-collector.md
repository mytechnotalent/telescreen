# Walkthrough 07: `collector`

**every line of `src/collector.c`, explained**

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

`collector.c` is the **local lab sink** (`PARTS.md`, the isolated-lab rule). Every beacon
the daemon seals is handed here, and the collector keeps a tiny accounting record. It is
local-only by design: a lab, not the internet.

```
collector_init   (public)   zero the counters
collector_accept (public)   accept or reject one frame
collector_stats  (public)   read the accounting snapshot
```

## 1. The State

```c
static collector_stat_t g_stats;
```

`collector_stat_t` (`include/collector.h`) is:

```
u32 accepted
u32 rejected
u32 last_seq
```

A single file-scope struct. `g_` marks it as file-global state (the house convention).

## 2. `collector_init`

```c
void collector_init(void) {
    g_stats.accepted = 0u;
    g_stats.rejected = 0u;
    g_stats.last_seq = 0u;
}
```

Zeroes the three fields. Called at daemon init (`docs/walkthrough/12`).

## 3. `collector_accept`

```c
bool collector_accept(const uint8_t *sealed, size_t len, uint32_t seq) {
    if ((sealed == NULL) || (len != AEAD_SEALED_SIZE)) {
        g_stats.rejected += 1u;
        return false;
    }
    g_stats.accepted += 1u;
    g_stats.last_seq = seq;
    return true;
}
```

| line | condition | effect |
| ---- | --------- | ------ |
| guard | null or wrong length | `rejected++`, return false |
| success | a full sealed frame | `accepted++`, record `seq`, return true |

The length check is `len != AEAD_SEALED_SIZE` - **exact**, not a minimum. A sealed frame
is exactly `AEAD_PAYLOAD_SIZE + AEAD_TAG_SIZE` bytes (`include/aead.h`). Accepting
anything shorter would mean silently reading past the buffer.

**What this collector does not do:** it does not decrypt. It counts and records. That is
deliberate - the collector is the **blue-team** vantage point (`docs/28`): it sees the
beacons arrive and can measure their rate and size without needing the key.

## 4. `collector_stats`

```c
const collector_stat_t *collector_stats(void) {
    return &g_stats;
}
```

Returns a `const` pointer to the state, so callers can read but not mutate. A common,
safe accessor pattern.

## 5. AArch64

`collector_accept` compiles to a null/length test, two increments (an `ldr`/`add`/`str`
sequence on the global), and a return. Nothing exotic - and that is fine. Not every
function is a crypto routine.

## 6. Tests

```bash
$ python3 scripts/run_tests.py | grep collector
:test_collector_accept:PASS
:test_collector_reject:PASS
```

- `accept` sends a full-size frame and checks `accepted == 1` and `last_seq == 5`.
- `reject` sends `NULL` and a too-short frame and checks `rejected == 2`.

## 7. Where It Sits

```
teled_beacon  ->  beacon_seal  ->  collector_accept  ->  g_stats
   (docs/12)       (docs/06)         (this module)       (blue team)
```

The collector is the **sink** of the exfiltration path. In the story it is the Ministry's
"memory hole"; in the lab it is your bench.

## Exercises

1. Accept three frames with sequences 1, 2, 3 and read back `last_seq`.
2. Send a frame one byte short and confirm it is rejected.
3. Explain why the collector does not need the key to be useful.
4. Add a `bytes` counter and keep the module at 100% coverage.

## Reference

- `src/collector.c`, `include/collector.h`
- `docs/28` (blue team), `docs/walkthrough/06` (beacon)
