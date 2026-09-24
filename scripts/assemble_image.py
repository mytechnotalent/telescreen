"""Assemble a whole-flash TELESCREEN image from the four partition files.

The layout is locked; this is the inverse of ``carve.py``.
"""
import argparse
from pathlib import Path

# (name, offset, size) - the four locked regions.
PARTS = (
    ("boot", 0x000000, 0x020000),
    ("bootargs", 0x020000, 0x010000),
    ("kernel", 0x030000, 0x1C0000),
    ("rootfs", 0x1F0000, 0xE10000),
)
TOTAL = 0x1F0000 + 0xE10000


def assemble(boot: bytes, bootargs: bytes, kernel: bytes, rootfs: bytes) -> bytes:
    """
    Place the four partition images into a whole-flash image.

    Parameters
    ----------
    boot : bytes
        Boot partition bytes (<= 128 KiB).
    bootargs : bytes
        Environment partition bytes (<= 64 KiB).
    kernel : bytes
        Kernel container bytes (<= 1792 KiB).
    rootfs : bytes
        JFFS2 rootfs bytes (<= 14400 KiB).

    Returns
    -------
    bytes
        The 16 MiB whole-flash image.

    Raises
    ------
    ValueError
        If any partition is larger than its region.
    """
    buf = bytearray(TOTAL)
    for (name, off, size), data in zip(PARTS, (boot, bootargs, kernel, rootfs)):
        if len(data) > size:
            raise ValueError(f"{name} is {len(data)} bytes; region is {size}")
        buf[off:off + len(data)] = data
    return bytes(buf)


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
    ap.add_argument("--boot", required=True)
    ap.add_argument("--bootargs", required=True)
    ap.add_argument("--kernel", required=True)
    ap.add_argument("--rootfs", required=True)
    ap.add_argument("--out", required=True)
    return ap.parse_args()


def main() -> int:
    """
    Assemble the whole-flash image from the four partition files.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Zero on success.
    """
    a = _parse()
    data = assemble(Path(a.boot).read_bytes(), Path(a.bootargs).read_bytes(),
                    Path(a.kernel).read_bytes(), Path(a.rootfs).read_bytes())
    Path(a.out).write_bytes(data)
    print(f"wrote {a.out} ({len(data)} bytes, {len(data) // 1024} KiB)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
