#!/usr/bin/env python3
"""Round-trip test: build the partitions, assemble, verify, carve, compare.

Proves the four-partition pipeline is self-consistent without any hardware:

    build (boot/bootargs/kernel)  ->  assemble full image
        ->  verify_telescreen.py (magics)  ->  carve.py  ->  byte-compare

Uses ``mkfs.jffs2`` for a real JFFS2 rootfs when it is on PATH (Linux), and a
minimal JFFS2 cleanmarker stub otherwise (macOS/CI), so the test runs anywhere.
"""
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import assemble_image          # noqa: E402
import build_images            # noqa: E402

JFFS2_MAGIC = b"\x85\x19\x03\x20"        # LE magic 0x1985 + cleanmarker 0x2003


def make_rootfs(tree: Path) -> bytes:
    """
    Produce a rootfs image (real JFFS2 if mkfs.jffs2 exists, else a stub).

    Parameters
    ----------
    tree : pathlib.Path
        Rootfs source tree.

    Returns
    -------
    bytes
        Rootfs image bytes.
    """
    out = tree.parent / "rootfs.img"
    if shutil.which("mkfs.jffs2"):
        build_images._build_rootfs(tree, out)
        return out.read_bytes()
    return JFFS2_MAGIC + b"\x00" * 4096


def run(cmd: list) -> str:
    """
    Run a command in the repository root and return its combined output.

    Parameters
    ----------
    cmd : list
        Command and arguments.

    Returns
    -------
    str
        Combined stdout and stderr.
    """
    res = subprocess.run(cmd, capture_output=True, text=True, cwd=HERE.parent)
    return res.stdout + res.stderr


def inputs(tree: Path) -> dict:
    """
    Build the four in-memory partition images from synthetic sources.

    Parameters
    ----------
    tree : pathlib.Path
        Rootfs source tree.

    Returns
    -------
    dict
        Mapping of partition name to image bytes.
    """
    return {
        "boot": b"\x15\x05\x00\xea" + b"\x00" * 0x1000,
        "bootargs": build_images.env_blob(build_images.DEFAULT_ENV),
        "kernel": build_images.container(b"TELESCREEN KERNEL " * 64),
        "rootfs": make_rootfs(tree),
    }


def write_inputs(tmp: Path, built: dict) -> None:
    """
    Write the in-memory partition images to disk.

    Parameters
    ----------
    tmp : pathlib.Path
        Scratch directory.
    built : dict
        Partition name to image bytes.

    Returns
    -------
    None
    """
    for name, data in built.items():
        (tmp / f"{name}.img").write_bytes(data)


def full_image(tmp: Path, built: dict) -> Path:
    """
    Assemble the whole-flash image from the partition images.

    Parameters
    ----------
    tmp : pathlib.Path
        Scratch directory.
    built : dict
        Partition name to image bytes.

    Returns
    -------
    pathlib.Path
        Path to the assembled full image.
    """
    full = tmp / "full.img"
    data = assemble_image.assemble(built["boot"], built["bootargs"],
                                   built["kernel"], built["rootfs"])
    full.write_bytes(data)
    return full


def check_verify(full: Path) -> list:
    """
    Run verify_telescreen and return its checks.

    Parameters
    ----------
    full : pathlib.Path
        Whole-flash image path.

    Returns
    -------
    list
        (name, bool) check tuples.
    """
    out = run([sys.executable, "scripts/verify_telescreen.py",
               "--image", str(full)])
    return [("verify: magics ok", out.count("magic ok") == 3)]


def check_carve(full: Path, tmp: Path) -> tuple:
    """
    Run carve.py and return its checks plus the output directory.

    Parameters
    ----------
    full : pathlib.Path
        Whole-flash image path.
    tmp : pathlib.Path
        Scratch directory.

    Returns
    -------
    tuple
        (checks list, carved directory path).
    """
    carved = tmp / "carved"
    out = run([sys.executable, "scripts/carve.py",
               "--image", str(full), "--out", str(carved)])
    ok = out.count("wrote") == 4 and len(list(carved.glob("*.img"))) == 4
    return [("carve: wrote 4 parts", ok)], carved


def compare(carved: Path, built: dict) -> list:
    """
    Byte-compare each carved region against the built image.

    Parameters
    ----------
    carved : pathlib.Path
        Carved partition directory.
    built : dict
        Partition name to image bytes.

    Returns
    -------
    list
        (name, bool) check tuples.
    """
    checks = []
    for name, _off, _size in assemble_image.PARTS:
        region = (carved / f"{name}.img").read_bytes()
        checks.append((f"round-trip: {name} bytes match",
                       region[:len(built[name])] == built[name]))
    return checks


def report(checks: list) -> int:
    """
    Print per-check results and the summary.

    Parameters
    ----------
    checks : list
        (name, bool) check tuples.

    Returns
    -------
    int
        Zero when every check passes.
    """
    for name, good in checks:
        print(("PASS " if good else "FAIL ") + name)
    passed = sum(1 for _, good in checks if good)
    print(f"{passed}/{len(checks)} checks, {len(checks) - passed} failures")
    return 0 if passed == len(checks) else 1


def run_all(tmp: Path, built: dict) -> int:
    """
    Assemble, verify, carve and compare, returning the exit code.

    Parameters
    ----------
    tmp : pathlib.Path
        Scratch directory.
    built : dict
        Partition name to image bytes.

    Returns
    -------
    int
        Zero when every check passes.
    """
    full = full_image(tmp, built)
    carve_checks, carved = check_carve(full, tmp)
    return report(check_verify(full) + carve_checks + compare(carved, built))


def main() -> int:
    """
    Set up synthetic inputs and run the round-trip.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero when every stage passes.
    """
    tmp = Path(tempfile.mkdtemp(prefix="telescreen-rt-"))
    tree = tmp / "rootfs-src"
    tree.mkdir()
    (tree / "hello.txt").write_text("operation telescreen\n")
    built = inputs(tree)
    write_inputs(tmp, built)
    return run_all(tmp, built)


if __name__ == "__main__":
    raise SystemExit(main())
