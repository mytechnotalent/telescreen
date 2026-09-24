#!/usr/bin/env python3
"""Generate Appendix J: the function-by-function reverse-engineering report.

For every application function in the stripped TELESCREEN target this writes:

  * its address and the meaningless name Ghidra shows for it,
  * what it really is (module + role),
  * the machine-disassembly call graph around it (who calls it / it calls),
  * the exact evidence that resolves it, and
  * its decompiled C body.

Nothing here is guessed: the call graph comes from `objdump -d` of the
unstripped twin, and every address is matched 1:1 against `nm`.
"""
from __future__ import annotations

import re
from pathlib import Path

import resolve_functions as rf

HERE = rf.HERE
GT = rf.GT
OUT = HERE.parent / "docs" / "appendix" / "J-ghidra-function-resolution.md"


def load_disasm():
    """Return (funcs_by_addr, calls) parsed from ground_truth/disasm.txt."""
    funcs: dict[int, str] = {}
    calls: list[tuple[int, int]] = []
    cur = None
    fn_re = re.compile(r"^([0-9a-f]{16}) <(.+)>:")
    bl_re = re.compile(r"\bbl\s+([0-9a-f]+) <")
    for line in (GT / "disasm.txt").read_text().splitlines():
        m = fn_re.match(line)
        if m:
            cur = int(m.group(1), 16)
            funcs[cur] = m.group(2)
            continue
        m = bl_re.search(line)
        if m and cur is not None:
            calls.append((cur, int(m.group(1), 16)))
    return funcs, calls


def main() -> None:
    symbols = rf.load_symbols()
    sources = rf.parse_sources()
    funcs, calls = load_disasm()

    name_of: dict[int, str] = {}
    for addr, (typ, name) in symbols.items():
        name_of[addr] = name
    for addr, raw in funcs.items():
        if addr not in name_of:
            name_of[addr] = raw

    callers: dict[int, set[int]] = {}
    callees: dict[int, set[int]] = {}
    for a, b in calls:
        callees.setdefault(a, set()).add(b)
        callers.setdefault(b, set()).add(a)

    def resolve_name(addr: int) -> str:
        return name_of.get(addr, f"sub_{addr:x}")

    rows = [r for r in rf.resolve() if r["kind"] == "application"]
    rows.sort(key=lambda r: r["addr"])

    out: list[str] = []
    w = out.append
    w("# Appendix J - Function-by-Function Reverse Engineering")
    w("")
    w("This appendix is the complete reverse-engineering record of the stripped")
    w("target `firmware/teled.stripped`.  For every one of the 42 application")
    w("functions it gives the address, the meaningless label Ghidra shows, what")
    w("the function really is, the call graph around it extracted from the real")
    w("machine code, the evidence that resolves it, and its decompiled body.")
    w("")
    w("Everything here is reproducible from `firmware/teled.stripped` alone plus")
    w("the instructor's `firmware/teled.unstripped` answer key.")
    w("")
    w("> **The four resolution rules** (see `ghidra/RESOLUTION_MAP.md`):")
    w(">")
    w("> - **R1** exact address match against the unstripped twin (certain),")
    w("> - **R2** `.plt` stub -> `JUMP_SLOT` relocation -> import name (certain),")
    w("> - **R3** the `.plt` PLT0 lazy resolver (certain),")
    w("> - **R4** phantom/overlapping function on alignment padding (certain).")
    w("")
    w("---")
    w("")

    for r in rows:
        addr = r["addr"]
        name = r["name"]
        module = r["module"]
        brief = r.get("brief", "")
        w(f"## `0x{addr:08x}` - `{name}`  ({module})")
        w("")
        w(f"- **Ghidra shows:** `{r['ghidra']}` (a stripped binary has no names).")
        w(f"- **Resolved name:** `{name}`")
        w(f"- **Module:** `src/{module}.c`")
        w(f"- **Role:** {brief or '(see source)'}")
        w(f"- **Evidence:** {r['evidence']}.")
        if addr in callers:
            cs = ", ".join(f"`{resolve_name(c)}`" for c in sorted(callers[addr]))
            w(f"- **Called by ({len(callers[addr])}):** {cs}")
        else:
            w("- **Called by:** _(entry points only)_")
        if addr in callees:
            ce = ", ".join(f"`{resolve_name(c)}`" for c in sorted(callees[addr]))
            w(f"- **Calls ({len(callees[addr])}):** {ce}")
        else:
            w("- **Calls:** _(leaf function)_")
        w("")
        w("```c")
        body = (rf.RESOLVED / f"{addr:08x}_{re.sub(r'[^A-Za-z0-9_.-]', '_', name)}.c")
        if body.exists():
            w(body.read_text().rstrip())
        else:
            w((rf.DECOMP / r["file"]).read_text().rstrip())
        w("```")
        w("")

    # --- Lessons section -----------------------------------------------------
    w("---")
    w("")
    w("## Reverse-engineering lessons this binary teaches")
    w("")
    w("### Lesson 1 - Identical Code Folding (ICF)")
    w("")
    w("`aead_open` contains two paths whose final step is byte-for-byte")
    w("identical.  The linker merged them, so the symbol table points two source")
    w("functions at the SAME address:")
    w("")
    w("```")
    # `symbols` is keyed by address, so a folded pair would collapse; read the
    # raw file instead and group every name that shares an address.
    by_addr: dict[int, list[tuple[str, str]]] = {}
    for line in (GT / "symbols.txt").read_text().splitlines():
        parts = line.split()
        if len(parts) >= 3:
            by_addr.setdefault(int(parts[0], 16), []).append((parts[1], parts[2]))
    for addr, names in sorted(by_addr.items()):
        code = [(t, n) for t, n in names if t in ("t", "T")]
        if len(code) > 1:
            for t, n in code:
                w(f"{addr:016x} {t} {n}")
    w("```")
    w("")
    w("Two names, one address.  A reverse engineer must read the surrounding")
    w("code to decide which one is executing in a given path.")
    w("")
    w("### Lesson 2 - Phantom functions on alignment padding")
    w("")
    w("Modern toolchains align functions to 16 bytes and pad with `nop`.  Ghidra")
    w("can mistake the padding for the start of a small function, producing a")
    w("phantom that overlaps the real one:")
    w("")
    w("- `0x00401b9c` is a phantom whose body is identical to `beacon_seal`")
    w("  (`0x00401ba0`).")
    w("- `0x004020dc` is a phantom whose body is identical to `identity_sign`")
    w("  (`0x004020e0`).")
    w("")
    w("Always confirm a function's true entry with the call graph and the")
    w("prologue (`stp x29, x30, [sp, #-N]!`), never by Ghidra's guess alone.")
    w("")
    w("### Lesson 3 - The dual PLT (with and without pointer authentication)")
    w("")
    w("There are two `.plt` sections (`linux_x86_64`-style `.plt` plus `.plt.sec`),")
    w("so each imported library function appears twice.  Both thunks end in")
    w("`br x17`; the address they branch to is the GOT slot named by the import.")
    w("")
    out_text = "\n".join(out) + "\n"
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(out_text)
    print(f"wrote {OUT} ({len(out_text.splitlines())} lines, {len(rows)} functions)")


if __name__ == "__main__":
    main()
