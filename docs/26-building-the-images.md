# Volume 26: Building the Images

**mkfs.jffs2, the container, and the four-region layout**

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

## What You'll Learn

- How to build all four partition images from source artifacts
- The container rebuild and the JFFS2 build
- The exact tooling (`scripts/build_images.py`)
- How to reproduce the CTF artifacts

## 1. Inputs

| input | produces |
| ----- | -------- |
| `u-boot.bin` | `boot.img` |
| env text | `bootargs.img` |
| kernel `Image` | `kernel.img` |
| rootfs tree | `rootfs.img` |

## 2. The Builder

```bash
python3 scripts/build_images.py \
    --uboot u-boot.bin --kernel Image --rootfs rootfs/ --out out/
```

`scripts/build_images.py` encodes the layout once:

```python
def _build_all(args):
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    _write(out / "boot.img", Path(args.uboot).read_bytes())
    _write(out / "bootargs.img", env_blob(DEFAULT_ENV))
    _write(out / "kernel.img", container(Path(args.kernel).read_bytes()))
    _build_rootfs(Path(args.rootfs), out / "rootfs.img")
```

## 3. The Environment

```python
def env_blob(text: str) -> bytes:
    body = text.encode() + b"\x00"
    crc = _crc32_le(0xFFFFFFFF, body) ^ 0xFFFFFFFF
    return struct.pack("<I", crc) + body
```

The `crc32_uboot` init/final (`docs/07`) is applied here, so the blob validates.

## 4. The Container

```python
def container(payload: bytes) -> bytes:
    gz = gzip.compress(payload)
    head = struct.pack("<I", CONTAINER_MAGIC) + struct.pack("<I", len(payload))
    return head + b"gziphead" + gz
```

The length field is the **uncompressed** length (`docs/08`).

## 5. The Rootfs

```python
def _build_rootfs(tree: Path, out: Path) -> None:
    cmd = ["mkfs.jffs2", "-r", str(tree), "-o", str(out),
           "-e", "0x10000", "-X", "zlib", "-n", "-p"]
    subprocess.run(cmd, check=True)
```

- `-e 0x10000` : 64 KiB erase blocks (matches the layout).
- `-X zlib` : the compression the mount expects.
- `-n` : no cleanmarker (or `-p` to pad to size).

## 6. Assemble the Whole Image

Place each image at its locked offset (`docs/02`):

```
boot     0x000000
bootargs 0x020000
kernel   0x030000
rootfs   0x1F0000
```

Then verify:

```bash
python3 scripts/verify_telescreen.py --image out/full.img
```

## 7. Reproduce the CTF Artifacts

The CTF artifacts (`CTF_telescreen/CTF-XX-*.img`) were built exactly this way, then
carved and hashed. Reproducing them proves you understand the layout.

## Labs

1. Build all four images from a small tree and a dummy kernel.
2. Concatenate them at the locked offsets into `full.img`.
3. Run the verifier and confirm all three magics pass.
4. Rebuild the container and confirm `container_valid` accepts it.

## Reference

- `scripts/build_images.py`, `scripts/verify_telescreen.py`
- `docs/02` (layout), `docs/08` (container), `docs/27` (flash and verify)
