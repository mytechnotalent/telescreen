#!/usr/bin/env bash
#
# build_target.sh - build the TELESCREEN reverse-engineering target.
#
# WHAT THIS SCRIPT DOES (plain language, no magic):
#
#   1. Compiles every C file in ../src into ONE Linux aarch64 (ARM 64-bit)
#      executable.  aarch64 is the CPU architecture used by the Raspberry Pi 5
#      and by the TELESCREEN device this lab is modelled on.
#   2. Compiles it TWICE, producing two files:
#
#        teled.unstripped  - the same program WITH its symbol table still in it.
#                            This is the answer key.  The instructor uses it to
#                            prove that every function the student recovers is
#                            correct.  It is byte-for-byte the same code as the
#                            stripped file; only the names are different.
#
#        teled.stripped    - the same program with ALL symbol names removed.
#                            This is what the student receives.  A stripped
#                            binary shows functions only as FUN_00401e00 and
#                            so on, exactly like a real captured device.
#
#   3. Prints the SHA-256 of each file so you can prove later that you are
#      analysing the exact same bytes we did.
#
# WHY DOCKER (and why this works identically on Windows, Linux and macOS):
#
#   We build the target inside a CONTAINER so that the compiler is the exact
#   same program on every host operating system.  The container platform is
#   pinned to linux/arm64 ("PLATFORM" below).  That gives three properties:
#
#     * macOS arm64 (Apple Silicon): the container runs NATIVELY at full speed.
#     * Windows x64 / Linux x64     : Docker runs the same arm64 image through
#                                     its built-in emulation.  Emulation is
#                                     slower, but the COMPILER IS THE SAME
#                                     BINARY, so the output is byte-identical.
#
#   Because the container image + compiler + flags are fixed, the SHA-256 of
#   teled.stripped is the SAME on all three host OSes.  That is what lets every
#   address in this course's Ghidra notes (0x401e00, ...) be true for everyone.
#
# HOW TO RUN IT:
#
#   macOS / Linux / WSL2 / Git Bash:   ./firmware/build_target.sh
#
#   Windows (native PowerShell/cmd) does not run bash scripts; use one of:
#     (a) WSL2 (recommended):  open Ubuntu, then run the line above; or
#     (b) Git Bash:            run the line above.
#   See docs/31-prerequisites-and-install.md for a step-by-step Windows guide.
#
# REQUIREMENTS:
#
#   - Docker Desktop (running)
#   - Internet access the first time only (to download the small gcc:14 image)
#
# PLATFORM OVERRIDE (optional):
#
#   PLATFORM=linux/arm64 ./firmware/build_target.sh
#   The default is already linux/arm64 and should not be changed if you want
#   the published addresses/hashes to match.
#
set -euo pipefail

# --- Locate directories ------------------------------------------------------
# HERE = the directory this script lives in (…/telescreen/firmware).
HERE="$(cd "$(dirname "$0")" && pwd)"
# ROOT = the repository root (…/telescreen).
ROOT="$(cd "$HERE/.." && pwd)"
# SOURCES = every module's .c file.
SOURCES="$ROOT/src"
# HEADERS = the include directory the compiler needs (-I).
HEADERS="$ROOT/include"
# DOCKER_IMAGE = the tiny Linux aarch64 toolchain image we build once.
DOCKER_IMAGE="telescreen-cross:arm64"
# DOCKERFILE_DIR = scratch directory used to build that image.
DOCKERFILE_DIR="$HERE/.docker"
# PLATFORM = the container platform we pin to.  Do not change this if you want
#            the SHA-256 hashes and Ghidra addresses in the docs to match.
PLATFORM="${PLATFORM:-linux/arm64}"

echo "[*] Repository root : $ROOT"
echo "[*] Sources          : $SOURCES"
echo "[*] Headers          : $HEADERS"

# --- Sanity checks -----------------------------------------------------------
if ! command -v docker >/dev/null 2>&1; then
    echo "[!] Docker is not installed or not on your PATH."
    echo "    Install Docker Desktop, start it, and re-run this script."
    exit 1
