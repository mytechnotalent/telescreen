# Walkthrough 14: The Tooling

**every script in `scripts/`, explained**

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

## The Nine Tools

| script | role |
| ------ | ---- |
| `audit_c_standard.py` | C standard: blank lines, 8-line bodies, Doxygen |
| `audit_python_standard.py` | Python standard: PEP8, 8-line bodies, docstrings |
| `run_tests.py` | compile and run the native suite |
| `check_coverage.py` | the 100% line-coverage gate |
| `carve.py` | carve the four partitions |
| `build_images.py` | build the four images |
| `verify_telescreen.py` | verify hash + magics |
| `weak_decrypt.py` | the recovered weak KDF |
| `gen_banner.py` | the ASCII banner |

All nine pass `audit_python_standard.py` (no blank lines in bodies, `<= 8` executable
lines per function, NumPy-style docstrings).

## 1. `carve.py`

Encodes the layout once:

```python
PARTS = (
    ("boot", 0x000000, 0x020000),
    ("bootargs", 0x020000, 0x010000),
    ("kernel", 0x030000, 0x1C0000),
    ("rootfs", 0x1F0000, 0xE10000),
)
```

`main` parses `--image`/`--out`, reads the image, makes the output dir, and calls
`_carve_one` per part:

```python
def _carve_one(image, name, off, size, out):
    data = image[off:off + size]
    (out / f"{name}.img").write_bytes(data)
    print(f"{name:9} off={off:#08x} size={size:#08x} wrote {len(data)} bytes")
```

The slice `image[off:off+size]` is exactly the `dd` line from `docs/04`. The CLI helpers
`_parse()` keep `main` within eight lines.

```bash
python3 scripts/carve.py --image CTF-XX-full.img --out carved/
```

## 2. `verify_telescreen.py`

Checks the SHA-256 (when `--sha256` is given) and the three magic-bearing partitions:

```python
def _check_magics(data):
    ok = True
    for name, off, size, magic in PARTS:
        chunk = data[off:off + len(magic)]
        good = chunk == magic
        print(f"{name:9} magic {'ok' if good else 'BAD'}")
        ok = ok and good
    return ok
```

The `ok = ok and good` accumulates over all partitions so the exit code is correct even if
a later check passes after an earlier failure.

```bash
python3 scripts/verify_telescreen.py --image CTF-XX-full.img --sha256 <hash>
```

## 3. `build_images.py`

The mirror of `carve`: it **constructs** the four images.

```python
def env_blob(text):
    body = text.encode() + b"\x00"
    crc = _crc32_le(0xFFFFFFFF, body) ^ 0xFFFFFFFF
    return struct.pack("<I", crc) + body

def container(payload):
    gz = gzip.compress(payload)
    head = struct.pack("<I", CONTAINER_MAGIC) + struct.pack("<I", len(payload))
    return head + b"gziphead" + gz

def _build_rootfs(tree, out):
    cmd = ["mkfs.jffs2", "-r", str(tree), "-o", str(out),
           "-e", "0x10000", "-X", "zlib", "-n", "-p"]
    subprocess.run(cmd, check=True)
```

`env_blob` applies the U-Boot CRC32 init/final; `container` wraps a kernel in the vendor
container; `_build_rootfs` shells out to `mkfs.jffs2` with the exact erase-block and
compressor the mount expects (`docs/26`).

```bash
python3 scripts/build_images.py --uboot u-boot.bin --kernel Image --rootfs rootfs/ --out out/
```

## 4. `weak_decrypt.py`

Reimplements `beacon_weak_key` (`docs/walkthrough/06`) in Python so you can recover the
key from a public UID:

```python
def weak_key(uid):
    raw = uid.encode()
    seed = _crc32_le(0, raw)
    out = bytearray()
    for _ in range(32):
        seed = _crc32_le(seed, raw)
        out.append(seed & 0xFF)
    return bytes(out)
```

Same reflected CRC, same loop. Running it on the device's UID prints the key that
"protects" the beacon - proving the key is public.

```bash
python3 scripts/weak_decrypt.py --uid SSAT-468547-FEEBD
```

## 5. `run_tests.py`

Compiles the native suite and runs it.

```python
def _find_compiler():
    return shutil.which("clang") or shutil.which("gcc") or "cc"

def _owned_sources():
    return ["src/aead.c", "src/partition.c", "src/crc.c", "src/env.c",
            "src/jffs2.c", "src/container.c", "src/beacon.c", "src/kex.c",
            "src/identity.c", "src/collector.c", "src/teled.c", "src/camera.c"]

def _sources():
    return ["test/harness/harness.c", "test/test_telescreen.c"]
```

`_owned_sources()` is the list the **coverage** gate requires at 100%; `_sources()` are
the files actually compiled (the test file `#include`s each owned `.c`). `_openssl_libs()`
adds `-lcrypto`.

```bash
python3 scripts/run_tests.py        # 98 checks, 0 failures
```

## 6. `check_coverage.py`

The gate. It compiles with `-fprofile-instr-generate -fcoverage-mapping`, runs the test
binary, merges the profile, and reads the report:

```python
def _verify_full_coverage(report):
    print(report)
    for line in report.splitlines():
        if any(src in line for src in ("aead.c", "partition.c", "crc.c",
                                       "env.c", "jffs2.c", "container.c",
                                       "beacon.c", "kex.c", "identity.c",
                                       "collector.c", "teled.c", "camera.c")):
            if "100.00%" not in line:
                return False
    return True
```

Any owned file below `100.00%` line coverage fails the gate. This is what forced the
aggressive factoring in `aead.c` and the removal of the dead clamp in `nonce_reused`.

```bash
python3 scripts/check_coverage.py    # exit 0
```

## 7. `audit_c_standard.py`

A scanner, not a linter. It strips comments and strings from each line, tracks brace
depth, and - for each function body it finds - reports:

- **blank lines inside the body** (forbidden);
- **bodies over eight executable lines**;
- **functions missing a Doxygen `@brief`**.

It found the nine-line `main` and the over-long `hkdf_expand` during development. Run it
and require **no output**:

```bash
python3 scripts/audit_c_standard.py   # (empty) = compliant
```

## 8. `audit_python_standard.py`

The same idea for Python: PEP8 shape, the eight-line rule, no blank lines in bodies, and
NumPy docstrings. It keeps the tooling itself honest.

## 9. `gen_banner.py`

A tiny ASCII banner renderer for the course documents. It maps letters to five-row glyphs
and joins them:

```python
def _render(word):
    rows = [""] * 5
    for ch in word.upper():
        glyph = FONT.get(ch, ["?????"] * 5)
        for i in range(5):
            rows[i] += glyph[i] + " "
    return "\n".join(rows)
```

## The Point: Tooling as Curriculum

Every tool here is a **teachable artifact**, not glue:

- `carve`/`build`/`verify` are the four-partition model in code (`docs/02`, `docs/26`).
- `weak_decrypt` is the weak KDF made explicit (`docs/17`).
- `run_tests`/`check_coverage`/`audit_*` enforce the standards that make the code worth
  reading (`docs/appendix`).

Read the tool, then read the module it exercises.

## Exercises

1. Add `--list` to `carve.py` that prints the four regions without carving.
2. Make `verify_telescreen.py` also verify the `bootargs` CRC.
3. Extend `weak_decrypt.py` to decrypt a captured frame, not just derive the key.
4. Explain what `check_coverage.py` would do if one owned file dropped to 99.9%.

## Reference

- `scripts/*.py`, `docs/appendix/D-scripts-listing.md`
