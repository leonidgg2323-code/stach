"""Offline tests for Stage-12 artifact packager (fake images stay temporary)."""
import importlib.util
import json
import tempfile
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parent.parent
SPEC = importlib.util.spec_from_file_location('stach_pack_firmware', ROOT / 'tools/package_firmware.py')
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class ReleaseTests(unittest.TestCase):
    def create_fake_builds(self, root: Path):
        for profile in MODULE.PROFILES:
            folder = root / '.pio' / 'build' / profile
            folder.mkdir(parents=True)
            for name in MODULE.REQUIRED_IMAGES:
                (folder / name).write_bytes(('TESTONLY:' + profile + ':' + name).encode())

    def test_all_three_profiles_and_hashes(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            self.create_fake_builds(root)
            manifest = MODULE.package(root, root / 'dist', 'abc123')
            self.assertEqual(set(manifest['profiles']), set(MODULE.PROFILES))
            self.assertEqual(manifest['source_commit'], 'abc123')
            self.assertFalse(manifest['hardware_validated'])
            self.assertEqual(manifest['release'], '12.0.0-rc2')
            checksum_lines = (root / 'dist/SHA256SUMS.txt').read_text().splitlines()
            self.assertEqual(len(checksum_lines), 9)
            for profile, images in manifest['profiles'].items():
                for name, item in images.items():
                    path = root / 'dist' / profile / name
                    self.assertEqual(item['sha256'], MODULE.sha256(path))
            loaded = json.loads((root / 'dist/MANIFEST.json').read_text())
            self.assertEqual(loaded['profiles'], manifest['profiles'])

    def test_missing_image_fails_without_creating_release(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            self.create_fake_builds(root)
            (root / '.pio/build/stach-waveshare-co5300/firmware.bin').unlink()
            with self.assertRaises(FileNotFoundError):
                MODULE.package(root, root / 'dist')
            self.assertFalse((root / 'dist').exists())

    def test_empty_image_fails(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            self.create_fake_builds(root)
            (root / '.pio/build/stach-waveshare-sh8601/bootloader.bin').write_bytes(b'')
            with self.assertRaises(FileNotFoundError):
                MODULE.package(root, root / 'dist')

    def test_no_143c_profile(self):
        self.assertFalse(any('143c' in x or '1.43c' in x for x in MODULE.PROFILES))

    def test_config_flash_sizing(self):
        ini = (ROOT / 'platformio.ini').read_text()
        self.assertIn('platform = espressif32@8.0.0', ini)
        for tag in ('[env:stach-waveshare-sh8601]', '[env:stach-waveshare-co5300]'):
            section = ini.split(tag, 1)[1].split('\n[env:', 1)[0]
            self.assertIn('board_upload.flash_size = 16MB', section)
            self.assertNotIn('board_upload.maximum_size = 16777216', section)  # Keep app partition size check
            self.assertIn('qio_opi', section)


if __name__ == '__main__':
    unittest.main()
