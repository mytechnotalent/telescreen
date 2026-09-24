#!/usr/bin/env python3
"""Resolve every function in the stripped TELESCREEN target to a real identity.

This script is the heart of the reverse-engineering course.  It takes the raw,
meaningless names that Ghidra prints for a stripped binary (FUN_00401e00, ...)
and produces:

    1. resolved/                     - a copy of every decompiled function,
                                       renamed to the real function name.
    2. RESOLUTION_MAP.md             - a complete, evidence-backed table that
                                       shows, for every function in the binary,
                                       what it really is and HOW we know.

It deliberately does NOT use any secret.  The three inputs are all things a
student can reproduce:

    ground_truth/symbols.txt   - `nm` output of the UNSTRIPPED twin (instructor)
    ground_truth/imports.txt   - the dynamic imports (.plt targets)
    decomp/_index.txt          - Ghidra's raw address -> FUN_ name map
    decomp/*.c                 - the decompiled bodies

The resolution rules mirror what a professional reverse engineer does, in order
of certainty:

    R1  Exact address match against the unstripped twin.      (certain)
    R2  .plt stub -> JUMP_SLOT relocation -> import name.     (certain)
    R3  PLT0 (the lazy resolver trampoline).                  (certain)
    R4  Phantom/overlapping function: identical body to a
        real function, sitting on alignment padding.          (certain)
"""
from __future__ import annotations

import json
import re
import shutil
from pathlib import Path

HERE = Path(__file__).resolve().parent
GT = HERE / "ground_truth"
DECOMP = HERE / "decomp"
RESOLVED = HERE / "resolved"

# ---------------------------------------------------------------------------
# Load the three ground-truth inputs.
# ---------------------------------------------------------------------------
def load_symbols() -> dict[int, tuple[str, str]]:
    """Return {address: (type, name)} from the unstripped twin."""
    out: dict[int, tuple[str, str]] = {}
    for line in (GT / "symbols.txt").read_text().splitlines():
        parts = line.split()
        if len(parts) >= 3:
            try:
                out[int(parts[0], 16)] = (parts[1], parts[2])
            except ValueError:
                continue
    return out


def load_imports() -> set[str]:
    """Return the set of dynamic import names (.plt targets)."""
    names = set()
    for line in (GT / "imports.txt").read_text().splitlines():
        line = line.strip()
        if line:
            names.add(line.split("@")[0])
    return names


def load_index() -> list[tuple[int, str, str]]:
    """Return [(address, ghidra_name, decomp_file)] from Ghidra's index."""
    rows = []
    for line in (DECOMP / "_index.txt").read_text().splitlines():
        parts = line.split()
        if len(parts) >= 3:
            rows.append((int(parts[0], 16), parts[1], parts[2]))
    rows.sort()
    return rows


# ---------------------------------------------------------------------------
# Map each application function to its source module and its Doxygen @brief.
# We parse the sources directly so the map is always in sync with the code.
# ---------------------------------------------------------------------------
def parse_sources() -> dict[str, tuple[str, str]]:
    """Return {function_name: (module, brief)} parsed from ../src/*.c.

    House style puts the return type and the parameter list on one or more
    lines, then the opening brace on its own line (Allman).  To be robust we
    assemble a whole top-level statement (from a non-indented line up to the
    first '{' or ';') and then decide whether it is a function definition.
    """
    src = HERE.parent / "src"
    result: dict[str, tuple[str, str]] = {}
    brief_re = re.compile(r"@brief\s+(.*)")
    for path in sorted(src.glob("*.c")):
        module = path.stem
        lines = path.read_text().splitlines()
        last_brief = ""
        i = 0
        while i < len(lines):
            line = lines[i]
            m = brief_re.search(line)
            if m:
                last_brief = m.group(1).strip()
            # A top-level statement starts at column 0 with a type/identifier.
            if line and line[0] not in " \t#/*}" and not line.startswith("//"):
                stmt = line
                j = i
                while "{" not in stmt and ";" not in stmt and j + 1 < len(lines):
                    j += 1
                    stmt += " " + lines[j].strip()
                head = stmt.split("{")[0]
                if "{" in stmt and "(" in head:
                    mm = re.search(r"(\w+)\s*\(", head)
                    if mm:
                        result[mm.group(1)] = (module, last_brief)
                    last_brief = ""
                i = j + 1
                continue
            i += 1
    return result


# ---------------------------------------------------------------------------
# Resolution.
# ---------------------------------------------------------------------------
RUNTIME = {
    "_init", "_fini", "_start", "__libc_csu_init", "__libc_csu_fini",
    "call_weak_fn", "deregister_tm_clones", "register_tm_clones",
    "__do_global_dtors_aux", "frame_dummy", "_dl_relocate_static_pie",
    "main",
}

# Weak undefined symbols that the C runtime references but that are resolved to
# zero at run time.  They are not application code.
WEAK = {
    "_ITM_deregisterTMCloneTable",
    "_ITM_registerTMCloneTable",
    "__gmon_start__",
}


def find_plt0() -> int:
    """Return the address of the `.plt` section (the PLT0 lazy resolver).

    Every binary has its own address for this, so we read it from
    `readelf -SW` rather than hard-coding it.
    """
    for line in (GT / "sections.txt").read_text().splitlines():
        m = re.search(r"\]\s+\.plt\s+\S+\s+([0-9a-f]+)", line)
        if m:
            return int(m.group(1), 16)
    return -1


