# Appendix D: Tooling Source Listing

Every script, verbatim.

***
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. 

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:

1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws regarding cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**

***

## `scripts/audit_c_standard.py`

```python
#!/usr/bin/env python3
"""Audit owned C files against the strict C standard.

Checks the no-blank-line-in-function-body rule, the eight-executable-line
function limit, and function-level Doxygen documentation.
"""
from pathlib import Path

EXCLUDED_PARTS = (
    "build",
    "packet_artifact.h",
    "generated/",
    ".venv/",
    "node_modules/",
)


def _owned_files() -> list[Path]:
    """
    Find owned C and header files.

    Parameters
    ----------
    None

    Returns
    -------
    list[pathlib.Path]
        Owned non-generated C and header paths.
    """
    paths = Path(".").glob("**/*")
    return sorted(path for path in paths if _is_owned(path))


def _is_owned(path: Path) -> bool:
    """
    Determine whether a path is in the audit scope.

    Parameters
    ----------
    path : pathlib.Path
        Candidate source path.

    Returns
    -------
    bool
        True when the path is an owned C or header file.
    """
    name = str(path)
    if any(part in name for part in EXCLUDED_PARTS):
        return False
    return path.suffix in {".c", ".h"}


def _strip_line(line: str, in_block: bool) -> tuple[str, bool]:
    """
    Remove comments and string literals from one line.

    Parameters
    ----------
    line : str
        Source line.
    in_block : bool
        Whether a block comment is already open.

    Returns
    -------
    tuple[str, bool]
        Code-only text and the updated block-comment state.
    """
    out = []
    i = 0
    in_str = False
    in_chr = False
    while i < len(line):
        ch = line[i]
        if in_block:
            if ch == "*" and i + 1 < len(line) and line[i + 1] == "/":
                in_block = False
                i += 2
                continue
            i += 1
            continue
        if in_str or in_chr:
            if ch == "\\":
                i += 2
                continue
            if in_str and ch == '"':
                in_str = False
            elif in_chr and ch == "'":
                in_chr = False
            i += 1
            continue
        if ch == "/" and i + 1 < len(line) and line[i + 1] == "*":
            in_block = True
            i += 2
            continue
        if ch == "/" and i + 1 < len(line) and line[i + 1] == "/":
            break
        if ch == '"':
            in_str = True
            i += 1
            continue
        if ch == "'":
            in_chr = True
            i += 1
            continue
        out.append(ch)
        i += 1
    return "".join(out), in_block


def _records(path: Path) -> list[dict]:
    """
    Collect function records for one file.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.

    Returns
    -------
    list[dict]
        Function records with name, bounds, and executable line numbers.
    """
    state = _scan_state(path)
    for number, line in enumerate(state["lines"], 1):
        _scan_line(state, line, number)
    return state["records"]


def _scan_state(path: Path) -> dict:
    """
    Build the mutable scanner state for one file.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.

    Returns
    -------
    dict
        Scanner accumulation state.
    """
    lines = path.read_text(encoding="utf-8").splitlines()
    return {"lines": lines, "records": [], "depth": 0, "in_block": False,
            "start": None, "name": "", "sig": 0, "opened": 0, "body": []}


def _scan_line(state: dict, line: str, number: int) -> None:
    """
    Apply one source line to the scanner state.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    line : str
        Raw source line.
    number : int
        One-based line number.

    Returns
    -------
    None
    """
    code, state["in_block"] = _strip_line(line, state["in_block"])
    _scan_open(state, code, number)
    active = state["start"] is not None
    if active and number not in (state["start"], state["opened"]):
        _collect(code, number, state["body"])
    state["depth"] += code.count("{") - code.count("}")
    _scan_close(state, number)


def _scan_open(state: dict, code: str, number: int) -> None:
    """
    Detect a function signature and body opening.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    code : str
        Comment-free source line.
    number : int
        One-based line number.

    Returns
    -------
    None
    """
    if state["start"] is not None or state["depth"] != 0:
        return
    if _sig_start(code):
        state["sig"] = number
        state["name"] = code.split("(")[0].strip()
    if state["sig"] and "{" in code:
        _start_body(state, number)


def _start_body(state: dict, number: int) -> None:
    """
    Open a new function body at the given line.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    number : int
        One-based opening line number.

    Returns
    -------
    None
    """
    state["start"] = state["sig"]
    state["opened"] = number
    state["body"] = []
    state["sig"] = 0


def _scan_close(state: dict, number: int) -> None:
    """
    Close the current function body when its depth returns to zero.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    number : int
        One-based line number.

    Returns
    -------
    None
    """
    if state["start"] is None or state["depth"] > 0 or number <= state["start"]:
        return
    state["records"].append(_record(state, number))
    state["start"] = None
    state["name"] = ""
    state["sig"] = 0
    state["opened"] = 0


def _sig_start(code: str) -> bool:
    """
    Detect the first line of a function signature.

    Parameters
    ----------
    code : str
        Comment-free source line.

    Returns
    -------
    bool
        True when the line begins a function signature.
    """
    text = code.strip()
    if not text or text.startswith("#") or text.startswith("}"):
        return False
    if "(" not in text or text.endswith(";"):
        return False
    return "=" not in text.split("(")[0]


def _collect(code: str, number: int, body: list[tuple[int, str]]) -> None:
    """
    Append one executable or declaration line to the body list.

    Parameters
    ----------
    code : str
        Comment-free source line.
    number : int
        One-based line number.
    body : list[tuple[int, str]]
        Mutable body accumulator.

    Returns
    -------
    None
    """
    text = code.strip()
    if not text or text in {"{", "}"} or text.startswith("}"):
        return
    body.append((number, text))


def _record(state: dict, end: int) -> dict:
    """
    Build a function record from the scanner state.

    Parameters
    ----------
    state : dict
        Scanner accumulation state.
    end : int
        Closing brace line number.

    Returns
    -------
    dict
        Function record.
    """
    return {
        "name": state["name"],
        "start": state["start"],
        "end": end,
        "count": len(state["body"]),
    }


def _blank_lines(path: Path, records: list[dict]) -> list[int]:
    """
    Find blank lines inside function bodies.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.
    records : list[dict]
        Function records for the file.

    Returns
    -------
    list[int]
        One-based blank-line numbers.
    """
    lines = path.read_text(encoding="utf-8").splitlines()
    spans = [(r["start"], r["end"]) for r in records]
    return [
        number
        for number, line in enumerate(lines, 1)
        if not line.strip() and _inside(number, spans)
    ]


def _inside(number: int, spans: list[tuple[int, int]]) -> bool:
    """
    Check whether a line lies inside a function span.

    Parameters
    ----------
    number : int
        One-based source line number.
    spans : list[tuple[int, int]]
        Function start and end lines.

    Returns
    -------
    bool
        True when the line is inside a function body.
    """
    return any(start < number < end for start, end in spans)


def _doc_errors(path: Path, records: list[dict]) -> list[str]:
    """
    Find functions missing a preceding Doxygen @brief block.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.
    records : list[dict]
        Function records for the file.

    Returns
    -------
    list[str]
        Human-readable documentation errors.
    """
    lines = path.read_text(encoding="utf-8").splitlines()
    errors = []
    for record in records:
        if not _has_doc(lines, record["start"]):
            errors.append(f"{path}: {record['name']} missing @brief")
    return errors


def _has_doc(lines: list[str], start: int) -> bool:
    """
    Check for a Doxygen block immediately above a function.

    Parameters
    ----------
    lines : list[str]
        Source lines.
    start : int
        One-based signature line number.

    Returns
    -------
    bool
        True when a @brief doc block precedes the signature.
    """
    index = start - 2
    while index >= 0 and not lines[index].strip():
        index -= 1
    if index < 0 or "*/" not in lines[index]:
        return False
    block = []
    while index >= 0:
        block.append(lines[index])
        if "/**" in lines[index]:
            break
        index -= 1
    return any("@brief" in line for line in block)


def main() -> int:
    """
    Audit all owned C and header files.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero for a clean audit, otherwise one.
    """
    failures = 0
    for path in _owned_files():
        failures += _audit_file(path)
    return int(bool(failures))


def _audit_file(path: Path) -> int:
    """
    Audit one file and print its violations.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.

    Returns
    -------
    int
        Number of violation groups found.
    """
    records = _records(path)
    blank = _blank_lines(path, records)
    long = [r for r in records if r["count"] > 8]
    docs = _doc_errors(path, records)
    _report(path, blank, long, docs)
    return int(bool(blank) or bool(long) or bool(docs))


def _report(path: Path, blank: list[int], long: list[dict],
            docs: list[str]) -> None:
    """
    Print one file's audit findings.

    Parameters
    ----------
    path : pathlib.Path
        C or header source path.
    blank : list[int]
        Blank line numbers inside bodies.
    long : list[dict]
        Functions over the executable-line limit.
    docs : list[str]
        Documentation errors.

    Returns
    -------
    None
    """
    if blank:
        print(f"{path}: blank lines in body {blank}")
    for record in long:
        print(f"{path}: {record['name']} has {record['count']} exec lines")
    for error in docs:
        print(error)


if __name__ == "__main__":
    raise SystemExit(main())
```

