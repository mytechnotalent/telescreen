# Volume 31 - Prerequisites and Installation (Windows, Linux, macOS)

***
**LEGAL DISCLAIMER:** The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with. **IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**
***

This volume takes you from a bare computer to a fully working reverse-engineering
workbench. It assumes **zero** prior experience. Every command is written out in
full, every term is defined the first time it appears, and every operating system
is covered:

- **Windows x64**
- **Linux x64**
- **macOS arm64** (Apple Silicon, M1/M2/M3/M4)

If you already have a tool installed, skip that section. If you get stuck, read
the Troubleshooting table at the very end.

---

## 31.1 What you are going to build, and why

At the end of this volume you will be able to do this:

1. Take a program called **`teled.stripped`** - a real, compiled, **stripped**
   ARM64 program - and open it in a tool called **Ghidra**.
2. Read the machine code Ghidra shows you.
3. Turn the meaningless names Ghidra prints (`FUN_00401e00`) back into the real
   function names, using nothing but evidence in the binary.
4. Prove every one of your answers against an instructor "answer key".

That is exactly what a professional reverse engineer does when handed a captured
device. This course makes you do it 42 times, once for every function, so the
skill becomes automatic.

### Words you need (a tiny glossary)

Read this once. You will meet every one of these words in the first hour.

| Word | Plain meaning |
| ---- | ------------- |
| **binary** | A file of instructions a CPU runs. Also called an *executable*. On Linux it is usually an **ELF** file. |
| **ELF** | "Executable and Linkable Format". The standard layout for Linux programs. |
| **aarch64 / ARM64** | The 64-bit CPU family used by the Raspberry Pi 5 and by every Apple Silicon Mac. Same instruction set, different operating systems. |
| **compile** | Turn source code (`hello.c`) into a binary. The program that does this is a **compiler** (here, `gcc`). |
| **toolchain** | The set of programs used to build software: compiler, linker, archiver. |
| **stripped** | A binary with the human-readable function names removed. All that is left is `FUN_00401e00`. This is what you get from real devices. |
| **symbol table** | The map from names (`crc32_le`) to addresses (`0x401d80`). Stripping deletes it. |
| **disassembly** | Machine code shown as text instructions (`bl 40146c`). |
| **decompiler** | A tool that turns machine code back into approximate C code. Ghidra has a very good one. |
| **Ghidra** | A free, professional reverse-engineering suite from the NSA. |
| **Docker** | A tool that runs a small, complete Linux system inside a container on your computer. |
| **container / image** | A packaged mini-Linux. An **image** is the package; a **container** is a running copy of it. |
| **JDK** | "Java Development Kit". Ghidra is written in Java and needs a JDK to run. |

### The one big idea: we all build the *same* binary

When a compiler turns the same source into a binary, the result depends only on
the compiler and its settings - **not** on whether your computer runs Windows,
Linux, or macOS. To make sure every student's binary is *byte-for-byte identical*,
we build it inside a **pinned Linux/ARM64 container**. That means:

- the SHA-256 fingerprint of the file is the same on every OS,
- and therefore every address in this course (`0x401d80`, ...) is true for you.

On a Mac with Apple Silicon the container runs natively. On Windows x64 and
Linux x64, Docker runs the same ARM64 image through emulation. The emulation is
slower, but the compiler inside is the *same program*, so the output is the same.

---

## 31.2 What you need on EVERY operating system

| Tool | Why | Version used here |
| ---- | --- | ----------------- |
| **Docker Desktop** (Win/mac) or **Docker Engine** (Linux) | build & run the ARM64 target | any recent |
| **Git** | clone this repository | any recent |
| **JDK 21** | Ghidra is a Java program | Temurin 21 or OpenJDK 21 |
| **Ghidra** | the reverse-engineering tool | **12.1.3** |
| **A web browser** | read these docs | any |

