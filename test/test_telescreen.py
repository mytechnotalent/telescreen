"""Unit test adapter for VS Code Test Explorer."""
import subprocess
import sys
import unittest

_CACHED_OUTPUT = ""


def _get_harness_output() -> str:
    """
    Execute native tests and return stdout.

    Parameters
    ----------
    None

    Returns
    -------
    str
        Standard output from native test suite.
    """
    global _CACHED_OUTPUT
    if not _CACHED_OUTPUT:
        cmd = [sys.executable, "scripts/run_tests.py"]
        res = subprocess.run(cmd, capture_output=True, text=True)
        _CACHED_OUTPUT = res.stdout
    return _CACHED_OUTPUT


def _assert_harness_pass(test_name: str) -> None:
    """
    Assert that a named harness test passed.

    Parameters
    ----------
    test_name : str
        Name of harness test function.

    Returns
    -------
    None
    """
    output = _get_harness_output()
    expected = f":{test_name}:PASS"
    assert expected in output, f"{test_name} did not pass in harness output"


class TestTelescreenFirmware(unittest.TestCase):
    """Test cases for the TELESCREEN RP5 lab modules."""

    def test_01_aead_nonce_size_before_init(self) -> None:
        """
        Verify the aead_nonce_size_before_init native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_nonce_size_before_init")

    def test_02_aead_init_rejects_bad_algo(self) -> None:
        """
        Verify the aead_init_rejects_bad_algo native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_init_rejects_bad_algo")

    def test_03_aead_init_aes(self) -> None:
        """
        Verify the aead_init_aes native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_init_aes")

    def test_04_aead_seal_bad_args(self) -> None:
        """
        Verify the aead_seal_bad_args native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_seal_bad_args")

    def test_05_aead_open_bad_args(self) -> None:
        """
        Verify the aead_open_bad_args native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_open_bad_args")

    def test_06_aead_seal_open_aes(self) -> None:
        """
        Verify the aead_seal_open_aes native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_seal_open_aes")

    def test_07_aead_open_aes_bad_tag(self) -> None:
        """
        Verify the aead_open_aes_bad_tag native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_open_aes_bad_tag")

    def test_08_aead_nonce_reuse(self) -> None:
        """
        Verify the aead_nonce_reuse native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_nonce_reuse")

    def test_09_aead_init_xchacha(self) -> None:
        """
        Verify the aead_init_xchacha native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_init_xchacha")

    def test_10_aead_seal_open_xchacha(self) -> None:
        """
        Verify the aead_seal_open_xchacha native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_seal_open_xchacha")

    def test_11_aead_open_xchacha_bad_tag(self) -> None:
        """
        Verify the aead_open_xchacha_bad_tag native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_aead_open_xchacha_bad_tag")

    def test_12_crc32_uboot_check(self) -> None:
        """
        Verify the crc32_uboot_check native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_crc32_uboot_check")

    def test_13_crc32_le_empty(self) -> None:
        """
        Verify the crc32_le_empty native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_crc32_le_empty")

    def test_14_crc16_check(self) -> None:
        """
        Verify the crc16_check native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_crc16_check")

    def test_15_env_crc_valid(self) -> None:
        """
        Verify the env_crc_valid native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_env_crc_valid")

    def test_16_env_crc_invalid(self) -> None:
        """
        Verify the env_crc_invalid native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_env_crc_invalid")

    def test_17_env_short(self) -> None:
        """
        Verify the env_short native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_env_short")

    def test_18_jffs2_header(self) -> None:
        """
        Verify the jffs2_header native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_jffs2_header")

    def test_19_jffs2_crc(self) -> None:
        """
        Verify the jffs2_crc native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_jffs2_crc")

    def test_20_container_fields(self) -> None:
        """
        Verify the container_fields native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_container_fields")

    def test_21_container_invalid(self) -> None:
        """
        Verify the container_invalid native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_container_invalid")

    def test_22_beacon_weak_key(self) -> None:
        """
        Verify the beacon_weak_key native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_beacon_weak_key")

    def test_23_beacon_weak_key_differs(self) -> None:
        """
        Verify the beacon_weak_key_differs native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_beacon_weak_key_differs")

    def test_24_beacon_nonce(self) -> None:
        """
        Verify the beacon_nonce native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_beacon_nonce")

    def test_25_beacon_seal(self) -> None:
        """
        Verify the beacon_seal native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_beacon_seal")

    def test_26_kex_x25519_rfc7748(self) -> None:
        """
        Verify the kex_x25519_rfc7748 native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_kex_x25519_rfc7748")

    def test_27_kex_hkdf_rfc5869(self) -> None:
        """
        Verify the kex_hkdf_rfc5869 native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_kex_hkdf_rfc5869")

    def test_28_identity_rfc8032(self) -> None:
        """
        Verify the identity_rfc8032 native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_identity_rfc8032")

    def test_29_identity_verify_bad(self) -> None:
        """
        Verify the identity_verify_bad native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_identity_verify_bad")

    def test_30_collector_accept(self) -> None:
        """
        Verify the collector_accept native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_collector_accept")

    def test_31_collector_reject(self) -> None:
        """
        Verify the collector_reject native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_collector_reject")

    def test_32_teled_beacon(self) -> None:
        """
        Verify the teled_beacon native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_teled_beacon")

    def test_33_teled_beacon_fail(self) -> None:
        """
        Verify the teled_beacon_fail native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_teled_beacon_fail")

    def test_34_part_desc_for_boot(self) -> None:
        """
        Verify the part_desc_for_boot native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_desc_for_boot")

    def test_35_part_desc_for_unknown(self) -> None:
        """
        Verify the part_desc_for_unknown native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_desc_for_unknown")

    def test_36_part_identify_boot(self) -> None:
        """
        Verify the part_identify_boot native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_identify_boot")

    def test_37_part_identify_kernel(self) -> None:
        """
        Verify the part_identify_kernel native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_identify_kernel")

    def test_38_part_identify_rootfs(self) -> None:
        """
        Verify the part_identify_rootfs native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_identify_rootfs")

    def test_39_part_identify_env(self) -> None:
        """
        Verify the part_identify_env native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_identify_env")

    def test_40_part_identify_unknown(self) -> None:
        """
        Verify the part_identify_unknown native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_identify_unknown")

    def test_41_part_verify_layout(self) -> None:
        """
        Verify the part_verify_layout native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_verify_layout")

    def test_42_part_carve_ok(self) -> None:
        """
        Verify the part_carve_ok native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_carve_ok")

    def test_43_part_carve_bad_args(self) -> None:
        """
        Verify the part_carve_bad_args native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_carve_bad_args")

    def test_44_part_carve_too_small(self) -> None:
        """
        Verify the part_carve_too_small native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_carve_too_small")

    def test_45_part_carve_unknown_kind(self) -> None:
        """
        Verify the part_carve_unknown_kind native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_part_carve_unknown_kind")

    def test_46_camera_rtsp_url(self) -> None:
        """
        Verify the camera_rtsp_url native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_camera_rtsp_url")

    def test_47_camera_mjpeg_url(self) -> None:
        """
        Verify the camera_mjpeg_url native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_camera_mjpeg_url")

    def test_48_camera_url_bad(self) -> None:
        """
        Verify the camera_url_bad native case.

        Parameters
        ----------
        None

        Returns
        -------
        None
        """
        _assert_harness_pass("test_camera_url_bad")
