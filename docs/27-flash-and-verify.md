# Volume 27: Flash and Verify

**writing the four images and proving them**

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

- How to write the four images to the RP5 image store
- How to verify a write (and why verify is mandatory)
- The golden-image rule and the recovery path
- Why verification is the difference between a lab and a brick

## 1. Write

For a camera (SPI-NOR) you would use `flashrom`. For the RP5 the image store is
**microSD / NVMe**, so you write raw regions or a whole-image file:

```bash
# whole image to a block device (BE SURE of the device path!)
sudo dd if=CTF-XX-full.img of=/dev/rdiskN bs=4m
sync
```

On a camera the equivalent is:

```bash
flashrom -p ch341a_spi -w full.img
```

## 2. Verify (mandatory)

A write is not done when it returns; it is done when it **verifies**:

```bash
# image file
python3 scripts/verify_telescreen.py --image CTF-XX-full.img \
      --sha256 <expected>

# block device: read it back and compare
sudo dd if=/dev/rdiskN of=/tmp/back.img bs=4m
shasum -a 256 /tmp/back.img
```

For a physical flash, `flashrom -v` reports `VERIFIED`. Require it.

## 3. The Golden Image Rule

Always keep the stock image. It is your rollback and your control:

```
golden  <- read it, hash it, keep it
patched <- build it, verify it, flash it
```

On the camera the golden image is the first, clean read (`docs/04`). On the RP5 it is
the stock four images. Without it, a bad write has no clean reference.

## 4. The Recovery Path

A camera's BootROM reads the flash head first. If the **boot** partition is corrupt,
the device does not boot and needs an **external reflash**. The RP5 is more forgiving
(boot from a fresh SD), but the principle holds: **the boot region is the one you must
not break** without a recovery plan.

## 5. Verify the Hardened Image

The CTF ships `CTF-XX-full_fixed.img` (the hardened reference). After you build your own
fix, compare:

```bash
shasum -a 256 CTF-XX-full.img CTF-XX-full_fixed.img
python3 scripts/verify_telescreen.py --image CTF-XX-full_fixed.img
```

## 6. Why Verify Is a Security Property

An unverified write is an opportunity for silent corruption - and, on a real device, for
a **supply-chain swap**. Verify is how you know the bytes you wrote are the bytes you
intended. It is the same discipline as the CRC checks in `docs/03`, `docs/07`, `docs/11`.

## Labs

1. Write `CTF-XX-full.img` to a spare SD card and read it back; compare hashes.
2. Corrupt the read-back image and show the verifier catches it.
3. Explain why the boot region is the critical one for recovery.
4. Compare the stock and hardened image hashes.

## Reference

- `scripts/verify_telescreen.py`
- `docs/04` (carve/verify), `docs/26` (build), `docs/17` (hardened beacon)
