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
