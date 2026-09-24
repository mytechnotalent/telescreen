# Walkthrough 16: Solving the CTF (Offensive Path)

**the full solve, step by step, against the compromised image**

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

## The Rules

Work only on the artifacts in `CTF_telescreen/`, on hardware you own, on an isolated
network. This walkthrough is the **offensive** path; the defensive path is `docs/28`.

## Phase 0: Triage

```bash
file CTF-XX-full.img
xxd -l 32 CTF-XX-full.img
ls -l CTF-XX-*.img
```

You see a 16 MiB blob. `xxd` shows `15 05 00 ea` at offset 0 - the ARM reset vector.

## Phase 1: Carve and Verify

```bash
python3 scripts/carve.py --image CTF-XX-full.img --out carved/
python3 scripts/verify_telescreen.py --image CTF-XX-full.img
sha256sum CTF-XX-*.img
```

You now have the four regions and a hash for each. Record them; you will diff against
them later.

## Phase 2: The Boot Chain

```bash
xxd -l 32 carved/boot.img
python3 - <<'PY'
w = 0xEA000515; off = w & 0xFFFFFF
print(hex(0x08 + off*4))     # reset handler
PY
strings -n4 carved/boot.img | grep -i u-boot
```

**Finding:** a Cortex-A vector table at offset 0, reset handler at `0x145C`, and a U-Boot
banner string. Document both (`docs/05`).

```bash
strings -n4 carved/bootargs.img | head
```

**Finding:** `bootargs` with the `mtdparts` and the root specification (`docs/06`,
`docs/07`).

## Phase 3: The Kernel Container

```bash
xxd -l 32 carved/kernel.img
dd if=carved/kernel.img bs=1 skip=16 | gunzip > Image
file Image
```

**Finding:** the vendor container magic `21 84 1b 00` + `gziphead`, then a gzip stream;
inflating yields the raw kernel (`docs/08`).

## Phase 4: The Rootfs

```bash
xxd -l 16 carved/rootfs.img
jefferson -d rootfs carved/rootfs.img
find rootfs -maxdepth 3 | head
```

**Finding:** JFFS2 (`85 19 03 20`). The extracted tree contains the application config
and scripts (`docs/11`).

## Phase 5: Find the Backdoors

### B1 - config-sourced root exec

```bash
grep -rnE '^\. |source ' rootfs/etc rootfs/mnt 2>/dev/null | head
```

Look for an init path that **sources a writable config as root**. **Finding:** a script
sources a config file under the app directory (`docs/16`).

### B2 - CGI command injection

```bash
strings -n4 Image teled 2>/dev/null | grep -E '\.cgi' | head
```

Find the route names, then the handler that calls `system()`. **Finding:** a handler
builds a shell command from a request parameter (`docs/14`, `docs/15`).

### B3 - archive-to-root restore

```bash
grep -rn 'tar ' rootfs 2>/dev/null | head
grep -rn 'unzip' rootfs 2>/dev/null | head
```

**Finding:** a restore path running `tar -xvzf <upload> -C /` (`docs/16`). The attack is a
crafted archive with `../` entries.

### B4 - credentials

```bash
cat rootfs/etc/passwd rootfs/etc/shadow 2>/dev/null
ls -l rootfs/web/.htpasswd 2>/dev/null
```

**Finding:** an empty or default credential store (`docs/16`).

### B5 - debug shell

```bash
grep -rn 'telnetd\|debug\|/bin/sh' rootfs/etc rootfs/usr 2>/dev/null | head
```

**Finding:** a debug path that yields a root shell.

## Phase 6: Break the Exfiltration Crypto

```bash
grep -rn 'beacon\|uid\|key' rootfs/etc rootfs/mnt 2>/dev/null | head
python3 scripts/weak_decrypt.py --uid <the-uid-you-found>
```

**Finding:** the beacon key is derived from the **public UID** (`docs/17`). Anyone can
recompute it. Recover the key and decrypt a captured frame.

## Phase 7: The Finding Write-Up

For each defect, write:

```
DEFECT : B<n>
WHERE  : partition + file + function/offset
MECH   : the exact line or instruction
PATH   : how to reach it
IMPACT : what you get
FIX    : the change
```

## Phase 8: Harden

Take the four fixes:

| defect | fix |
| ------ | --- |
| B1 | stop sourcing config; parse and validate |
| B2 | `execve` with an argument vector |
| B3 | reject `../`; extract sandboxed |
| B4 | require a password |
| B5 | remove the debug path |
| B6 | X25519 + HKDF + AES-256-GCM |

Rebuild the images (`docs/26`), verify (`docs/27`), and compare to
`CTF-XX-full_fixed.img`.

## Phase 9: The Report

Produce `TELESCREEN-Answers.md` with the partition table, the boot-chain analysis, the
backdoor catalogue, the crypto break, your hardened build, and the defensive playbook.

## The Map (finding -> volume)

| phase | volume |
| ----- | ------ |
| carve | `docs/04` |
| boot | `docs/05`, `docs/06` |
| kernel | `docs/08` |
| rootfs | `docs/11` |
| backdoors | `docs/14`, `docs/15`, `docs/16` |
| crypto | `docs/17` |
| hardening | `docs/18`-`docs/23` |
| rebuild | `docs/26`, `docs/27` |
| defense | `docs/28`, `docs/29` |

## Exercises

1. Write the finding format for all six defects.
2. For B3, construct the archive that exploits it.
3. For B6, show the key is public and decrypt a frame.
4. Produce a hardened image and diff it against `CTF-XX-full_fixed.img`.

## Reference

- `CTF_telescreen/CTF-XX-S.md`
- every `docs/` volume cited above
