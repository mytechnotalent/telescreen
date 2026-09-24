# Walkthrough 34: The Daemon, End to End

**how every module composes into one running device**

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

## The Composition

`teled` is not one program; it is a **composition** of the modules you have read. This
volume shows the composition, the startup order, and the runtime loops.

## 1. Startup Order

```
main()
  memset(key, 0, 32)                 ; a lab stub key
  memset(payload, 0, 48)             ; a zero payload
  aead_init(AEAD_ALGO_AES_256_GCM)   ; select the backend          [aead]
  teled_init(uid, key)               ; identity + key + collector  [teled]
    memset(state)                    ; reset the daemon state
    strncpy(uid)                     ; copy the public UID
    memcpy(key)                      ; copy the session key
    collector_init()                 ; reset the sink              [collector]
  teled_beacon(payload)              ; one cycle
    seq++                            ; the nonce source
    beacon_seal(key, seq, ...)       ; seal                        [beacon -> aead]
    collector_accept(sealed, ...)    ; deliver                     [collector]
```

Every arrow is a module you have read. The daemon adds nothing but **ordering**.

## 2. The Runtime Loops (what a product adds)

A lab runs one cycle. A product runs loops:

| loop | period | job |
| ---- | ------ | --- |
| beacon | ~60 s | `teled_beacon` (`docs/walkthrough/09`) |
| camera | continuous | capture UVC, encode, serve RTSP (`docs/25`) |
| HTTP | on request | dispatch routes (`docs/14`) |
| router | continuous | NAT, DHCP, DNS (`docs/24`) |
| watchdog | ~seconds | feed the watchdog |

The lab keeps the **beacon** loop's logic and elides the rest into prose, because the
security lessons live in the beacon and the dispatcher.

## 3. The Module Dependencies

```
main
 └── teled
      ├── beacon ── aead ── (OpenSSL)
      └── collector
 └── aead ── (OpenSSL)
kex ── (OpenSSL, HMAC)
identity ── (OpenSSL)
partition ── (memcpy)
env ── crc
jffs2 ── crc
container ── (memcmp)
camera ── (snprintf)
```

Read bottom-up: the leaves (`crc`, `aead`) are tested alone; the composition (`teled`,
`main`) is tested through them.

## 4. The State

| state | module | lifetime |
| ----- | ------ | -------- |
| `g_aead_algo`, `g_aead_ready`, `g_last_nonce` | aead | process |
| `g_teled` (uid, key, seq, beacons) | teled | process |
| `g_stats` (accepted, rejected, last_seq) | collector | process |
| `g_parts[4]` | partition | static const |
| `g_msg`, test fixtures | tests | test case |

Everything is **process state**. No heap, no globals outside a module. That is the house
discipline and it is why the code is easy to reason about.

## 5. Which Module Owns Which Decision

| decision | owner |
| -------- | ----- |
| which cipher | `aead_init` (the caller of `main`) |
| where the key comes from | the **caller**, not the daemon |
| the nonce | `beacon_nonce` (from the sequence) |
| the tag check | `aead_open` |
| delivery | `collector_accept` |

**The key insight:** `teled` does **not** choose the crypto posture. The **key source**
does. Feed it `beacon_weak_key` and it is the flawed Ministry beacon; feed it `kex_hkdf`
and it is the hardened one. Same daemon, opposite security.

## 6. The Failure Paths

| failure | handling |
| ------- | -------- |
| `aead_init` bad backend | `main` still proceeds; seal will fail |
| `beacon_seal` fails | `teled_beacon` returns false; **not counted** |
| `collector_accept` wrong size | rejected, counted |
| null payload | seal rejects, nothing counted |

Every failure is **local and counted**, not a crash. That is the difference between a lab
and a device that reboots on a bad packet.

## 7. Extending the Daemon

Want a new feature? Add a module, test it to 100%, then wire it in `teled`:

1. write `src/<feature>.c` + `include/<feature>.h` (house style, eight-line bodies);
2. add tests to `test/test_telescreen.c`;
3. add it to `_owned_sources()` in `run_tests.py` and the coverage list;
4. run the three gates;
5. call it from `teled_init` or a loop.

That is the whole contribution process.

## 8. The Composition Lesson

A device is **not** a monolith; it is a handful of small, tested parts and the wiring
between them. If you can read the parts, you can read the device. If the parts are
tested, you can change the device safely. That is what this repo demonstrates.

## Exercises

1. Draw the module dependency graph from memory.
2. Add a `feature` module with one function, test it, and pass the gates.
3. Change `main` to use XChaCha and run the suite.
4. Explain why the daemon does not own the key decision.

## Reference

- `docs/walkthrough/09`, `docs/walkthrough/10`, `docs/walkthrough/11`
- `docs/modules/*`, `docs/appendix/G`
