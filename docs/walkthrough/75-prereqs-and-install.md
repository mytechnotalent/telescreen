# Walkthrough 75 - Prerequisites and Install (lab sheet)

***
**LEGAL DISCLAIMER:** The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with. **IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
***

**Full teaching text:** `docs/31-prerequisites-and-install.md`. This sheet is the
checklist version.

## Goal

From a bare computer to a working bench on **Windows x64**, **Linux x64**, or
**macOS arm64**, in under an hour.

## Checklist

### 1. Install

- [ ] **Git** - `git --version`
- [ ] **Docker** - `docker run --rm hello-world`
- [ ] **JDK 21** - `java -version` prints `21`
- [ ] **Ghidra 12.1.3** - unzipped to `~/ghidra`

### 2. Verify ARM64 emulation / nativity

```bash
docker run --rm --platform linux/arm64 alpine uname -m     # aarch64
```

- macOS arm64: runs natively.
- Linux x64: if it errors, `sudo apt-get install -y qemu-user-static binfmt-support`.
- Windows x64: enable WSL integration in Docker Desktop.

### 3. Verify the Ghidra decompiler natives

```bash
ls ~/ghidra/Ghidra/Features/Decompiler/os/
```

- Linux x64 → `linux_x86_64` present.
- Windows x64 → `win_x86_64` present.
- macOS arm64 → **must** show `mac_arm_64`; if not, build them
  (`WALKTHROUGH/08-build-ghidra/BUILD.md`).

### 4. Build and check

```bash
./firmware/build_target.sh
#  -> firmware/teled.stripped  (student target)
#  -> firmware/teled.unstripped (answer key)
#  -> prints the SHA-256 of each; it should match  ghidra/RESOLUTION_MAP.md
```

### 5. Analyse and open

```bash
./ghidra/make_project.sh
./ghidra/decompile.sh
open ~/Applications/Ghidra.app    # then open ghidra/proj/TelescreenRE.gpr
```

## Done when

- [ ] `docker run --platform linux/arm64` reports `aarch64`
- [ ] `build_target.sh` prints `exit code = 0`
- [ ] the SHA-256 of `teled.stripped` matches the published value
- [ ] `Ghidra/Features/Decompiler/os/` contains your platform's natives
- [ ] the Ghidra GUI decompiles `0x401000` (`_start`) with `F`