## `scripts/audit_python_standard.py`

```python
#!/usr/bin/env python3
"""Audit owned Python tooling for the repository Python standard."""
import ast
from pathlib import Path


def _python_files() -> list[Path]:
    """
    Find owned Python tooling files.

    Parameters
    ----------
    None

    Returns
    -------
    list[pathlib.Path]
        Python files under the scripts directory.
    """
    return sorted(Path("scripts").rglob("*.py"))


def _doc_errors(node: ast.AST) -> list[str]:
    """
    Check one module or function docstring.

    Parameters
    ----------
    node : ast.AST
        Module, function, or class syntax node.

    Returns
    -------
    list[str]
        Documentation error messages.
    """
    if isinstance(node, ast.Module):
        return [] if ast.get_docstring(node) else ["missing module docstring"]
    doc = ast.get_docstring(node, clean=False) or ""
    required = ("Parameters", "Returns")
    return [f"missing {item} section" for item in required if item not in doc]


def _walk_doc_errors(tree: ast.AST, path: Path) -> list[str]:
    """
    Collect missing documentation section errors.

    Parameters
    ----------
    tree : ast.AST
        Parsed Python module.
    path : pathlib.Path
        Source path used in diagnostics.

    Returns
    -------
    list[str]
        Documentation diagnostics.
    """
    kinds = (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)
    nodes = [tree] + [
        item for item in ast.walk(tree) if isinstance(item, kinds)
    ]
    return [
        f"{path}: {type(node).__name__}: {error}"
        for node in nodes
        for error in _doc_errors(node)
    ]


def _blank_function_lines(path: Path) -> list[str]:
    """
    Find blank physical lines inside Python function spans.

    Parameters
    ----------
    path : pathlib.Path
        Python source path.

    Returns
    -------
    list[str]
        Diagnostics for blank lines inside function spans.
    """
    lines = path.read_text(encoding="utf-8").splitlines()
    tree = ast.parse("\n".join(lines), filename=str(path))
    spans = _function_spans(tree)
    doc_lines = _docstring_lines(tree)
    return _blank_lines(path, lines, spans, doc_lines)


def _function_spans(tree: ast.AST) -> list[tuple[int, int]]:
    """
    Return executable spans for Python functions.

    Parameters
    ----------
    tree : ast.AST
        Parsed Python module.

    Returns
    -------
    list[tuple[int, int]]
        Function start and end line pairs.
    """
    kinds = (ast.FunctionDef, ast.AsyncFunctionDef)
    return [
        (node.lineno, node.end_lineno)
        for node in ast.walk(tree)
        if isinstance(node, kinds)
    ]


def _docstring_lines(tree: ast.AST) -> set[int]:
    """
    Return physical lines occupied by Python docstrings.

    Parameters
    ----------
    tree : ast.AST
        Parsed Python module.

    Returns
    -------
    set[int]
        One-based docstring line numbers.
    """
    kinds = (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)
    nodes = [tree] + [
        node for node in ast.walk(tree) if isinstance(node, kinds)
    ]
    return {line for node in nodes for line in _node_doc_lines(node)}


def _node_doc_lines(node: ast.AST) -> range:
    """
    Return the line range of one node's docstring.

    Parameters
    ----------
    node : ast.AST
        Module, class, or function node.

    Returns
    -------
    range
        One-based docstring line range, or an empty range.
    """
    body = getattr(node, "body", [])
    first = body[0] if body else None
    value = getattr(first, "value", None)
    has_doc = isinstance(value, ast.Constant) and isinstance(value.value, str)
    return range(first.lineno, first.end_lineno + 1) if has_doc else range(0)


def _blank_lines(
    path: Path,
    lines: list[str],
    spans: list[tuple[int, int]],
    doc_lines: set[int],
) -> list[str]:
    """
    Format blank executable-line diagnostics.

    Parameters
    ----------
    path : pathlib.Path
        Source path.
    lines : list[str]
        Source lines.
    spans : list[tuple[int, int]]
        Function spans.
    doc_lines : set[int]
        Docstring line numbers.

    Returns
    -------
    list[str]
        Blank-line diagnostics.
    """
    return [
        f"{path}:{number}: blank line in function"
        for number, line in enumerate(lines, 1)
        if number not in doc_lines
        and not line.strip()
        and any(start < number < end for start, end in spans)
    ]


def _file_errors(path: Path) -> list[str]:
    """
    Audit one Python file.

    Parameters
    ----------
    path : pathlib.Path
        Python source path.

    Returns
    -------
    list[str]
        All diagnostics for the file.
    """
    tree = ast.parse(path.read_text(encoding="utf-8"), filename=str(path))
    errors = _walk_doc_errors(tree, path) + _blank_function_lines(path)
    return errors + _length_errors(tree, path)


def _length_errors(tree: ast.AST, path: Path) -> list[str]:
    """
    Find functions whose executable body exceeds eight lines.

    Parameters
    ----------
    tree : ast.AST
        Parsed Python module.
    path : pathlib.Path
        Source path used in diagnostics.

    Returns
    -------
    list[str]
        Function-length diagnostics.
    """
    kinds = (ast.FunctionDef, ast.AsyncFunctionDef)
    return [
        f"{path}: {node.name}: {len(body)} executable lines"
        for node in ast.walk(tree)
        if isinstance(node, kinds)
        for body in [_executable_body(node)]
        if len(body) > 8
    ]


def _executable_body(node: ast.AST) -> list[ast.AST]:
    """
    Return a function body excluding its docstring.

    Parameters
    ----------
    node : ast.AST
        Function syntax node.

    Returns
    -------
    list[ast.AST]
        Executable body statements.
    """
    has_doc = ast.get_docstring(node) is not None
    return node.body[1:] if has_doc else node.body


def main() -> int:
    """
    Audit all owned Python tooling.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero for a clean audit, otherwise one.
    """
    errors = [
        error for path in _python_files() for error in _file_errors(path)
    ]
    print("\n".join(errors))
    return int(bool(errors))


if __name__ == "__main__":
    raise SystemExit(main())
```

