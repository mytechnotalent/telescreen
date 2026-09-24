# Walkthrough 76 - Ghidra from Zero (first session)

***
**LEGAL DISCLAIMER:** The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with. **IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
***

**Full teaching text:** `docs/33-ghidra-nation-state-re.md`.

## Goal

Open the stripped ARM64 target and recover your first three function names,
entirely by evidence.

## 1. Open the project

```
File -> Open Project -> ghidra/proj/TelescreenRE.gpr
double-click  teled.stripped
```

If the Decompiler pane is empty on macOS, build the `mac_arm_64` natives
(Volume 31 §31.3 Step 5).

## 2. Entry point

```bash
readelf -h firmware/teled.stripped | grep Entry       # 0x401000
```

In Ghidra: press `G`, type `0x401000`, Enter. That is `_start`. Press `F`.

## 3. Find `main`

In `_start`, look for the address loaded into `x0` right before the call to
`__libc_start_main`. That address is `main` (here `0x400f80`). Click it, press
`L`, name it `main`.

## 4. Read `main`

Press `F` on `main`. You will see three calls and a string:

```
aead_init(AEAD_ALGO_AES_256_GCM);
teled_init("SSAT-468547-FEEBD", key);
return teled_beacon(payload) ? 0 : 1;
```

Rename each call target with `L`.

## 5. Climb the graph

Press `X` on `teled_beacon`, then on `beacon_seal`, then on `aead_seal`. At
`aead_seal`, the imports tell you it is encryption:

```
EVP_EncryptInit_ex ... EVP_EncryptUpdate ... EVP_EncryptFinal_ex
```

Press `X` on a string to see who references it. That is the whole technique.

## 6. Record your evidence

For each name you decide, write it in a table and cite a rule (R1-R4):

```
0x00400fc0  FUN_00400fc0 -> main         evidence: R1 + arg to __libc_start_main
0x00401400  FUN_00401400 -> aead_init    evidence: R2 EVP_* cluster
0x00401b20  FUN_00401b20 -> beacon_weak_key  evidence: R1 + crc32 loop over UID
```

## Done when

- [ ] you named `_start`, `main`, `aead_init`, `teled_init`, `teled_beacon`
- [ ] you can explain why `aead_seal` is AES-GCM (cite the imports)
- [ ] you wrote down the rule (R1-R4) for every name

Continue with **Walkthrough 77** to resolve all 42 functions.
