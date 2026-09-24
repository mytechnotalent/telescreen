# Walkthrough 22: Build System and CI

**CMake, the standard gates, and the continuous-integration workflow**

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

## Two Builds

The lab has **two** build paths:

1. the **native test/coverage build** (host C compiler, no RP5 needed);
2. the **RP5 target build** (CMake + OpenSSL, application-class AArch64).

## 1. The Native Build

`scripts/run_tests.py` compiles `test/harness/harness.c` + `test/test_telescreen.c`
(the latter `#include`s every owned `.c`) for the **host**, links libcrypto, and runs:

```python
def _compile_test_binary(out_bin):
    flags = ["-Wall", "-Wextra", "-O2", "-o", str(out_bin)]
    cmd = [_find_compiler()] + flags + _include_flags() + _sources()
    cmd += _openssl_libs()
    return subprocess.run(cmd).returncode
```

`_include_flags()` includes `include/`, `test/`, `test/mock/`, `test/harness/`, and the
OpenSSL headers. The host can be Linux, macOS, or anything with a C compiler.

## 2. The CMake Target Build

`CMakeLists.txt` is the product build:

```cmake
cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
project(telescreen C)

add_compile_options(-Wall -Wextra -Wshadow -Wformat=2
                    -fstack-protector-strong -D_FORTIFY_SOURCE=2 -O2)
find_package(OpenSSL REQUIRED)
add_executable(telescreen src/main.c src/aead.c src/partition.c ...)
target_link_libraries(telescreen OpenSSL::Crypto)
```

The compile options are **part of the lesson**: `-fstack-protector-strong` and
`-D_FORTIFY_SOURCE=2` are exactly what the TELESCREEN ships **without** (the CTF's bugs),
so building the lab with them shows the defence.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## 3. The Three Gates

Every change must pass all three:

| gate | command | pass condition |
| ---- | ------- | -------------- |
| C standard | `python3 scripts/audit_c_standard.py` | **no output** |
| Python standard | `python3 scripts/audit_python_standard.py` | **exit 0** |
| tests + coverage | `python3 scripts/run_tests.py` then `check_coverage.py` | 0 failures, **100.00%** |

### Why a standard gate at all

Formatting is not the point. The point is that **a body over eight lines is a body doing
too much**, and a function without a `@brief` is a function nobody understands. The gate
forces decomposition - which is why `aead.c` is readable and testable.

### Why 100% coverage

A single uncovered line is a line no test exercises - and in a security module, an
untested branch is a **potential vulnerability**. The gate turned two dead lines into
fixes during development (`docs/walkthrough/11`, `docs/walkthrough/12`).

## 4. Continuous Integration

`.github/workflows/ci.yml` runs all gates on every push and pull request:

```yaml
jobs:
  standards-and-tests:
    runs-on: macos-14
    steps:
      - uses: actions/checkout@v4
      - run: brew install openssl@3
      - run: python3 scripts/audit_c_standard.py
      - run: python3 scripts/audit_python_standard.py
      - run: python3 scripts/run_tests.py
      - run: python3 scripts/check_coverage.py
```

A red check means a regression. Nothing merges green-less.

## 5. The Toolchain Files

| file | role |
| ---- | ---- |
| `.clang-format` | the formatter config |
| `.clangd` | the language-server config |
| `.gitignore` | ignores `build/`, `*.profraw`, `*.profdata`, `__pycache__/` |

## 6. Reproduce Everything

```bash
# 1. standards
python3 scripts/audit_c_standard.py
python3 scripts/audit_python_standard.py

# 2. tests + coverage
python3 scripts/run_tests.py
python3 scripts/check_coverage.py

# 3. the RP5 target
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build

# 4. the images
python3 scripts/build_images.py --uboot u-boot.bin --kernel Image --rootfs rootfs/ --out out/
python3 scripts/verify_telescreen.py --image out/full.img
```

## 7. Why This Is Curriculum

A security lab that ships sloppy code teaches nothing. The build system, the three gates,
and the CI are a **worked example** of how to keep a security-sensitive codebase honest:
one standard, one suite, one coverage rule, enforced automatically.

## Exercises

1. Run all three gates and record the output.
2. Add `-Wconversion` and fix the warnings it reveals.
3. Add a fourth CI step that builds the RP5 target.
4. Explain why `-D_FORTIFY_SOURCE=2` is a mitigation the CTF device lacks.

## Reference

- `CMakeLists.txt`, `scripts/*.py`, `.github/workflows/ci.yml`
- `docs/walkthrough/14` (tooling), `docs/walkthrough/15` (tests)