def resolve() -> list[dict]:
    symbols = load_symbols()
    imports = load_imports()
    sources = parse_sources()
    plt0 = find_plt0()
    rows = []
    for addr, ghidra_name, cfile in load_index():
        entry: dict = {"addr": addr, "ghidra": ghidra_name, "file": cfile}
        if addr == plt0:
            entry.update(kind="runtime", name="<.plt> PLT0 resolver", module="-",
                         evidence="R3: linker-provided lazy-binding trampoline")
        elif addr in symbols:
            _, name = symbols[addr]
            module, brief = sources.get(name, ("-", ""))
            if name in RUNTIME:
                entry.update(kind="runtime", name=name, module="-",
                             evidence="R1: exact nm match (C runtime start-up)")
            else:
                entry.update(kind="application", name=name, module=module,
                             brief=brief,
                             evidence="R1: exact nm match against the unstripped twin")
        elif ghidra_name in imports or ghidra_name in WEAK:
            entry.update(kind="import", name=ghidra_name, module="lib",
                         evidence="R2: .plt stub / weak symbol -> import name")
        else:
            # Phantom functions: their body is identical to a real function.
            twin = find_phantom_twin(cfile)
            entry.update(kind="phantom", name=f"(phantom of {twin})", module="-",
                         evidence=f"R4: identical body to {twin}; alignment padding")
        rows.append(entry)
    return rows


# Ghidra invents names like local_18 / uStack_14 / iVar1.  Two functions that
# are in fact identical differ only in these auto-generated names, so we blank
# them before comparing bodies.
_AUTO_NAME = re.compile(
    r"\b(?:[iulsbpfc]?[A-Za-z]*Stack_[0-9a-fA-F]+"
    r"|local_[0-9a-fA-F]+"
    r"|[iulsbpfc]Var[0-9]+"
    r"|[A-Za-z]*_[0-9a-fA-F]+)\b"
)


def _normalise(text: str) -> str:
    """Blank Ghidra's auto-generated variable names so bodies can be compared."""
    return _AUTO_NAME.sub("V", text)


def find_phantom_twin(cfile: str) -> str:
    """Return the Ghidra name of the function this phantom duplicates."""
    body = _normalise(strip_header((DECOMP / cfile).read_text()))
    for other in sorted(DECOMP.glob("*_FUN_*.c")):
        if other.name == cfile:
            continue
        if _normalise(strip_header(other.read_text())) == body:
            return other.stem.split("_", 1)[1]
    return "unknown"


def strip_header(text: str) -> str:
    """Remove the first 3 lines (blank + signature + '{') for body comparison."""
    return "\n".join(text.splitlines()[3:])


# ---------------------------------------------------------------------------
# Output.
# ---------------------------------------------------------------------------
def write_resolved(rows: list[dict]) -> None:
    if RESOLVED.exists():
        shutil.rmtree(RESOLVED)
    RESOLVED.mkdir(parents=True)
    for entry in rows:
        src = DECOMP / entry["file"]
        if not src.exists():
            continue
        safe = re.sub(r"[^A-Za-z0-9_.-]", "_", entry["name"])
        out = RESOLVED / f'{entry["addr"]:08x}_{safe}.c'
        shutil.copyfile(src, out)


def write_map(rows: list[dict]) -> None:
    app = [r for r in rows if r["kind"] == "application"]
    imports = [r for r in rows if r["kind"] == "import"]
    phantom = [r for r in rows if r["kind"] == "phantom"]
    runtime = [r for r in rows if r["kind"] == "runtime"]

    lines = []
    lines.append("# TELESCREEN - Function Resolution Map")
    lines.append("")
    lines.append("Every function Ghidra found in the stripped target, resolved to a")
    lines.append("real identity, with the evidence that proves it.  Totals:")
    lines.append("")
    lines.append(f"- **{len(app)}** application functions (our firmware code)")
    lines.append(f"- **{len(imports)}** imported-library thunks (OpenSSL / glibc)")
    lines.append(f"- **{len(phantom)}** phantom functions (alignment padding)")
    lines.append(f"- **{len(runtime)}** C runtime / start-up functions")
    lines.append(f"- **{len(rows)}** total functions in the binary")
    lines.append("")
    lines.append("## Application functions (resolved 1:1 against the unstripped twin)")
    lines.append("")
    lines.append("| address | Ghidra label | resolved name | module | role |")
    lines.append("| ------- | ------------ | ------------- | ------ | ---- |")
    for r in app:
        brief = r.get("brief", "").replace("|", "\\|")
        lines.append(f'| `0x{r["addr"]:08x}` | `{r["ghidra"]}` | `{r["name"]}` | '
                     f'{r["module"]} | {brief} |')
    lines.append("")
    lines.append("## Imported-library thunks")
    lines.append("")
    lines.append("| address | resolved name |")
    lines.append("| ------- | ------------- |")
    for r in imports:
        lines.append(f'| `0x{r["addr"]:08x}` | `{r["name"]}` |')
    lines.append("")
    lines.append("## Phantom functions (Ghidra artefacts - NOT real functions)")
    lines.append("")
    for r in phantom:
        lines.append(f'- `0x{r["addr"]:08x}` (`{r["ghidra"]}`): {r["evidence"]}')
    lines.append("")
    lines.append("## C runtime / start-up")
    lines.append("")
    for r in runtime:
        lines.append(f'- `0x{r["addr"]:08x}` (`{r["ghidra"]}`) -> `{r["name"]}`')
    lines.append("")
    (HERE / "RESOLUTION_MAP.md").write_text("\n".join(lines) + "\n")
    (HERE / "resolution.json").write_text(json.dumps(rows, indent=2) + "\n")


def main() -> None:
    rows = resolve()
    write_resolved(rows)
    write_map(rows)
    app = sum(1 for r in rows if r["kind"] == "application")
    print(f"resolved {len(rows)} functions "
          f"({app} application) -> RESOLUTION_MAP.md + resolved/")


if __name__ == "__main__":
    main()
