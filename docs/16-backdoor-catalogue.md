# Volume 16: Backdoor Catalogue

**B1-B6 with code and exploit paths**

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

- The six defects the CTF ships, each with code and an exploit path
- How each maps to a fixing technique from a later volume
- How to write a finding: mechanism, path, impact, fix

## The Catalogue

### B1 - Config-Sourced Root Execution

**Mechanism.** The init path sources a writable configuration file **as root**:

```sh
. $WIFIPATH          # wifi.conf, sourced as root
```

**Path.** Any process that can write that file executes as root on the next boot.
**Impact.** Root code execution from a config write.
**Fix.** Never source attacker-writable data; parse it with a validated reader.

### B2 - CGI Command Injection

**Mechanism.** A handler builds a shell command from a request parameter and calls
`system()` (`docs/15`).

**Path.** `GET /<route>?param=<payload with ; or $()>` -> root command execution.
**Impact.** Unauthenticated remote root.
**Fix.** `execve` with an argument vector; never a shell string.

### B3 - Archive-to-Root Restore

**Mechanism.** The restore handler runs:

```sh
tar -xvzf <uploaded> -C /
```

**Path.** An archive containing `../` entries writes anywhere as root. The **path is
fixed**; the **contents** are the attack.
**Impact.** Arbitrary root file write -> persistence.
**Fix.** Validate entries; extract to a sandbox; drop privileges.

### B4 - Empty / Default Credentials

**Mechanism.** The web credential store is empty, or the device ships default
credentials.

**Path.** Log in with an empty or default password.
**Impact.** Admin access to every config setter.
**Fix.** Require a password at first boot; refuse defaults.

### B5 - Debug Root Shell

**Mechanism.** A debug path (a service or a script) provides a local root shell.

**Path.** Reach the debug path on the console or via a trigger.
**Impact.** Local privilege escalation.
**Fix.** Remove debug paths from production images.

### B6 - Weak Key Schedule

**Mechanism.** The beacon key is derived from the **public** device UID
(`docs/17`).

**Path.** Recompute the key from the public UID and decrypt the telemetry.
**Impact.** The "sealed" channel provides no confidentiality.
**Fix.** X25519 + HKDF + AES-256-GCM (`docs/18`-`docs/23`).

## The Finding Format

Write every finding the same way:

```
DEFECT : B<n> short name
WHERE  : partition + file/function + address or offset
MECH   : the exact line or instruction
PATH   : how an attacker reaches it
IMPACT : what the attacker gets
FIX    : the specific change
```

## Mapping Defects to Fixes

| defect | fix volume |
| ------ | ---------- |
| B1 | `docs/12` (config discipline) |
| B2 | `docs/15` (`execve`) |
| B3 | `docs/15`, `docs/16` |
| B4 | `docs/16` |
| B5 | `docs/16` |
| B6 | `docs/17`-`docs/23` |

## Labs

1. For each defect, write a finding in the format above.
2. For B3, describe the archive that exploits it.
3. For B6, write the derivation and show the key is public.
4. Propose the patch for each and name the technique.

## Reference

- `docs/15` (system sites), `docs/17` (weak KDF), `CTF_telescreen/CTF-XX-S.md`
