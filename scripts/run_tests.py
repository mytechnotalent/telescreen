#!/usr/bin/env python3
"""Compile and execute the native unit test suite."""
import shutil
import subprocess
import sys
from pathlib import Path


def _find_compiler() -> str:
    """
    Locate host C compiler.

    Parameters
    ----------
    None

    Returns
    -------
    str
        Path or command name for host C compiler.
    """
    return shutil.which("clang") or shutil.which("gcc") or "cc"


def _owned_sources() -> list[str]:
    """
    Return owned module C source files.

    Parameters
    ----------
    None

    Returns
    -------
    list[str]
        Firmware module source path strings.
    """
    return [
        "src/aead.c",
        "src/partition.c",
        "src/crc.c",
        "src/env.c",
        "src/jffs2.c",
        "src/container.c",
        "src/beacon.c",
        "src/kex.c",
        "src/identity.c",
        "src/collector.c",
        "src/teled.c",
        "src/camera.c",
    ]


def _sources() -> list[str]:
    """
    Compose the full native test translation unit list.

    Parameters
    ----------
    None

    Returns
    -------
    list[str]
        Harness and adapter test source paths.
    """
    return ["test/harness/harness.c", "test/faults.c", "test/test_telescreen.c"]


def _include_flags() -> list[str]:
    """
    Compose compiler include flags.

    Parameters
    ----------
    None

    Returns
    -------
    list[str]
        Include flags for compilation.
    """
    return [
        "-Iinclude",
        "-Itest",
        "-Itest/mock",
        "-Itest/harness",
        "-I/opt/homebrew/opt/openssl@3/include",
    ]


def _openssl_libs() -> list[str]:
    """
    Compose OpenSSL link flags for the AEAD module.

    Parameters
    ----------
    None

    Returns
    -------
    list[str]
        Linker flags for libcrypto.
    """
    return ["-L/opt/homebrew/opt/openssl@3/lib", "-lcrypto"]


def _fault_flags() -> list[str]:
    """
    Compose macro flags that rename OpenSSL calls to fault-injection wrappers.

    Parameters
    ----------
    None

    Returns
    -------
    list[str]
        Preprocessor flags mapping each OpenSSL call to its fault_* wrapper.
    """
    syms = [
        "EVP_CIPHER_CTX_new", "EVP_MD_CTX_new",
        "EVP_EncryptInit_ex", "EVP_EncryptUpdate", "EVP_EncryptFinal_ex",
        "EVP_DecryptInit_ex", "EVP_DecryptUpdate", "EVP_DecryptFinal_ex",
        "EVP_CIPHER_CTX_ctrl", "HMAC", "EVP_PKEY_get_raw_public_key",
        "EVP_DigestSignInit", "EVP_DigestSign",
        "EVP_DigestVerifyInit", "EVP_DigestVerify",
        "EVP_PKEY_derive_init", "EVP_PKEY_derive_set_peer", "EVP_PKEY_derive",
    ]
    return [f"-D{s}=fault_{s}" for s in syms]


def _compile_test_binary(out_bin: Path) -> int:
    """
    Compile test binary with host C compiler.

    Parameters
    ----------
    out_bin : pathlib.Path
        Output executable destination path.

    Returns
    -------
    int
        Compiler return code.
    """
    flags = ["-Wall", "-Wextra", "-O2", "-fno-builtin", "-o", str(out_bin)]
    cmd = [_find_compiler()] + flags + _fault_flags() + _include_flags()
    cmd += _sources() + _openssl_libs()
    return subprocess.run(cmd).returncode


def _execute_test(out_bin: Path) -> int:
    """
    Execute compiled test binary.

    Parameters
    ----------
    out_bin : pathlib.Path
        Test executable path.

    Returns
    -------
    int
        Test process exit code.
    """
    return subprocess.run([str(out_bin)]).returncode


def main() -> int:
    """
    Build and execute test suite.

    Parameters
    ----------
    None

    Returns
    -------
    int
        Process return code.
    """
    out_dir = Path("build/test")
    out_dir.mkdir(parents=True, exist_ok=True)
    out_bin = out_dir / "test_cold_chain_monitor_and_security"
    rc = _compile_test_binary(out_bin)
    return _execute_test(out_bin) if rc == 0 else rc


if __name__ == "__main__":
    sys.exit(main())