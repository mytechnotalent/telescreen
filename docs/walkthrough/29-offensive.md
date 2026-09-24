# Walkthrough 29: Offensive Techniques

**the attacker's toolkit, class by class**

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

## The Map

| class | primitive | lab defect |
| ----- | --------- | ---------- |
| info leak | read a file | B3 (`d.cgi`-style dump) |
| command exec | `system()` from input | B2 |
| path write | archive extraction | B3 |
| auth bypass | empty/default creds | B4 |
| local priv | debug shell | B5 |
| crypto | key from public data | B6 |
| memory | overflow a parser | (the V1 class) |

Each class is a **capability ladder**: info leak -> write -> execute -> persist.

## 1. Info Leak

The first rung. A handler that reads a file the caller should not get:

```
GET /dump?file=/etc/shadow
```

On real devices this is `d.cgi`-style endpoints that dump `/dev/mtdN`. The output is a
**firmware image** to analyse offline.

**Detect:** enumerate handlers; look for ones that read `/dev/`, `/proc/`, or config.

## 2. Command Execution

The core rung. Any `system()`/`popen()` that receives input:

```c
sprintf(buf, "ping %s -c 1", host);   /* host = "1.1.1.1; id" */
system(buf);
```

Fix: `execve` with an argument vector (`docs/15`).

**Detect:** every `system()` caller; trace its inputs.

## 3. Path Write

Write anywhere as root, without executing yet:

```
restore an archive with ../../etc/init.d/S99x
```

**Detect:** any extraction (`tar -C /`, `unzip -d /`, `cp -r`), validated for `../`.

## 4. Auth Bypass

Skip authentication entirely:

```
GET /admin  ->  no password configured
```

**Detect:** empty credential stores, default creds, `.htpasswd` that never rejects.

## 5. Local Privilege

A debug path that yields a shell:

```
serial getty with a blank root password
telnetd left enabled
```

**Detect:** unexpected listeners, `telnetd`, debug flags.

## 6. Cryptographic Weakness

Not "break AES". Break the **key management**:

```
key = derive(public_uid)      /* docs/17 */
```

**Detect:** find the key source; if it is public, the crypto is theatre.

## 7. Memory Corruption

The highest rung and the one that needs reverse engineering:

- an over-long URL into a fixed stack buffer (the RTSP class);
- a length field trusted without a bound;
- a `strcpy`/`sprintf` with attacker input.

**Exploit shape on ARMv7-A/ARMv8:** overflow the saved `LR`, control `PC`, and use
**ret2libc** into `system` (uClibc has no PIE on many devices). The steps:

```
1. reachability (a network port, pre-auth)
2. crash it (grow the input, watch the console)
3. find the offset (cyclic pattern -> LR value)
4. choose a target (system; check NX/ASLR; set the Thumb bit if needed)
5. build the payload (ret2libc or a short ROP chain)
```

**Detect:** check every parser for fixed buffers and unchecked `strcpy`/`sprintf`.

## 8. Persistence

Once root, stay root:

- write the flash (`docs/12`, `docs/27`);
- add a config payload (the `wifi.conf`-source class, B1);
- add an init script.

**Detect:** watch the config files and the flash for writes.

## 9. The Ladder in One Diagram

```
 info leak -----> path write -----> command exec -----> root
     |                                    |                |
     v                                    v                v
  firmware                            shell            persist
  to analyse                          as root          in flash
```

A defence only counts if it breaks the ladder at the **lowest** rung it can: no info leak,
no path write, no exec.

## Exercises

1. For each class, name the lab defect and the fix.
2. Walk the ladder from `/dump` to persistence on the lab.
3. For the memory class, describe the ret2libc shape.
4. Name one defence that breaks the ladder at rung 2.

## Reference

- `docs/15`, `docs/16`, `docs/17`, `docs/28`
- `docs/walkthrough/16` (the solve path)
