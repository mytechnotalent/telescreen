# firmware/ - the TELESCREEN target and its answer key

This folder contains the program you reverse in this course, plus the key the
instructor uses to prove your answers.

| file | what it is | who uses it |
| ---- | ---------- | ----------- |
| `build_target.sh` | builds both files below from `../src` | everyone |
| `teled.stripped` | the **stripped** ARM64 target - no function names | students |
| `teled.unstripped` | the same code **with** names - the answer key | instructors |

## Build it

```bash
./firmware/build_target.sh
```

The build runs inside a pinned `linux/arm64` Docker container, so the output is
byte-for-byte identical on Windows x64, Linux x64, and macOS arm64 (Apple
Silicon). On Apple Silicon the container runs natively; on x64 hosts Docker
emulates the same ARM64 image, which is slower but produces the same bytes.

## Why two files?

Stripping removes the **symbol table** (the map from names like `crc32_le` to
addresses like `0x401d80`). The machine code is untouched. Addresses are therefore
identical in both files, which is what lets the instructor resolve every
`FUN_00xxxxxx` in the stripped file exactly:

```
firmware/teled.stripped     -> 0x401d80  FUN_00401e00
firmware/teled.unstripped   -> 0x401d80  crc32_le
```

## Prove you have the right bytes

`build_target.sh` prints the SHA-256 of each file. Compare against the values in
`../ghidra/RESOLUTION_MAP.md`. If they match, every address in the course is true
for you.

## Expected facts

- `file teled.stripped` → `ELF 64-bit LSB executable, ARM aarch64, ... stripped`
- entry point → `0x401000`
- application functions → **42**
- total functions Ghidra finds → **127**
