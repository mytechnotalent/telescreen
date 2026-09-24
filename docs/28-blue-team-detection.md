# Volume 28: Blue-Team Detection

**detecting the beacon, the backdoors, and the weak KDF**

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

- How to **detect** a beacon from the network side
- How to **detect** each backdoor class
- How to recognise a weak-key channel
- A one-page playbook you can apply to a real device

## 1. Detect the Beacon

A beacon is a **periodic** outbound flow to a **fixed** destination, often with
**high-entropy** payloads.

| signal | method |
| ------ | ------ |
| periodicity | histogram inter-arrival times; look for a fixed interval |
| jitter | measure the spread; near-zero jitter is machine-like |
| destination | flag a fixed IP/domain you did not configure |
| entropy | payload entropy near 8 bits/byte suggests ciphertext |

```bash
# who is this box talking to, and how often?
sudo tcpdump -i eth0 -n -ttt 'not arp and not port 22' | head
```

## 2. Detect the Backdoors

| class | indicator |
| ----- | --------- |
| B1 config-sourced root exec | a shell script written at boot; watch the config file for writes |
| B2 CGI command injection | `system()` calls reachable from the web server |
| B3 archive-to-root restore | an endpoint that extracts an upload to `/` |
| B4 empty/default credentials | an empty credential store; a login that never fails |
| B5 debug root shell | an unexpected listener or console path |

Static checks:

```bash
grep -rn 'system(' src/                    # shell from code
readelf -s teled | grep -w system
strings -n4 teled | grep -E '\.cgi|/dev/mtd'
```

## 3. Detect a Weak-Key Channel

If a channel's key is derivable from **public** data (a UID, a serial, a broadcast ID),
the traffic is effectively plaintext. Test: recompute the key from the public ID and try
to decrypt a captured frame. If it opens, there is no confidentiality.

```bash
python3 scripts/weak_decrypt.py --uid <public-uid>
```

## 4. The Playbook (one page)

```
DETECT
  - periodic outbound flow to an unconfigured endpoint      -> beacon
  - config file written at boot, sourced as root            -> B1
  - system() reachable from the web server                  -> B2
  - an endpoint that extracts uploads to /                   -> B3
  - a login that accepts empty/default credentials          -> B4
  - an unexpected listener or console path                  -> B5
  - a key derivable from a public ID                        -> B6

CONTAIN
  - block the beacon endpoint; isolate the device
  - make the rootfs read-only and lock the config file
  - require auth before any stream or config

HARDEN
  - secure boot + signed images
  - remove default credentials
  - replace the public-ID KDF with X25519 + HKDF + AES-GCM
```

## 5. Why Detection Is the Deliverable

You cannot patch every device in the field, but you **can** detect the class. The
blue-team playbook is what turns this course from "how to break a camera" into "how to
defend a fleet".

## Labs

1. Capture the beacon on `eth0` and measure its interval.
2. Flag each backdoor on your build with a static check.
3. Recompute the weak key and decrypt one frame.
4. Write your own one-page playbook for a device you own.

## Reference

- `docs/17` (weak KDF), `docs/16` (backdoors), `docs/15` (system sites)
- `CTF_telescreen/CTF-XX-R.md` (Task 8)
