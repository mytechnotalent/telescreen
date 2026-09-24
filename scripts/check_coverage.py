#!/usr/bin/env python3
"""Run native unit tests and verify 100% C source code coverage."""
import os
import subprocess
import sys
from pathlib import Path
from run_tests import (
    _find_compiler,
    _include_flags,
    _openssl_libs,
    _owned_sources,
    _sources,
    _fault_flags,
)


def _compile_coverage_bin(out_bin: Path) -> int:
    """
    Compile test binary with LLVM coverage instrumentation.

    Parameters
    ----------
    out_bin : pathlib.Path
        Output test binary path.

    Returns
    -------
    int
        Compiler return code.
    """
    flags = ["-fprofile-instr-generate", "-fcoverage-mapping", "-O0", "-g"]
    flags += ["-Wall", "-Wextra", "-fno-builtin", "-o", str(out_bin)]
    cmd = [_find_compiler()] + flags + _fault_flags() + _include_flags()
    cmd += _sources() + _openssl_libs()
    return subprocess.run(cmd).returncode


def _execute_prof(out_bin: Path, profraw: Path) -> int:
    """
    Execute instrumented test binary emitting profraw data.

    Parameters
    ----------
    out_bin : pathlib.Path
        Test executable path.
    profraw : pathlib.Path
        Raw profile output path.

    Returns
    -------
    int
        Process return code.
    """
    env = dict(os.environ, LLVM_PROFILE_FILE=str(profraw))
    return subprocess.run([str(out_bin)], env=env).returncode


def _merge_profile(profraw: Path, profdata: Path) -> int:
    """
    Merge raw profile into indexed profdata.

    Parameters
    ----------
    profraw : pathlib.Path
        Raw profile path.
    profdata : pathlib.Path
        Indexed profile output path.

    Returns
    -------
    int
        llvm-profdata return code.
    """
    cmd = ["xcrun", "llvm-profdata", "merge", "-sparse", str(profraw)]
    cmd += ["-o", str(profdata)]
    return subprocess.run(cmd).returncode


def _report_coverage(out_bin: Path, profdata: Path) -> str:
    """
    Generate coverage text report for owned C files.

    Parameters
    ----------
    out_bin : pathlib.Path
        Instrumented test binary.
    profdata : pathlib.Path
        Indexed profdata path.

    Returns
    -------
    str
        Summary coverage report output.
    """
    cmd = ["xcrun", "llvm-cov", "report", str(out_bin)]
    cmd += [f"-instr-profile={profdata}"] + _owned_sources()
    res = subprocess.run(cmd, capture_output=True, text=True)
    return res.stdout


# Owned application modules that must meet the coverage gate.
OWNED = ("aead.c", "partition.c", "crc.c", "env.c", "jffs2.c", "container.c",
         "beacon.c", "kex.c", "identity.c", "collector.c", "teled.c", "camera.c")
# Gate: every owned file must be 100% line and 100% function; the aggregate
# must additionally reach this branch floor (a regression guard; raise it as
# error-path tests are added).
MIN_TOTAL_BRANCH = 100.0
MIN_FILE_BRANCH = 100.0


def _row_pcts(line: str) -> list[float] | None:
    """
    Extract the four percentages (region, function, line, branch) from a row.

    Parameters
    ----------
    line : str
        One llvm-cov report row.

    Returns
    -------
    list[float] or None
        The four percentages, or None when the row is not a file row.
    """
    import re
    pcts = re.findall(r"(\d+\.\d+)%", line)
    return [float(p) for p in pcts] if len(pcts) == 4 else None


def _verify_full_coverage(report: str) -> bool:
    """
    Verify 100% line + function and the branch floor on all owned files.

    Parameters
    ----------
    report : str
        Coverage table text.

    Returns
    -------
    bool
        True when every file passes line/function/branch and the total branch
        floor is met.
    """
    print(report)
    ok = True
    total_branch = None
    for line in report.splitlines():
        pcts = _row_pcts(line)
        if pcts is None:
            continue
        name = line.split()[0]
        region, func, ln, br = pcts
        if name == "TOTAL":
            total_branch = br
            continue
        if any(src == name for src in OWNED):
            if ln < 100.0 or func < 100.0 or br < MIN_FILE_BRANCH:
                print(f"  gate FAIL {name}: line={ln}% function={func}% "
                      f"branch={br}% (need line/function 100%, branch >= "
                      f"{MIN_FILE_BRANCH}%)")
                ok = False
    if total_branch is None or total_branch < MIN_TOTAL_BRANCH:
        print(f"  gate FAIL TOTAL branch={total_branch}% "
              f"(need >= {MIN_TOTAL_BRANCH}%)")
        ok = False
    return ok


def _generate_profile(out_dir: Path) -> Path | None:
    """
    Build, execute instrumented tests, and merge profile data.

    Parameters
    ----------
    out_dir : pathlib.Path
        Output directory for profile artifacts.

    Returns
    -------
    pathlib.Path or None
        Merged profile data path, or None on failure.
    """
    out_bin = out_dir / "test_cov"
    profraw = out_dir / "test_cov.profraw"
    profdata = out_dir / "test_cov.profdata"
    if _compile_coverage_bin(out_bin) != 0:
        return None
    if _execute_prof(out_bin, profraw) != 0:
        return None
    if _merge_profile(profraw, profdata) != 0:
        return None
    return profdata


def main() -> int:
    """
    Execute coverage pipeline and verify 100% coverage.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero on 100% coverage success, otherwise non-zero.
    """
    out_dir = Path("build/test")
    out_dir.mkdir(parents=True, exist_ok=True)
    profdata = _generate_profile(out_dir)
    if profdata is None:
        return 1
    report = _report_coverage(out_dir / "test_cov", profdata)
    return 0 if _verify_full_coverage(report) else 1


if __name__ == "__main__":
    sys.exit(main())