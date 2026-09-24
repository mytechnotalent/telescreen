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
