#!/usr/bin/env python3
"""Narrow, idempotent Arduino-ESP32 3.x compatibility patch for BLE Mouse 0.3.1.

Applies ONLY to PlatformIO's generated .pio/libdeps copies, never to Stach's
source or an upstream repo. Refuses unknown upstream library versions/layouts.
"""
from __future__ import annotations

import argparse
from pathlib import Path

ENVIRONMENTS = ("stach-waveshare-sh8601", "stach-waveshare-co5300")
DEPENDENCY_DIR = "ESP32 BLE Mouse"
SOURCE_FILE = "BleMouse.cpp"
REPLACEMENTS = (
    (
        "BLEDevice::init(bleMouseInstance->deviceName);",
        "BLEDevice::init(String(bleMouseInstance->deviceName.c_str()));",
    ),
    (
        "bleMouseInstance->hid->manufacturer()->setValue(bleMouseInstance->deviceManufacturer);",
        "bleMouseInstance->hid->manufacturer()->setValue(String(bleMouseInstance->deviceManufacturer.c_str()));",
    ),
)


class PatchError(RuntimeError):
    pass


def patch_text(source: str) -> tuple[str, bool]:
    """Patch only the two known signature mismatches; accept already-patched files."""
    result = source
    changed = False
    for old, new in REPLACEMENTS:
        old_count = result.count(old)
        new_count = result.count(new)
        if old_count == 1 and new_count == 0:
            result = result.replace(old, new)
            changed = True
        elif old_count == 0 and new_count == 1:
            pass
        else:
            raise PatchError(
                "Unexpected ESP32 BLE Mouse 0.3.1 source; refusing blind patch "
                f"(old occurrences={old_count}, patched occurrences={new_count})."
            )
    return result, changed


def patch_file(path: Path) -> bool:
    if not path.is_file():
        raise PatchError(f"Required dependency missing: {path}")
    original = path.read_text(encoding="utf-8")
    patched, changed = patch_text(original)
    if changed:
        path.write_text(patched, encoding="utf-8")
    return changed


def patch_project(root: Path) -> list[tuple[str, bool]]:
    results = []
    for env in ENVIRONMENTS:
        path = root / ".pio" / "libdeps" / env / DEPENDENCY_DIR / SOURCE_FILE
        results.append((env, patch_file(path)))
    return results


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project-dir", type=Path, default=Path(__file__).resolve().parent.parent)
    args = parser.parse_args()
    for env, changed in patch_project(args.project_dir.resolve()):
        print(f"BLE Mouse Arduino3 compatibility [{env}]: {'patched' if changed else 'already patched'}")


if __name__ == "__main__":
    try:
        main()
    except PatchError as exc:
        raise SystemExit(f"BLE compatibility patch failed: {exc}") from exc