## `scripts/build_images.py`

```python
"""Build the four TELESCREEN partition images from source artifacts."""
import argparse
import gzip
import struct
import subprocess
from pathlib import Path

CONTAINER_MAGIC = 0x001B8421
POLY = 0xEDB88320
DEFAULT_ENV = "bootargs=console=ttyAMA0,115200\0bootdelay=1\0"


def _crc32_byte(crc: int, byte: int) -> int:
    """
    Fold one byte into a reflected CRC-32 residual.

    Parameters
    ----------
    crc : int
        Current checksum.
    byte : int
        Byte to fold in.

    Returns
    -------
    int
        Updated checksum.
    """
    crc ^= byte
    for _ in range(8):
        crc = (crc >> 1) ^ (POLY if crc & 1 else 0)
    return crc & 0xFFFFFFFF


def _crc32_le(seed: int, data: bytes) -> int:
    """
    Compute the reflected crc32_le used by the device.

    Parameters
    ----------
    seed : int
        Initial checksum.
    data : bytes
        Input bytes.

    Returns
    -------
    int
        Computed checksum.
    """
    crc = seed
    for byte in data:
        crc = _crc32_byte(crc, byte)
    return crc


def container(payload: bytes) -> bytes:
    """
    Wrap a kernel payload in the vendor container.

    Parameters
    ----------
    payload : bytes
        Raw kernel image bytes.

    Returns
    -------
    bytes
        Vendor container bytes.
    """
    gz = gzip.compress(payload)
    head = struct.pack("<I", CONTAINER_MAGIC) + struct.pack("<I", len(payload))
    return head + b"gziphead" + gz


def env_blob(text: str) -> bytes:
    """
    Build a U-Boot environment blob with a leading CRC32.

    Parameters
    ----------
    text : str
        NUL-separated key=value text.

    Returns
    -------
    bytes
        Environment blob bytes.
    """
    body = text.encode() + b"\x00"
    crc = _crc32_le(0xFFFFFFFF, body) ^ 0xFFFFFFFF
    return struct.pack("<I", crc) + body


def _write(path: Path, data: bytes) -> None:
    """
    Write bytes to a file and report the size.

    Parameters
    ----------
    path : pathlib.Path
        Output path.
    data : bytes
        Bytes to write.

    Returns
    -------
    None
    """
    path.write_bytes(data)
    print(f"wrote {path} ({len(data)} bytes)")


def _build_rootfs(tree: Path, out: Path) -> None:
    """
    Build a JFFS2 rootfs image from a tree.

    Parameters
    ----------
    tree : pathlib.Path
        Rootfs source tree.
    out : pathlib.Path
        Output image path.

    Returns
    -------
    None
    """
    cmd = ["mkfs.jffs2", "-r", str(tree), "-o", str(out),
           "-e", "0x10000", "-X", "zlib", "-n", "-p"]
    subprocess.run(cmd, check=True)


def _parse() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Parameters
    ----------
    None

    Returns
    -------
    argparse.Namespace
        Parsed arguments.
    """
    ap = argparse.ArgumentParser()
    ap.add_argument("--uboot", required=True)
    ap.add_argument("--kernel", required=True)
    ap.add_argument("--rootfs", required=True)
    ap.add_argument("--out", required=True)
    return ap.parse_args()


def _build_all(args: argparse.Namespace) -> None:
    """
    Build the four partition images.

    Parameters
    ----------
    args : argparse.Namespace
        Parsed arguments.

    Returns
    -------
    None
    """
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    _write(out / "boot.img", Path(args.uboot).read_bytes())
    _write(out / "bootargs.img", env_blob(DEFAULT_ENV))
    _write(out / "kernel.img", container(Path(args.kernel).read_bytes()))
    _build_rootfs(Path(args.rootfs), out / "rootfs.img")


def main() -> int:
    """
    Build the four images into the output directory.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero on success.
    """
    _build_all(_parse())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

## `scripts/carve.py`

```python
"""Carve a whole-flash TELESCREEN image into its four partitions."""
import argparse
from pathlib import Path

