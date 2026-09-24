# Walkthrough 48: Case Studies

**how the lab's lessons appear in real device classes**

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

## How to Read a Case Study

Each case follows the same shape: **target, discovery, root cause, impact, fix, lesson.**
The details are illustrative of the *class*; the method is what transfers.

## Case 1: The Dump Endpoint

**Target.** A camera whose web UI offers a "download diagnostics" button.
**Discovery.** The button calls an internal route that streams a file. The filename is a
request parameter.

```
GET /diag?file=/dev/mtd3
```

**Root cause.** The handler reads any path and returns it. No allowlist.
**Impact.** An attacker downloads the entire flash, including configs and keys.
**Fix.** Serve only whitelisted files from a fixed directory; never take a path.
**Lesson.** This is **info leak**, the first rung (`docs/walkthrough/29`). It hands the
attacker the image to analyse.

## Case 2: The Ping Tool

**Target.** A router admin page with a "ping" utility.
**Discovery.** The host field is concatenated into a shell command.

```
host = "1.1.1.1; id"
system("ping 1.1.1.1; id -c 1")
```

**Root cause.** `system()` from input (`docs/15`).
**Impact.** Command execution as root.
**Fix.** `execve` with an argument vector.
**Lesson.** The most common embedded root bug. It is not exotic.

## Case 3: The Config Restore

**Target.** A device that lets you upload and restore a configuration backup.
**Discovery.** The restore runs `tar -xvzf <upload> -C /` as root.
**Root cause.** Archive contents are trusted (`docs/16`, B3).
**Impact.** A crafted archive writes `/etc/init.d/S99x` -> root on next boot.
**Fix.** Reject `../` and absolute entries; extract sandboxed; drop privileges.
**Lesson.** "Fixed path" does not mean "safe": the **contents** are the attack.

## Case 4: The Default Login

**Target.** A camera that ships with `admin/admin`.
**Discovery.** The credential store is a config file with a default.
**Root cause.** Shipped defaults (`docs/16`, B4).
**Impact.** Admin access to every config setter.
**Fix.** Force a password at first boot; store an Argon2id hash.
**Lesson.** Convenience defaults are a permanent vulnerability across a fleet.

## Case 5: The Cloud Key

**Target.** A camera that reaches clients through a vendor relay.
**Discovery.** The session key is derived from the device's public ID.
**Root cause.** Key management, not the cipher (`docs/17`).
**Impact.** Anyone with the public ID decrypts the "sealed" traffic.
**Fix.** X25519 + HKDF + a real AEAD (`docs/21`, `docs/19`).
**Lesson.** Ask **where the key comes from**, always.

## Case 6: The Parser Overflow

**Target.** A device with a pre-auth network parser.
**Discovery.** A URL/field copied into a fixed stack buffer with no bound.
**Root cause.** `strcpy`/`sprintf` on attacker input.
**Impact.** Control of the return address -> potential remote root.
**Fix.** Bounded copies; authenticate before parsing.
**Lesson.** The highest rung. It needs reverse engineering, but the *cause* is the same
class of mistake as the rest.

## The Commonalities

| case | class | the one-word cause |
| ---- | ----- | ------------------ |
| 1 | info leak | trust |
| 2 | command exec | `system` |
| 3 | path write | contents |
| 4 | auth bypass | defaults |
| 5 | crypto | key source |
| 6 | memory | bounds |

Every case is a **missing boundary**: a path not allowlisted, a shell not avoided, an
archive not validated, a password not required, a key not secret, a length not checked.

## The Defensive Pattern

```
validate input  ->  use safe primitives  ->  require auth  ->  keep secrets secret
```

That is the whole of `docs/walkthrough/30`, seen six times.

## Exercises

1. For each case, write the finding in the standard format.
2. Match each case to its lab defect (B1-B6).
3. For one case, describe the detection (`docs/28`).
4. Add a seventh case from a device class you know.

## Reference

- `docs/15`, `docs/16`, `docs/17`, `docs/28`, `docs/walkthrough/29`
- `docs/walkthrough/25` (real-world comparison)
