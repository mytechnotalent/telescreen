# Walkthrough 30: Defensive Hardening

**building the device the CTF does not ship**

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

## The Principle

Every defect has a **countermeasure**, and the countermeasures stack into a defended
device. This volume is the "fix everything" companion to `docs/walkthrough/16`.

## 1. Secure Boot

| threat | countermeasure |
| ------ | -------------- |
| replaced kernel/rootfs | **verify a signature before jumping** |
| replaced bootloader | **ROM-verified first stage** + signed images |

`identity.c` (`docs/22`) is the primitive: U-Boot holds a public key, checks an Ed25519
signature over the kernel, and refuses to boot a bad image. Without it (the lab's state),
anyone with image access boots anything (`docs/27`).

## 2. No Shell From Input

| defect | fix |
| ------ | --- |
| B2 command injection | `execve` with an argument vector |

The rule: **no shell**. Build the vector, call `execve` (`docs/15`). A shell parses
metacharacters; a vector does not.

## 3. Validate Every Path

| defect | fix |
| ------ | --- |
| B3 archive-to-root | reject `../` and absolute entries; extract sandboxed |

Canonicalize the entry, ensure it is under the target root, and only then write. Drop
privileges during extraction.

## 4. Real Credentials

| defect | fix |
| ------ | --- |
| B4 empty/default | require a password at first boot |

Force a set at provisioning; refuse defaults; store an Argon2id hash, never plaintext
(`docs/23`).

## 5. Remove Debug

| defect | fix |
| ------ | --- |
| B5 debug shell | strip debug paths from production images |

No `telnetd`, no blank root, no vendor test hook in a shipped build.

## 6. Secrets, Not Public Data

| defect | fix |
| ------ | --- |
| B6 weak KDF | X25519 + HKDF; never derive from a public ID |

The full stack (`docs/21`-`docs/23`): X25519 for agreement, HKDF for derivation, AES-GCM
for data, Ed25519 for identity, Argon2id for passwords.

## 7. Memory Safety

| practice | effect |
| -------- | ------ |
| bounds-checked copies (`snprintf`, `strncpy` + NUL) | no overflow |
| `-fstack-protector-strong` | canary on the stack |
| `-D_FORTIFY_SOURCE=2` | checked libc copies |
| no `strcpy`/`sprintf` on input | no unbounded writes |

The CMake build enables these (`docs/walkthrough/22`) - the CTF device does not.

## 8. Network Posture

| control | effect |
| ------- | ------ |
| authenticate before parse | no pre-auth parser bugs |
| no auto inbound holes | no silent exposure |
| no default beaconing | no telemetry by default |
| client isolation | no campaign from one client |

## 9. The Hardening Checklist

```
[ ] signed images + verified boot
[ ] no system() from input            (execve)
[ ] archive entries validated         (no ../)
[ ] real credentials, Argon2id hash
[ ] debug paths removed
[ ] X25519 + HKDF + AES-GCM           (no public-ID keys)
[ ] bounds-checked string handling
[ ] stack protector + FORTIFY
[ ] auth before parse on every listener
[ ] no auto NAT holes; no default beacon
```

## 10. The Hardened Image

Apply the fixes, rebuild (`docs/26`), verify (`docs/27`), and diff against the stock
(`docs/walkthrough/27`). The result should be a device that:

- boots only signed images;
- never builds a shell command from input;
- extracts nothing outside a sandbox;
- keeps its secrets secret;
- answers nothing before authentication.

## Exercises

1. Walk the checklist and mark which items the lab already satisfies.
2. Implement the `execve` fix for one `system()` site.
3. Add `-Wconversion` and fix the narrowing warnings.
4. Produce a hardened image and diff it against the stock.

## Reference

- `docs/15`-`docs/23`, `docs/27`, `docs/28`
- `docs/walkthrough/16` (the offensive path)