We do **not** need the Raspberry Pi 5 hardware for anything in this volume or in
the RE volumes. Static analysis and building both run on your computer. (See
Volume 33 § "When you *do* need hardware" for the few things that do not.)

---

## 31.3 macOS arm64 (Apple Silicon)

### Step 1 - Install Homebrew

Homebrew is the standard package manager for macOS. Open **Terminal**
(Applications → Utilities → Terminal) and paste:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

When it finishes it prints two lines beginning with `echo` and `eval`; run those
too so `brew` is on your PATH. Confirm:

```bash
brew --version
```

### Step 2 - Install Git and the JDK

```bash
brew install git openjdk@21
```

Tell your shell where Java is:

```bash
export JAVA_HOME="$(brew --prefix openjdk@21)/libexec/openjdk.jdk/Contents/Home"
export PATH="$JAVA_HOME/bin:$PATH"
java -version        # must print version 21
```

> Tip: add those two `export` lines to `~/.zshrc` so they apply to every new
> Terminal window.

### Step 3 - Install Docker Desktop

Download **Docker Desktop for Mac (Apple Silicon)** from
<https://www.docker.com/products/docker-desktop/> and install it. Start it and
wait until the whale icon in the menu bar is steady. Confirm:

```bash
docker version
docker run --rm hello-world
```

### Step 4 - Install Ghidra 12.1.3

```bash
# Download the official release
curl -L -o /tmp/ghidra.zip \
  https://github.com/NationalSecurityAgency/ghidra/releases/download/Ghidra_12.1.3_build/ghidra_12.1.3_PUBLIC_20260817.zip
unzip -q /tmp/ghidra.zip -d "$HOME"
mv "$HOME"/ghidra_12.1.3_PUBLIC "$HOME"/ghidra
"$HOME"/ghidra/ghidraRun        # launches the GUI; close it for now
```

### Step 5 - Build the macOS-ARM64 native decompiler (macOS only)

> This is the single most important macOS step, and the one everybody misses.

The official Ghidra download ships the decompiler built for **Linux x86-64** and
**Windows x86-64** only. It does **not** include a macOS-ARM64 build, so on an
Apple-Silicon Mac the Decompile window stays empty. You must build the natives
once. The full recipe lives in this repository at
`WALKTHROUGH/08-build-ghidra/BUILD.md`; the short form is:

```bash
brew install openjdk@21 bison flex cmake
export JAVA_HOME="$(brew --prefix openjdk@21)/libexec/openjdk.jdk/Contents/Home"
export PATH="$JAVA_HOME/bin:$PATH"

mkdir -p "$HOME/ghidra-build" && cd "$HOME/ghidra-build"
git clone --depth 1 --branch Ghidra_12.1.3_build \
  https://github.com/NationalSecurityAgency/ghidra.git ghidra-src
cd ghidra-src
./gradlew -I gradle/support/fetchDependencies.gradle
./gradlew buildNatives          # takes a while; go make tea
```

Then copy the built natives into your Ghidra install (the recipe in
`WALKTHROUGH/08-build-ghidra/BUILD.md` lists every `os/mac_arm_64` directory).
Verify:

```bash
ls "$HOME"/ghidra/Ghidra/Features/Decompiler/os/    # must list mac_arm_64
```

### Step 6 - Verify everything

```bash
git --version
java -version
docker run --rm --platform linux/arm64 alpine uname -m    # prints: aarch64
ls "$HOME"/ghidra/Ghidra/Features/Decompiler/os/          # lists mac_arm_64
```

You are ready. Go to **§31.6 Build the target**.

---

## 31.4 Linux x64 (Ubuntu / Debian)

### Step 1 - Base packages

```bash
sudo apt-get update
sudo apt-get install -y git curl unzip openjdk-21-jdk
java -version        # must print version 21
```

### Step 2 - Docker Engine

```bash
curl -fsSL https://get.docker.com | sudo sh
sudo usermod -aG docker "$USER"     # log out and back in for this to take effect
docker run --rm hello-world
```

