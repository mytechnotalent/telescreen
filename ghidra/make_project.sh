#!/usr/bin/env bash
#
# make_project.sh - create the Ghidra project for the TELESCREEN target.
#
# WHY WE DO THIS HEADLESSLY (the single most important lesson in Ghidra):
#
#   If you import a file in the Ghidra GUI and close it before the automatic
#   analysis finishes, you save a program that contains RAW BYTES and NO
#   FUNCTIONS.  It looks "frozen": the listing is full of ?? and the decompiler
#   says "No Function".  Tens of thousands of students hit this and think Ghidra
#   is broken.  It is not.
#
#   The fix is to let a headless script do the whole thing at once:
#       import  ->  run the FULL auto-analysis  ->  save
#   so that when you open the project in the GUI, everything is already there.
#
# WHAT THIS SCRIPT DOES:
#
#   1. Finds your Ghidra installation (default: ~/ghidra).
#   2. Imports  ../firmware/teled.stripped  (the fully stripped aarch64 target).
#   3. Runs Ghidra's full auto-analysis on it (about one second for this file).
#   4. Saves the analysed project to  ./proj/TelescreenRE
#
# HOW TO RUN:
#
#   ./ghidra/make_project.sh
#
set -euo pipefail

# HERE = directory of this script (…/telescreen/ghidra)
HERE="$(cd "$(dirname "$0")" && pwd)"
# GH = where Ghidra is installed.  Override with: GH=/path/to/ghidra ./make_project.sh
GH="${GH:-$HOME/ghidra}"
# TARGET = the stripped binary produced by firmware/build_target.sh
TARGET="$HERE/../firmware/teled.stripped"
# OUT = where the Ghidra project will be written.
OUT="$HERE/proj"
# NAME = the project name you will see in the Ghidra GUI.
NAME="TelescreenRE"

# --- Pick a JDK --------------------------------------------------------------
# Ghidra 12.x needs Java 21.  This lab used Temurin 21.  If you installed a JDK
# somewhere else, export JAVA_HOME before running this script.
if [ -x "$HOME/jdk/jdk-21.0.12.1+1/Contents/Home/bin/java" ]; then
    export JAVA_HOME="$HOME/jdk/jdk-21.0.12.1+1/Contents/Home"
fi
if [ -n "${JAVA_HOME:-}" ]; then
    export PATH="$JAVA_HOME/bin:$PATH"
fi
echo "[*] java: $(java -version 2>&1 | head -1)"

# --- Sanity checks -----------------------------------------------------------
if [ ! -x "$GH/support/analyzeHeadless" ]; then
    echo "[!] Could not find Ghidra's analyzeHeadless at: $GH/support/analyzeHeadless"
    echo "    Install Ghidra and/or set GH, e.g.  GH=~/ghidra ./ghidra/make_project.sh"
    exit 1
fi
if [ ! -f "$TARGET" ]; then
    echo "[!] Target not found: $TARGET"
    echo "    Build it first:  ./firmware/build_target.sh"
    exit 1
fi

mkdir -p "$OUT"
echo "[*] Importing and auto-analysing: $TARGET"
echo "[*] Project will be saved at    : $OUT/$NAME.gpr"

# -import                  : the file to bring in
# -processor AARCH64:LE:64:v8A : force the ARM 64-bit little-endian language
#                          (Ghidra would usually detect this from the ELF header,
#                           but being explicit removes all doubt)
# -analysisTimeoutPerFile  : give analysis up to 600 seconds (it needs ~1s here)
"$GH/support/analyzeHeadless" "$OUT" "$NAME" \
    -import "$TARGET" \
    -processor AARCH64:LE:64:v8A \
    -analysisTimeoutPerFile 600

cat <<EOF

[*] Done.  The project is analysed and saved at:
        $OUT/$NAME.gpr

[*] Open it in the Ghidra GUI:
        open ~/Applications/Ghidra.app        # or run  $GH/ghidraRun
        # File -> Open Project -> $OUT/$NAME.gpr
        # double-click  teled.stripped  -> the listing appears immediately
        # click in the listing, press  F  -> decompile the current function
        # press  G  and type an address   -> go to a function (see RESOLUTION_MAP.md)
EOF
