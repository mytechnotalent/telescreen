# Walkthrough 23: Data Flows

**trace a beacon and a web request end to end**

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

## Why Trace Data

A vulnerability is a **path** that data takes from a source you do not control to a sink you
should. Reading functions in isolation hides the path. Tracing data makes it visible. This
volume traces two flows: the **beacon** (outbound) and a **web request** (inbound).

## Flow 1: The Beacon (outbound telemetry)

```
boot -> teled_init(uid, key)
loop:
  teled_beacon(payload)
    g_teled.seq += 1
    beacon_seal(key, seq, payload, sealed)
      beacon_nonce(seq, nonce)
      aead_seal(key, nonce, payload, sealed)   [AES-256-GCM]
    collector_accept(sealed, size, seq)
      g_stats.accepted += 1
```

Step by step, with the module:

| step | function | module | volume |
| ---- | -------- | ------ | ------ |
| 1 | `teled_init` | teled | `docs/walkthrough/09` |
| 2 | `teled_beacon` | teled | `docs/walkthrough/09` |
| 3 | `beacon_nonce` | beacon | `docs/walkthrough/06` |
| 4 | `aead_seal` | aead | `docs/walkthrough/11` |
| 5 | `collector_accept` | collector | `docs/walkthrough/07` |

**The security decision is at step 1:** where does `key` come from? If it came from
`beacon_weak_key(uid)` (`docs/17`), the flow is **obfuscated**, not encrypted. If it came
from `kex_hkdf(...)` (`docs/walkthrough/12`), it is protected. The flow is identical; the
**key source** decides everything.

**The nonce discipline is at step 3:** the nonce is the sequence, and the sequence is
monotonic, so it never repeats under one key (`docs/18`).

## Flow 2: A Web Request (inbound)

```
client -> TCP 80
  teled HTTP server parses the request
    dispatcher looks up the route           [docs/14]
      handler
        reads request parameters
        builds a response, maybe with system()   [docs/15]
        returns bytes
```

Trace a dangerous variant:

```
GET /restore.cgi  (multipart body = archive)
  dispatcher -> restore handler
  handler writes the archive to /tmpfs/restore.bin
  handler: system("tar -xvzf /tmpfs/restore.bin -C /")
  archive contains ../../etc/init.d/S99evil
  -> root file write -> code execution on next boot   [docs/16, B3]
```

The **source** is the request body. The **sink** is `tar -C /`. The **bug** is that the
archive contents are not validated.

## Flow 3: The Config Write (the B1 path)

```
anything that can write conf/<file>
  -> the file is sourced as root at boot
  -> root code execution
```

The **source** is a file write (however it is achieved). The **sink** is `. <file>` in a
root shell. This is the design flaw B1 (`docs/16`).

## The Taint Method

```
1. For each sink (system, open, write, execve), list it.
2. For each sink, find its inputs.
3. For each input, trace back to a source (request, file, env, network).
4. If a source is attacker-controlled, it is a finding.
```

This is the same method from `docs/15`, applied to the whole system.

## The Data-Flow Diagram

```
   [network]                                      [flash]
       |                                             |
       v                                             v
  request body ---> handler ---> system(buf)      config file
       |                |             |                |
       |                |             v                v
       |                |        shell command    sourced as root
       |                v                                |
       |           response                              v
       |                                             root code
       v
  sealed beacon ---> collector

   the two dangerous sinks:  system()  and  the sourced config
```

## Exercises

1. Trace a `/param.cgi` request from socket to response, naming each function.
2. For B3, write the taint path from `GET /restore.cgi` to `tar -C /`.
3. For B1, write the taint path from "file write" to root execution.
4. Rewrite the beacon flow with an HKDF key and state what changes (only the key source).

## Reference

- `docs/14`, `docs/15`, `docs/16`, `docs/17`
- `docs/walkthrough/06`, `docs/walkthrough/07`, `docs/walkthrough/09`, `docs/walkthrough/11`
