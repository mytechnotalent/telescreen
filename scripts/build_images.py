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
