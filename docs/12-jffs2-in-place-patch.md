# Volume 12: JFFS2 In-Place Patch

**editing a read-only rootfs with CRC discipline**

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

- How to change a file in a JFFS2 rootfs **without reordering nodes**
- The higher-version INODE + PADDING technique
- The exact CRC order that makes the patch mountable
- Why this is the vendor update mechanism (and an attack surface)

## 1. The Problem

A File is read-only on the device, so you cannot edit it live. You must patch the
**image**. But JFFS2 has no fixed offsets: nodes are appended and scanned. If you change
a node's length, every subsequent node shifts and the filesystem breaks.

## 2. The Technique

Because JFFS2 is append-only and version-aware, you **append a newer node** for the same
inode and let the old one become obsolete:

1. Find the newest data-bearing **INODE** for the target file's inode number.
2. Append a new INODE with `version = old_version + 1000`, `offset = 0`, `compr = 0`
   (stored, uncompressed), `isize = csize = dsize = len(new_data)`.
3. If the new data is shorter than the old node, append a **PADDING** node so the
   **total length is unchanged** - no offsets shift.
4. Recompute the CRCs **in order**: `hdr_crc` -> `data_crc` -> `node_crc`.

## 3. Why the Order Matters

`node_crc` covers the header fields **including** `hdr_crc`, so `hdr_crc` must be
final before `node_crc` is computed:

```
1. write the header fields (version, csize, dsize, compr, ...)
2. hdr_crc  = crc32_le(0, node[0:8])          ; depends on magic/type/totlen
3. data_crc = crc32_le(0, data)                ; depends on the new bytes
4. node_crc = crc32_le(0, node[0:60])          ; depends on hdr_crc AND data_crc
```

Get the order wrong and the node fails to mount.

## 4. A Worked Example

Target: change one line of a config file.

```
before: hdr_crc=0x…  data_crc=0x…  node_crc=0x…
after : hdr_crc=0x…  data_crc=0x…  node_crc=0x…   (all re-verified)
```

The old node stays where it is (obsolete); the new node carries the higher version; the
mount scan picks the higher version. Minimal diff, no shift.

## 5. Script It

The repo ships patchers that do exactly this:

```bash
python3 scripts/weak_decrypt.py --uid SSAT-468547-FEEBD   # example tool
# the in-place JFFS2 patcher follows the same CRC order as src/jffs2.c + src/crc.c
```

## 6. Why This Is Also an Attack Surface

The **same** mechanism is how a vendor updates a file on a read-only rootfs - and how an
attacker with image-write access persists. That is why the fix is not "make JFFS2
read-only" (it is), but **secure boot + signed updates** (`docs/27`).

## 7. Verify After Patching

1. Re-parse every node: `hdr_crc`, `data_crc`, `node_crc` all valid.
2. Extract with `jefferson` and diff the target file against intent.
3. Hash the new image and record it.

## Labs

1. Patch one config value and show all three CRCs before and after.
2. Deliberately corrupt `node_crc` and show the node is ignored at mount.
3. Explain why the PADDING node is required when the new data is shorter.
4. Explain why this doubles as a persistence mechanism.

## Reference

- `src/jffs2.c`, `src/crc.c`
- `docs/11` (JFFS2 nodes), `docs/16` (backdoors), `docs/27` (flash and verify)