fi

if [ ! -d "$SOURCES" ]; then
    echo "[!] Could not find the source directory: $SOURCES"
    exit 1
fi

# --- Build the toolchain image (once) ---------------------------------------
# The base gcc:14 image does not ship OpenSSL's development headers, and our
# crypto modules (#include <openssl/evp.h>) need them, so we add libssl-dev.
mkdir -p "$DOCKERFILE_DIR"
cat > "$DOCKERFILE_DIR/Dockerfile" <<'DOCKERFILE'
FROM gcc:14
RUN apt-get update -qq \
 && apt-get install -y -qq libssl-dev >/dev/null 2>&1 \
 && rm -rf /var/lib/apt/lists/*
DOCKERFILE

echo "[*] Ensuring toolchain image exists: $DOCKER_IMAGE"
docker build -q -t "$DOCKER_IMAGE" "$DOCKERFILE_DIR" >/dev/null

# --- Compile ----------------------------------------------------------------
# -O2               : the optimisation level a real vendor build would use.
#                     It is deliberately NOT -O0; students must meet real code.
# -I"$HEADERS"      : tell the compiler where our headers are.
# -lcrypto          : link OpenSSL (AES-GCM, ChaCha20-Poly1305, X25519, Ed25519).
# We do NOT use -flto, so functions in different files are never merged and the
# function boundaries stay clean - exactly the situation on real firmware.
echo "[*] Compiling lab modules -> teled.unstripped"
docker run --rm --platform "$PLATFORM" \
    -v "$ROOT":/repo \
    -w /repo/firmware \
    "$DOCKER_IMAGE" bash -lc '
        set -e
        gcc -O2 -Wall -Wextra \
            -I/repo/include \
            -o teled.unstripped \
            /repo/src/*.c \
            -lcrypto
        cp teled.unstripped teled.stripped
        strip --strip-all teled.stripped
        echo "    unstripped: $(stat -c %s teled.unstripped) bytes"
        echo "    stripped  : $(stat -c %s teled.stripped) bytes"
    '

# --- Report hashes -----------------------------------------------------------
echo
echo "[*] Build complete. Artefacts:"
echo "    $HERE/teled.unstripped   (the answer key - keep for the instructor)"
echo "    $HERE/teled.stripped     (the student target - fully stripped)"
echo
echo "[*] SHA-256:"
( cd "$HERE" && shasum -a 256 teled.unstripped teled.stripped )

# --- Ground truth for the Ghidra resolution pipeline ------------------------
# nm/readelf/objdump of the freshly built target, so ghidra/resolve_functions.py
# and gen_appendix_j.py always match the binary.
GT_DIR="$ROOT/ghidra/ground_truth"
if [ -d "$ROOT/ghidra" ]; then
    mkdir -p "$GT_DIR"
    echo "[*] Capturing ground truth (nm/readelf/objdump) -> ghidra/ground_truth"
    docker run --rm --platform "$PLATFORM" \
        -v "$HERE":/fw -v "$GT_DIR":/gt -w /fw "$DOCKER_IMAGE" bash -lc '
            nm --defined-only teled.unstripped | sort > /gt/symbols.txt
            readelf -rW teled.stripped | awk "/JUMP_SLOT/ {print \$5}" | sort -u > /gt/imports.txt
            readelf -h teled.stripped > /gt/elf_header.txt
            readelf -SW teled.stripped > /gt/sections.txt
            readelf -rW teled.stripped > /gt/relocs.txt
            objdump -d teled.unstripped > /gt/disasm.txt
        '
fi

echo
echo "[*] Proving the stripped target runs natively (aarch64 Linux in Docker):"
docker run --rm --platform "$PLATFORM" \
    -v "$HERE":/fw -w /fw "$DOCKER_IMAGE" \
    bash -lc './teled.stripped; echo "    exit code = $?"'