> On x64 Linux the ARM64 container is emulated by Docker's built-in QEMU. If
> `docker run --rm --platform linux/arm64 alpine uname -m` does not print
> `aarch64`, install the emulator once:
> `sudo apt-get install -y qemu-user-static binfmt-support && sudo systemctl restart docker`

### Step 3 - Ghidra 12.1.3

```bash
curl -L -o /tmp/ghidra.zip \
  https://github.com/NationalSecurityAgency/ghidra/releases/download/Ghidra_12.1.3_build/ghidra_12.1.3_PUBLIC_20260817.zip
unzip -q /tmp/ghidra.zip -d "$HOME"
mv "$HOME"/ghidra_12.1.3_PUBLIC "$HOME"/ghidra
"$HOME"/ghidra/ghidraRun
```

On Linux x64 the bundled `linux_x86_64` natives work, so the decompiler is ready
immediately. Verify:

```bash
ls "$HOME"/ghidra/Ghidra/Features/Decompiler/os/    # must list linux_x86_64
```

You are ready. Go to **§31.6 Build the target**.

---

## 31.5 Windows x64

Windows needs two things done in order: a real Linux environment (WSL2) for the
build, and native Windows installs for Ghidra and Docker.

### Step 1 - Install WSL2 (Windows Subsystem for Linux)

Open **PowerShell as Administrator** and run:

```powershell
wsl --install -d Ubuntu
```

Reboot, open Ubuntu from the Start menu, and create your Linux username and
password. From now on, **all the `bash` commands in this course are typed inside
Ubuntu**, not in PowerShell.

### Step 2 - Inside Ubuntu, follow the Linux steps

Open Ubuntu and run the Linux x64 section above (31.4). Ubuntu can talk to
Docker Desktop directly once you enable the integration (next step).

### Step 3 - Install Docker Desktop (Windows)

Download **Docker Desktop for Windows** from
<https://www.docker.com/products/docker-desktop/>, install it, and start it. In
**Settings → Resources → WSL Integration**, enable integration with your Ubuntu
distribution. Then in Ubuntu:

```bash
docker run --rm hello-world
docker run --rm --platform linux/arm64 alpine uname -m   # prints: aarch64
```

### Step 4 - Install the JDK and Ghidra on Windows (native)

Ghidra runs on Windows itself (not inside WSL). Install **Temurin JDK 21** from
<https://adoptium.net/> and **Ghidra 12.1.3** from the GitHub releases page,
then launch:

```powershell
& "$HOME\ghidra_12.1.3_PUBLIC\ghidraRun.bat"
```

On Windows x64 the bundled `win_x86_64` natives are present, so the decompiler
works immediately. Verify (inside the Ghidra folder):

```powershell
dir "$HOME\ghidra_12.1.3_PUBLIC\Ghidra\Features\Decompiler\os"
```

### Step 5 - A note on paths

When you open a project in the Windows Ghidra GUI, the files were written by
Ubuntu/WSL2. They live under a path like
`\\wsl$\Ubuntu\home\<you>\...` or, if you cloned inside Windows,
`C:\Users\<you>\...`. Both work; if Windows Ghidra cannot see the project, copy
it to a Windows folder first.

You are ready. Go to **§31.6 Build the target**.

---

## 31.6 Build the target

From the repository root, on **every** operating system (Windows users: inside
Ubuntu/WSL2 or Git Bash):

```bash
./firmware/build_target.sh
```

This script:

1. builds a small Linux/ARM64 toolchain image with Docker (first run only),
2. compiles every module into one ARM64 program,
3. saves **`firmware/teled.unstripped`** (the answer key, has names), and
4. saves **`firmware/teled.stripped`** (the student target, names removed),
5. prints the SHA-256 of each file, and
6. runs the stripped binary to prove it works.

You should see (abridged):

