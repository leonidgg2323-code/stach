#!/usr/bin/env python3
"""Reproducible Stach build: install BLE deps, apply API patch, compile all envs.

Requires PlatformIO Core on PATH (`pip install platformio`).
Usage: python tools/build_firmware.py
"""
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parent.parent
PROFILES = ["stach-network-prototype", "stach-waveshare-sh8601", "stach-waveshare-co5300", "stach-waveshare-bringup"]


def main() -> None:
    # PlatformIO installs the pinned 0.3.1 library into .pio/libdeps/<environment>/
    install = ["pio", "pkg", "install"]
    for profile in PROFILES:
        install += ["-e", profile]
    print("[STACH] Installing pinned PlatformIO dependencies", flush=True)
    subprocess.run(install, cwd=ROOT, check=True)

    print("[STACH] Applying BLE Mouse compatibility patch", flush=True)
    subprocess.run([sys.executable, str(ROOT / "tools" / "patch_ble_mouse.py"),
                    "--project-dir", str(ROOT)], cwd=ROOT, check=True)

    build = ["pio", "run"]
    for profile in PROFILES:
        build += ["-e", profile]
    print("[STACH] Building all three firmware profiles", flush=True)
    subprocess.run(build, cwd=ROOT, check=True)


if __name__ == "__main__":
    main()
