# Walkthrough 09: `teled`

**every line of `src/teled.c`, explained**

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

`teled` is the TELESCREEN **daemon**: it owns the device identity, the session key, and
the beacon sequence, and it hands sealed beacons to the collector. It is the integration
point that ties `beacon` (`docs/walkthrough/06`) and `collector`
(`docs/walkthrough/07`) together.

```
teled_init    (public)   set the UID and key; reset the collector
teled_beacon  (public)   seal one beacon, deliver it, count it
teled_state   (public)   read the daemon state
```

## 1. The State

```c
static teled_state_t g_teled;
```

`teled_state_t` (`include/teled.h`):

```
char     uid[32]
uint8_t  key[32]
uint32_t seq
uint32_t beacons
```

`uid` is the public device identifier, `key` is the hardened session key, `seq` is the
monotonic beacon counter, and `beacons` counts emissions.

## 2. `teled_init`

```c
void teled_init(const char *uid, const uint8_t key[32]) {
    memset(&g_teled, 0, sizeof(g_teled));
    strncpy(g_teled.uid, uid, sizeof(g_teled.uid) - 1u);
    memcpy(g_teled.key, key, 32u);
    collector_init();
}
```

| line | what it does |
| ---- | ------------ |
| `memset` | zero the whole state first |
| `strncpy(..., sizeof(uid) - 1)` | copy the UID, leaving room for a NUL |
| `memcpy(key, ..., 32)` | copy the 32-byte session key |
| `collector_init()` | reset the collector counters |

The `- 1u` is the important detail: `strncpy` does **not** guarantee a NUL when the source
is at least as long as `n`. Reserving one byte and relying on the earlier `memset` to have
zeroed it means the string is always terminated. This is the safe idiom for a fixed
ID field.

> `strncpy` also does **not** pad a short source with spaces here in a harmful way - the
> state is already zeroed, so the copy is clean.

## 3. `teled_beacon`

```c
bool teled_beacon(const uint8_t *payload) {
    uint8_t sealed[AEAD_SEALED_SIZE];
    g_teled.seq += 1u;
    if (!beacon_seal(g_teled.key, g_teled.seq, payload, sealed)) return false;
    g_teled.beacons += 1u;
    return collector_accept(sealed, sizeof(sealed), g_teled.seq);
}
```

| line | what it does |
| ---- | ------------ |
| `uint8_t sealed[...]` | a stack buffer for the ciphertext + tag |
| `g_teled.seq += 1u` | bump the **monotonic** sequence (the nonce source) |
| `beacon_seal(key, seq, payload, sealed)` | seal; the sequence becomes the nonce |
| `if (!...) return false;` | a seal failure stops here; do **not** count |
| `g_teled.beacons += 1u` | count a successful seal |
| `collector_accept(..., seq)` | hand the sealed frame to the sink |

Two invariants:

1. **The sequence is bumped before sealing**, so the nonce is unique even if the seal
   fails - the next attempt uses a fresh number. Never reuse a nonce (`docs/18`).
2. **The count increments only on success**, so `beacons` means "sealed", not "attempted".

## 4. `teled_state`

```c
const teled_state_t *teled_state(void) {
    return &g_teled;
}
```

A `const` view of the state for tests and diagnostics.

## 5. The Data Flow

```
teled_beacon(payload)
   seq++                       (unique nonce source)
   beacon_seal(key, seq, ...)  (docs/walkthrough/06)
      aead_seal(...)           (docs/walkthrough/10)
   collector_accept(sealed)    (docs/walkthrough/07)
      g_stats.accepted++
```

This is the **whole application** in miniature: a monotonic counter, a seal, and a sink.
Everything else in the daemon is scheduling and I/O around this core.

## 6. Tests

```bash
$ python3 scripts/run_tests.py | grep teled
:test_teled_beacon:PASS
:test_teled_beacon_fail:PASS
```

- `beacon` initialises the daemon, seals one payload, and checks `beacons == 1` and the
  UID round-tripped.
- `beacon_fail` passes `NULL` and expects `false` (the seal rejects it, so nothing is
  counted).

## 7. The Security Note

`teled` uses **whatever key it is given**. With `beacon_weak_key` it is the flawed
Ministry beacon; with an HKDF session key it is the hardened one. The daemon does not
choose the crypto posture - the **key source** does (`docs/17`). That separation is
intentional and teaches the right lesson.

## Exercises

1. Call `teled_beacon` three times and confirm `seq == 3` and `beacons == 3`.
2. Show that a failed seal does not advance `beacons`.
3. Explain why `seq` is bumped before sealing.
4. Feed the daemon an HKDF key and show it still works.

## Reference

- `src/teled.c`, `include/teled.h`, `src/beacon.c`, `src/collector.c`
- `docs/13` (userland boot), `docs/17` (weak KDF)
