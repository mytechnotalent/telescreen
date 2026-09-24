#!/usr/bin/env python3
"""Regression tests for the lab's Ghidra resolution tooling.

Asserts golden invariants on `resolve_functions.resolve()` (counts, exact
address->name mappings, the ICF pair, the phantoms, the PLT0) and on the
generated Appendix J, using the committed ground truth.  Runs anywhere - no
Ghidra required.
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
GHIDRA = HERE.parent
sys.path.insert(0, str(GHIDRA))
import resolve_functions as rf  # noqa: E402

APPENDIX = GHIDRA.parent / "docs" / "appendix" / "J-ghidra-function-resolution.md"


def main() -> int:
    """
    Run the tooling assertions.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero when every check passes.
    """
    rows = rf.resolve()
    kinds: dict[str, int] = {}
    for r in rows:
        kinds[r["kind"]] = kinds.get(r["kind"], 0) + 1
    by_addr = {r["addr"]: r for r in rows}
    symbols = rf.load_symbols()

    def named(addr: int) -> str:
        return by_addr[addr]["name"]

    checks = [
        ("total functions == 127", len(rows) == 127),
        ("application == 42", kinds.get("application") == 42),
        ("imports == 72", kinds.get("import") == 72),
        ("phantoms == 2", kinds.get("phantom") == 2),
        ("runtime == 11", kinds.get("runtime") == 11),
        ("PLT0 == 0x400d20", rf.find_plt0() == 0x400D20),
        ("0x401d80 -> crc32_le", named(0x401D80) == "crc32_le"),
        ("0x401ae0 -> beacon_weak_key", named(0x401AE0) == "beacon_weak_key"),
        ("0x400f80 -> main", named(0x400F80) == "main"),
        ("0x401320 -> gcm/chacha_dec_final",
         named(0x401320) in ("gcm_dec_final", "chacha_dec_final")),
        ("0x401b5c phantom of beacon_seal (FUN_00401b60)",
         "FUN_00401b60" in by_addr[0x401B5C]["evidence"]),
        ("0x40205c phantom of identity_sign (FUN_00402060)",
         "FUN_00402060" in by_addr[0x40205C]["evidence"]),
        ("ICF: two code names at 0x401320",
         symbols.get(0x401320, ("", ""))[1] in ("gcm_dec_final", "chacha_dec_final")),
    ]

    # Appendix J has one section per application function.
    text = APPENDIX.read_text() if APPENDIX.exists() else ""
    app_sections = text.count("## `0x")
    checks.append(("Appendix J: 42 function sections", app_sections == 42))
    checks.append(("Appendix J: mentions crc32_le", "`crc32_le`" in text))

    passed = sum(1 for _, p in checks if p)
    for name, good in checks:
        print(("PASS " if good else "FAIL ") + name)
    print(f"{passed}/{len(checks)} checks, {len(checks) - passed} failures")
    return 0 if passed == len(checks) else 1


if __name__ == "__main__":
    raise SystemExit(main())
