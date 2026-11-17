#!/usr/bin/env python3
"""Host tests for the pinned upstream compatibility fix, no PlatformIO needed."""
from pathlib import Path
import tempfile
import unittest
from patch_ble_mouse import PatchError, patch_text, patch_file, patch_project, ENVIRONMENTS, DEPENDENCY_DIR, SOURCE_FILE

UPSTREAM_SAMPLE = """#include <BLEDevice.h>
void sample() {
  BLEDevice::init(bleMouseInstance->deviceName);
  bleMouseInstance->hid->manufacturer()->setValue(bleMouseInstance->deviceManufacturer);
}
"""


class CompatPatchTests(unittest.TestCase):
    def test_exact_two_substitutions(self):
        out, changed = patch_text(UPSTREAM_SAMPLE)
        self.assertTrue(changed)
        self.assertIn("BLEDevice::init(String(bleMouseInstance->deviceName.c_str()));", out)
        self.assertIn("setValue(String(bleMouseInstance->deviceManufacturer.c_str()));", out)
        self.assertNotIn("BLEDevice::init(bleMouseInstance->deviceName);", out)

    def test_idempotent(self):
        once, _ = patch_text(UPSTREAM_SAMPLE)
        twice, changed = patch_text(once)
        self.assertFalse(changed)
        self.assertEqual(once, twice)

    def test_missing_signature_rejected(self):
        with self.assertRaises(PatchError):
            patch_text("// unrelated source")

    def test_duplicated_signature_rejected(self):
        with self.assertRaises(PatchError):
            patch_text(UPSTREAM_SAMPLE + UPSTREAM_SAMPLE)

    def test_conflicting_signature_rejected(self):
        once, _ = patch_text(UPSTREAM_SAMPLE)
        with self.assertRaises(PatchError):
            patch_text(once + UPSTREAM_SAMPLE)

    def test_real_paths_and_no_repeat_changes(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            for env in ENVIRONMENTS:
                path = root / ".pio" / "libdeps" / env / DEPENDENCY_DIR / SOURCE_FILE
                path.parent.mkdir(parents=True)
                path.write_text(UPSTREAM_SAMPLE, encoding="utf-8")
            initial = patch_project(root)
            repeated = patch_project(root)
            self.assertEqual(len(initial), 2)
            self.assertTrue(all(patched for _, patched in initial))
            self.assertFalse(any(patched for _, patched in repeated))

    def test_missing_dependency_causes_failure(self):
        with tempfile.TemporaryDirectory() as td:
            with self.assertRaises(PatchError):
                patch_project(Path(td))

    def test_only_exact_source_is_changed(self):
        with tempfile.TemporaryDirectory() as td:
            path = Path(td) / "BleMouse.cpp"
            path.write_text(UPSTREAM_SAMPLE, encoding="utf-8")
            other = Path(td) / "unrelated.txt"
            other.write_text(UPSTREAM_SAMPLE, encoding="utf-8")
            self.assertTrue(patch_file(path))
            self.assertEqual(other.read_text(encoding="utf-8"), UPSTREAM_SAMPLE)


if __name__ == "__main__":
    unittest.main(verbosity=2)
