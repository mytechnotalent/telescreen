#!/usr/bin/env bash
#
# decompile.sh - run Ghidra's decompiler over every function and write one .c
#                file per function into  ./decomp/ .
#
# This is the SAME technique a professional reverse engineer uses to turn a
# stripped firmware binary into a readable, function-by-function listing.
#
# You do not strictly need this script to learn (the GUI shows the same thing),
# but it is how the reference output in  ./decomp/  was produced, and it is how
# you would build a report at scale.
#
# HOW TO RUN:
#
#   ./ghidra/decompile.sh
#
# PREREQUISITES:
#
#   1. Run ./ghidra/make_project.sh first (creates the analysed project).
#   2. The Ghidra macOS-arm64 native decompiler must be present; verify with:
#        ls ~/ghidra/Ghidra/Features/Decompiler/os/    # you want mac_arm_64
#
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
GH="${GH:-$HOME/ghidra}"
OUTDIR="$HERE/decomp"

# Use Temurin 21 if present (see make_project.sh for why).
if [ -x "$HOME/jdk/jdk-21.0.12.1+1/Contents/Home/bin/java" ]; then
    export JAVA_HOME="$HOME/jdk/jdk-21.0.12.1+1/Contents/Home"
fi
if [ -n "${JAVA_HOME:-}" ]; then
    export PATH="$JAVA_HOME/bin:$PATH"
fi

mkdir -p "$OUTDIR"
echo "[*] Exporting decompilation of 'teled.stripped' to $OUTDIR"

# -process teled.stripped : the program inside the project to work on
# -noanalysis             : do not re-analyse (it is already analysed)
# -postScript ExportDecomp.java <dir> : run our export script after loading
"$GH/support/analyzeHeadless" "$HERE/proj" TelescreenRE \
    -process teled.stripped -noanalysis \
    -scriptPath "$HERE" \
    -postScript ExportDecomp.java "$OUTDIR"

echo "[*] Done.  $(ls "$OUTDIR"/*.c 2>/dev/null | wc -l | tr -d ' ') C files + _index.txt in $OUTDIR"
