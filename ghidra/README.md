# ghidra/ - the reverse-engineering workspace

Everything you need to open `../firmware/teled.stripped` in Ghidra and give every
function its name back.

## Contents

| item | what it is |
| ---- | ---------- |
| `make_project.sh` | headless: import + full auto-analysis + save |
| `decompile.sh` | export one `.c` file per function |
| `ExportDecomp.java` | the Ghidra script `decompile.sh` runs |
| `proj/TelescreenRE.gpr` | the analysed Ghidra project (open this in the GUI) |
| `decomp/` | raw decompilation, `FUN_00401e00`-style names |
| `resolved/` | the same files renamed to their real functions |
| `ground_truth/` | `nm`, `readelf`, `objdump` evidence (instructor) |
| `RESOLUTION_MAP.md` | every function -> its real name + the rule that proves it |
| `resolution.json` | the same data, machine-readable |
| `resolve_functions.py` | generates `RESOLUTION_MAP.md`, `resolution.json`, `resolved/` |
| `gen_appendix_j.py` | generates `../docs/appendix/J-ghidra-function-resolution.md` |

## Quick start

```bash
../firmware/build_target.sh     # build the target first
./make_project.sh               # analyse it (headless)
./decompile.sh                  # export every function to C (optional)

# open the GUI and load proj/TelescreenRE.gpr, then:
#   G 0x401000 ; F   -> _start
#   G 0x401d80 ; F   -> crc32_le
#   G 0x401ae0 ; F   -> beacon_weak_key (the defect)
```

## Regenerate the reports

```bash
python3 resolve_functions.py    # -> RESOLUTION_MAP.md + resolution.json + resolved/
python3 gen_appendix_j.py       # -> ../docs/appendix/J-ghidra-function-resolution.md
```

## The four resolution rules

- **R1** exact address match against the unstripped twin (`../firmware/teled.unstripped`)
- **R2** `.plt` stub -> `JUMP_SLOT` relocation -> import name
- **R3** the `.plt` PLT0 lazy resolver at `0x400d20`
- **R4** phantom/overlapping function on alignment padding

Read `RESOLUTION_MAP.md` after you have tried each function yourself. See
`../docs/33-ghidra-nation-state-re.md` for the full workflow.
