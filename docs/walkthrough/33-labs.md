# Walkthrough 33: The Complete Lab Book

**every lab in the course, in one place, with a grading rubric**

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

## How to Use This Book

Each lab has: **goal, tools, steps, deliverable, points**. Do them in order; each builds on
the last. Everything is on your own isolated bench.

## Lab 0: Bring Up the Bench (10 pts)

**Goal.** Boot the four images and see the console.
**Tools.** RP5, PSU, microSD, UART adapter.

1. Write the four images to the card (`docs/27`).
2. Connect UART at 115200 8N1.
3. Power on; capture the log to a file.

**Deliverable.** The boot log, with U-Boot, kernel, and app lines marked. (10 pts)

## Lab 1: Carve and Verify (10 pts)

**Goal.** Prove the layout from the bytes.
**Tools.** `dd`, `xxd`, `carve.py`, `verify_telescreen.py`.

1. Carve the four partitions.
2. `xxd` the first 16 bytes of each; match the magic table.
3. Hash each and the whole image.

**Deliverable.** The partition table + hashes + magic evidence. (10 pts)

## Lab 2: Boot Chain (15 pts)

**Goal.** Trace power-on to the application.
**Tools.** `objdump`, `python3`, the console.

1. Decode the reset vector.
2. Disassemble the reset handler.
3. Read `bootargs`/`bootcmd`.
4. Record the kernel banner and the app banner.

**Deliverable.** A one-page trace with addresses. (15 pts)

## Lab 3: Kernel and Device Tree (15 pts)

**Goal.** Identify the kernel format and machine.
**Tools.** `xxd`, `gunzip`, `dtc`.

1. Parse the container header.
2. Inflate the payload; record the size.
3. Read the device tree model and the flash/UART nodes.

**Deliverable.** The header table + inflated size + DT model. (15 pts)

## Lab 4: Rootfs and CRCs (15 pts)

**Goal.** Extract the rootfs and prove the CRC discipline.
**Tools.** `jefferson`, Python.

1. Extract the tree.
2. Compute `crc32_le(0, node[0:8])` and confirm the CLEANMARKER header CRC.
3. Show zlib's CRC differs.

**Deliverable.** The tree + the CRC comparison. (15 pts)

## Lab 5: The Backdoors (20 pts)

**Goal.** Find B1-B5 and write exploit paths.
**Tools.** `strings`, Ghidra, `grep`.

1. Find the sourced config (B1).
2. Find the `system()` route (B2).
3. Find the archive-to-root restore (B3).
4. Find the credential store (B4).
5. Find the debug path (B5).

**Deliverable.** A finding table in the format (`docs/walkthrough/32`). (20 pts)

## Lab 6: Break the Crypto (15 pts)

**Goal.** Prove the beacon key is public.
**Tools.** `weak_decrypt.py`, Python.

1. Recover the key from the UID.
2. Decrypt a captured frame.
3. Write 200 words on why this is not encryption.

**Deliverable.** The key, the decrypted frame, and the analysis. (15 pts)

## Lab 7: The RP5 Router (10 pts)

**Goal.** Route and prove it.
**Tools.** `hostapd`, `dnsmasq`, `nftables`.

1. Bring up the AP, DHCP, and NAT.
2. Join with a phone; confirm the lease.
3. Capture a NATed packet.

**Deliverable.** The lease and the NAT capture. (10 pts)

## Lab 8: The RP5 Camera (10 pts)

**Goal.** Stream and view with no app.
**Tools.** the USB webcam, `ffmpeg`/`mjpg_streamer`, VLC.

1. Confirm `/dev/video0`.
2. Serve MJPEG; open it in the phone browser.
3. Serve RTSP; open it in VLC.

**Deliverable.** Both URLs working, one with no app. (10 pts)

## Lab 9: Harden and Reflash (15 pts)

**Goal.** Produce a hardened image.
**Tools.** the fixes, `build_images.py`, `verify_telescreen.py`.

1. Apply the six fixes.
2. Rebuild and verify.
3. Diff against `CTF-XX-full_fixed.img`.

**Deliverable.** The hardened image, its hash, and the diff. (15 pts)

## Lab 10: The Defensive Playbook (bonus, 10 pts)

**Goal.** A one-page detection playbook.
**Tools.** `tcpdump`, static checks.

1. Detect the beacon (periodicity, entropy).
2. Detect each backdoor.
3. Propose field hardening.

**Deliverable.** The playbook. (10 pts)

## The Rubric

| lab | points |
| --- | ------ |
| 0 bench | 10 |
| 1 carve | 10 |
| 2 boot | 15 |
| 3 kernel | 15 |
| 4 rootfs | 15 |
| 5 backdoors | 20 |
| 6 crypto | 15 |
| 7 router | 10 |
| 8 camera | 10 |
| 9 harden | 15 |
| 10 playbook | +10 |
| **base total** | **135** |

## Partial Credit

| scenario | credit |
| -------- | ------ |
| right approach, wrong value | 75% |
| found it, wrong fix | 60% |
| explained it, could not locate | 40% |
| patched, could not explain | 50% |
| documented thoroughly, wrong result | 30% |

## Academic Integrity

By submitting, you certify: this is your own work; you understand the ethics; you will use
these skills only on authorised hardware (`docs/29`).

## Reference

- every `docs/` and `docs/walkthrough/` volume
- `CTF_telescreen/CTF-XX-R.md` (the formal rubric)
