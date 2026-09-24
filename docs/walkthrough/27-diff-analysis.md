# Walkthrough 27: Diff Analysis (Stock vs Hardened)

**reading a binary diff as a story**

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

## Why Diff

A binary diff is the cheapest way to see **what changed between two builds**. A vendor
update is a diff; your patch is a diff; a supply-chain attack is a diff. Reading a diff
tells you where to look.

## 1. The Diff

Compare the stock and hardened images:

```bash
cmp -l CTF-XX-full.img CTF-XX-full_fixed.img | head
```

Real result for this lab:

```
runs: 1
  0x00000180-0x00000193 (19 bytes)
  old = 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  new = 54 45 4c 45 53 43 52 45 45 4e 2d 48 41 52 44 45   "TELESCREEN-HARDE"
```

The changed bytes are ASCII:

```
54 45 4c 45 53 43 52 45 45 4e 2d 48 41 52 44 45 4e 45 44
T  E  L  E  S  C  R  E  E  N  -  H  A  R  D  E  N  E  D
```

One run, 19 bytes, offset `0x180`, and it decodes to a marker. Read the story:

| observation | conclusion |
| ----------- | ---------- |
| one run | a single, localized change |
| length 19 | a short marker string |
| ASCII bytes | a text marker, not code |
| offset `0x180` | inside the boot region |
| the fixed image only | the "build" change |

## 2. What a Real Diff Looks Like

A real vendor update diff is messier:

| pattern | meaning |
| ------- | ------- |
| many small runs | recompiled code, shifted by layout |
| one large run | a replaced blob (a whole file or binary) |
| runs in the rootfs region | a changed config/script |
| runs in the kernel region | a patched kernel |
| runs in the boot region | a bootloader or vector change |

The **region** of a change tells you what kind of change it is. That is why you carve
first (`docs/04`) and diff per region.

## 3. Diff per Region

```bash
python3 scripts/carve.py --image CTF-XX-full.img       --out stock/
python3 scripts/carve.py --image CTF-XX-full_fixed.img --out fixed/
for r in boot bootargs kernel rootfs; do
  echo "== $r =="; cmp -l stock/$r.img fixed/$r.img | head
done
```

A change confined to `boot` is a bootloader change; a change in `rootfs` is a config or
app change. The lab's marker is in `boot` (`0x180 < 0x20000`).

## 4. Diff and the CTF

When you harden the image (`docs/walkthrough/16`), your diff **should** touch exactly the
regions your fixes need:

- B1/B4/B5 -> `rootfs`
- B2/B3/B6 -> the app (inside `kernel`'s container or the rootfs)
- a marker -> `boot`

A diff that touches something you did not intend is a **red flag**. Review your own diff
before you ship it - the same discipline a vendor should apply.

## 5. Reproduce the Diff

```bash
python3 - <<'PY'
a=open('CTF-XX-full.img','rb').read()
b=open('CTF-XX-full_fixed.img','rb').read()
i=0
while i<len(a):
    if a[i]!=b[i]:
        j=i
        while j<len(a) and a[j]!=b[j]: j+=1
        print(f"{i:#010x}-{j:#010x} ({j-i}B)")
        i=j
    else: i+=1
PY
```

This is the same loop a real update analyser uses.

## Exercises

1. Run the diff and confirm the single 19-byte run.
2. Decode the 19 bytes as ASCII and record the marker.
3. Diff per region and confirm the change is in `boot`.
4. Harden one defect, rebuild, and diff your change against the stock.

## Reference

- `docs/04`, `docs/26`, `docs/27`
- `docs/walkthrough/16` (the hardening path)