```
[*] Compiling lab modules -> teled.unstripped
    unstripped: 75416 bytes
    stripped  : 67752 bytes
[*] SHA-256:
e530b63...  teled.stripped
[*] Proving the stripped target runs natively (aarch64 Linux in Docker):
    exit code = 0
```

> **If the SHA-256 does not match** the value printed in `ghidra/RESOLUTION_MAP.md`,
> something is different (often a different Docker platform). Re-run with the
> default settings and make sure `--platform` is `linux/arm64`.

## 31.7 Create the Ghidra project

```bash
./ghidra/make_project.sh          # imports + analyses + saves, headless
./ghidra/decompile.sh             # optional: export every function to C
```

`make_project.sh` writes an analysed project to `ghidra/proj/TelescreenRE.gpr`
and `decompile.sh` fills `ghidra/decomp/` with one `.c` file per function. That
folder is the raw, confusing starting point - `FUN_00401e00` and friends - which
Volume 33 teaches you to resolve.

## 31.8 Your first five minutes in Ghidra

```bash
open ~/Applications/Ghidra.app        # macOS
~/ghidra/ghidraRun                    # Linux
```

On Windows, run `ghidraRun.bat`. Then:

1. **File → Open Project…** and choose `ghidra/proj/TelescreenRE.gpr`.
2. Double-click **`teled.stripped`** in the project tree. The listing opens.
3. Press **`G`**, type `0x401d80`, press Enter. You are now at `crc32_le`.
4. Press **`F`** to decompile the function your cursor is in.
5. Press **`G`**, type `0x401000`, Enter. That is `_start`, the program entry.

If the listing is full of `??` and the decompiler says "No Function", you opened
an unanalysed import. Close it and open `TelescreenRE.gpr` instead - never a raw
`teled.stripped` imported by hand.

---

## 31.9 Troubleshooting

| Symptom | Cause | Fix |
| ------- | ----- | --- |
| `docker: command not found` | Docker not installed / not started | start Docker Desktop; on Linux add yourself to the `docker` group and re-login |
| `docker run --platform linux/arm64` prints `Exec format error` | ARM64 emulation missing (x64 hosts) | Linux: install `qemu-user-static`; Windows/macOS: enable in Docker Desktop |
| Ghidra Decompile window is empty on macOS | the `mac_arm_64` natives were never built | do §31.3 Step 5 |
| `analyzeHeadless: JAVA_HOME not set` | Java not on PATH for the script | `export JAVA_HOME=.../jdk-21...` before running |
| Ghidra says `UnsupportedClassVersionError ... class file version 71.0 > 65.0` | two JDKs, a stale script cache | use JDK 21 only; `rm -rf ~/Library/ghidra/*/osgi/compiled-bundles` |
| GUI "freezes" right after opening a program | a hidden **"Reposition Program?"** dialog | press Return; then disable it in Edit → Tool Options → Navigation |
| `build_target.sh: Permission denied` | the file is not executable | `chmod +x firmware/build_target.sh ghidra/*.sh` |
| Windows path errors under Git Bash | MSYS path translation | use WSL2 Ubuntu instead (recommended) |

## 31.10 What you have now

- a reproducible ARM64 target (`firmware/teled.stripped`) and its answer key,
- a fully analysed Ghidra project (`ghidra/proj/TelescreenRE.gpr`),
- the raw decompilation (`ghidra/decomp/`),
- and the resolved map (`ghidra/RESOLUTION_MAP.md`).

Volume 32 explains where the four firmware images come from and how to carve
them. Volume 33 teaches you to resolve every function in the binary.

## 31.11 Real hardware (optional)

None of the reverse engineering needs a Raspberry Pi. If you *do* want to run the
lab on real silicon, **[Walkthrough 78](walkthrough/78-raspberry-pi-bringup.md)**
takes you from a blank microSD card to a booting, headless Raspberry Pi **4B or
5** with a serial console and a working camera - including where to download the
latest Raspberry Pi OS image and how to flash it.
