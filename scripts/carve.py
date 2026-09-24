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