PARTS = (
    ("boot", 0x000000, 0x020000),
    ("bootargs", 0x020000, 0x010000),
    ("kernel", 0x030000, 0x1C0000),
    ("rootfs", 0x1F0000, 0xE10000),
)


def _parse() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Parameters
    ----------
    None

    Returns
    -------
    argparse.Namespace
        Parsed arguments.
    """
    ap = argparse.ArgumentParser()
    ap.add_argument("--image", required=True)
    ap.add_argument("--out", required=True)
    return ap.parse_args()


def _carve_one(image: bytes, name: str, off: int, size: int, out: Path) -> None:
    """
    Write one partition slice to disk.

    Parameters
    ----------
    image : bytes
        Whole-flash image bytes.
    name : str
        Partition name.
    off : int
        Partition offset.
    size : int
        Partition size.
    out : pathlib.Path
        Output directory.

    Returns
    -------
    None
    """
    data = image[off:off + size]
    (out / f"{name}.img").write_bytes(data)
    print(f"{name:9} off={off:#08x} size={size:#08x} wrote {len(data)} bytes")


def main() -> int:
    """
    Carve the whole-flash image into the four partitions.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero on success.
    """
    args = _parse()
    image = Path(args.image).read_bytes()
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    for name, off, size in PARTS:
        _carve_one(image, name, off, size, out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

## `scripts/check_coverage.py`

```python
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
    flags += ["-Wall", "-Wextra", "-o", str(out_bin)]
    cmd = [_find_compiler()] + flags + _include_flags() + _sources()
    cmd += _openssl_libs()
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


def _verify_full_coverage(report: str) -> bool:
    """
    Verify report demonstrates 100% line coverage on all owned files.

    Parameters
    ----------
    report : str
        Coverage table text.

    Returns
    -------
    bool
        True when all files have 100.00% line coverage.
    """
    print(report)
    for line in report.splitlines():
        if any(src in line for src in ("aead.c", "partition.c", "crc.c",
                                       "env.c", "jffs2.c", "container.c", "beacon.c", "kex.c", "identity.c", "collector.c", "teled.c", "camera.c")):
            if "100.00%" not in line:
                return False
    return True


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
    sys.exit(main())```

## `scripts/gen_banner.py`

```python
"""Generate the TELESCREEN ASCII banner used by the course documents."""
import argparse

FONT = {
    "T": ["_____", "  |  ", "  |  ", "  |  ", "  |  "],
    "E": ["____ ", "|___ ", "|___ ", "|___ ", "____ "],
    "L": ["|    ", "|    ", "|    ", "|    ", "|___ "],
    "S": ["____ ", "|___ ", "___| ", "|___ ", "____ "],
    "C": [" ____", "/    ", "|    ", "\\____", " ____"],
    "R": ["____ ", "|__/ ", "|  \\ ", "|   \\", "|    "],
    "N": ["|\\  |", "| \\ |", "|  \\|", "|   |", "|   |"],
}


def _render(word: str) -> str:
    """
    Render a word as five ASCII rows.

    Parameters
    ----------
    word : str
        Word to render.

    Returns
    -------
    str
        Rendered banner text.
    """
    rows = [""] * 5
    for ch in word.upper():
        glyph = FONT.get(ch, ["?????"] * 5)
        for i in range(5):
            rows[i] += glyph[i] + " "
    return "\n".join(rows)


def main() -> int:
    """
    Print the banner for the supplied word.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero on success.
    """
    ap = argparse.ArgumentParser()
    ap.add_argument("--word", default="TELESCREEN")
    args = ap.parse_args()
    print(_render(args.word))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

## `scripts/run_tests.py`

```python
#!/usr/bin/env python3
"""Compile and execute the native unit test suite."""
import shutil
import subprocess
import sys
from pathlib import Path


def _find_compiler() -> str:
    """
    Locate host C compiler.

    Parameters
    ----------
    None

    Returns
    -------
    str
        Path or command name for host C compiler.
    """
    return shutil.which("clang") or shutil.which("gcc") or "cc"


def _owned_sources() -> list[str]:
    """
    Return owned module C source files.

    Parameters
    ----------
    None

    Returns
    -------
    list[str]
        Firmware module source path strings.
    """
    return [
        "src/aead.c",
        "src/partition.c",
        "src/crc.c",
        "src/env.c",
        "src/jffs2.c",
        "src/container.c",
        "src/beacon.c",
        "src/kex.c",
        "src/identity.c",
        "src/collector.c",
        "src/teled.c",
        "src/camera.c",
    ]


def _sources() -> list[str]:
    """
    Compose the full native test translation unit list.

    Parameters
    ----------
    None

    Returns
    -------
    list[str]
        Harness and adapter test source paths.
    """
    return ["test/harness/harness.c", "test/test_telescreen.c"]


def _include_flags() -> list[str]:
    """
    Compose compiler include flags.

    Parameters
    ----------
    None

    Returns
    -------
    list[str]
        Include flags for compilation.
    """
    return [
        "-Iinclude",
        "-Itest",
        "-Itest/mock",
        "-Itest/harness",
        "-I/opt/homebrew/opt/openssl@3/include",
    ]


def _openssl_libs() -> list[str]:
    """
    Compose OpenSSL link flags for the AEAD module.

    Parameters
    ----------
    None

    Returns
    -------
    list[str]
        Linker flags for libcrypto.
    """
    return ["-L/opt/homebrew/opt/openssl@3/lib", "-lcrypto"]


def _compile_test_binary(out_bin: Path) -> int:
    """
    Compile test binary with host C compiler.

    Parameters
    ----------
    out_bin : pathlib.Path
        Output executable destination path.

    Returns
    -------
    int
        Compiler return code.
    """
    flags = ["-Wall", "-Wextra", "-O2", "-o", str(out_bin)]
    cmd = [_find_compiler()] + flags + _include_flags() + _sources()
    cmd += _openssl_libs()
    return subprocess.run(cmd).returncode


def _execute_test(out_bin: Path) -> int:
    """
    Execute compiled test binary.

    Parameters
    ----------
    out_bin : pathlib.Path
        Test executable path.

    Returns
    -------
    int
        Test process exit code.
    """
    return subprocess.run([str(out_bin)]).returncode


def main() -> int:
    """
    Build and execute test suite.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Process return code.
    """
    out_dir = Path("build/test")
    out_dir.mkdir(parents=True, exist_ok=True)
    out_bin = out_dir / "test_cold_chain_monitor_and_security"
    rc = _compile_test_binary(out_bin)
    return _execute_test(out_bin) if rc == 0 else rc


if __name__ == "__main__":
    sys.exit(main())```

## `scripts/verify_telescreen.py`

```python
"""Verify a TELESCREEN image hash and four-partition layout."""
import argparse
import hashlib
from pathlib import Path

PARTS = (
    ("boot", 0x000000, 0x020000, b"\x15\x05\x00\xea"),
    ("kernel", 0x030000, 0x1C0000, b"\x21\x84\x1b\x00"),
    ("rootfs", 0x1F0000, 0xE10000, b"\x85\x19\x03\x20"),
)


def _parse() -> argparse.Namespace:
    """
    Parse command-line arguments.

    Parameters
    ----------
    None

    Returns
    -------
    argparse.Namespace
        Parsed arguments.
    """
    ap = argparse.ArgumentParser()
    ap.add_argument("--image", required=True)
    ap.add_argument("--sha256", default="")
    return ap.parse_args()


def _check_hash(data: bytes, expected: str) -> bool:
    """
    Compare the image SHA-256 against an expected value.

    Parameters
    ----------
    data : bytes
        Whole-flash image bytes.
    expected : str
        Expected lowercase hex digest, or empty to skip.

    Returns
    -------
    bool
        True when the hash matches or no hash was supplied.
    """
    got = hashlib.sha256(data).hexdigest()
    print(f"sha256 {got}")
    if not expected:
        return True
    return got == expected.lower()


def _check_magics(data: bytes) -> bool:
    """
    Confirm the three magic-bearing partitions.

    Parameters
    ----------
    data : bytes
        Whole-flash image bytes.

    Returns
    -------
    bool
        True when every magic matches.
    """
    ok = True
    for name, off, size, magic in PARTS:
        chunk = data[off:off + len(magic)]
        good = chunk == magic
        print(f"{name:9} magic {'ok' if good else 'BAD'}")
        ok = ok and good
    return ok


def main() -> int:
    """
    Verify the image hash and partition magics.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero when verification passes.
    """
    args = _parse()
    data = Path(args.image).read_bytes()
    ok = _check_hash(data, args.sha256)
    ok = _check_magics(data) and ok
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
```

## `scripts/weak_decrypt.py`

```python
"""Demonstrate the Ministry weak beacon key derived from the public UID."""
import argparse

POLY = 0xEDB88320


def _crc32_byte(crc: int, byte: int) -> int:
    """
    Fold one byte into a reflected CRC-32 residual.

    Parameters
    ----------
    crc : int
        Current checksum.
    byte : int
        Byte to fold in.

    Returns
    -------
    int
        Updated checksum.
    """
    crc ^= byte
    for _ in range(8):
        crc = (crc >> 1) ^ (POLY if crc & 1 else 0)
    return crc & 0xFFFFFFFF


def _crc32_le(seed: int, data: bytes) -> int:
    """
    Compute the reflected crc32_le used by the device.

    Parameters
    ----------
    seed : int
        Initial checksum.
    data : bytes
        Input bytes.

    Returns
    -------
    int
        Computed checksum.
    """
    crc = seed
    for byte in data:
        crc = _crc32_byte(crc, byte)
    return crc


def weak_key(uid: str) -> bytes:
    """
    Derive the weak 32-byte key from a public UID.

    Parameters
    ----------
    uid : str
        Public device identifier.

    Returns
    -------
    bytes
        Derived key bytes.
    """
    raw = uid.encode()
    seed = _crc32_le(0, raw)
    out = bytearray()
    for _ in range(32):
        seed = _crc32_le(seed, raw)
        out.append(seed & 0xFF)
    return bytes(out)


def main() -> int:
    """
    Print the weak key for a supplied UID.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero on success.
    """
    ap = argparse.ArgumentParser()
    ap.add_argument("--uid", required=True)
    args = ap.parse_args()
    print(weak_key(args.uid).hex())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```
