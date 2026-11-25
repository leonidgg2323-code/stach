#!/usr/bin/env python3
"""Package built Stach ESP32 firmware images. NEVER creates placeholder .bin files.

To run after successful `python tools/build_firmware.py`.
`firmware.bin` alone is not a full flash image; use PlatformIO's upload target.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
from datetime import datetime, timezone

PROFILES = (
    'stach-eyes-175',
    'stach-network-prototype',
    'stach-waveshare-sh8601',
    'stach-waveshare-co5300',
)
REQUIRED_IMAGES = ('bootloader.bin', 'partitions.bin', 'firmware.bin')


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def package(root: Path, output: Path, commit: str = 'unpublished') -> dict:
    root = root.resolve()
    output = output.resolve()
    if output == root or output in root.parents or (root / '.pio') in output.parents:
        raise ValueError('Refusing to erase project or PlatformIO build directory')
    # Check every input before writing any candidate artifacts.
    sources: dict[str, dict[str, Path]] = {}
    for profile in PROFILES:
        build = root / '.pio' / 'build' / profile
        images = {name: build / name for name in REQUIRED_IMAGES}
        for name, source in images.items():
            if not source.is_file() or source.stat().st_size == 0:
                raise FileNotFoundError(
                    f'Missing nonempty {name} for {profile}: {source}. '
                    'Run a successful PlatformIO build first.'
                )
        sources[profile] = images

    # Avoid a stale successful build accidentally incorporating old files.
    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True, exist_ok=True)
    files: dict[str, dict[str, dict]] = {}
    for profile, images in sources.items():
        folder = output / profile
        folder.mkdir()
        files[profile] = {}
        for name, source in images.items():
            target = folder / name
            shutil.copy2(source, target)
            files[profile][name] = {
                'path': f'{profile}/{name}',
                'size_bytes': target.stat().st_size,
                'sha256': sha256(target),
            }
    manifest = {
        'project': 'STACH',
        'release': '12.0.0-rc2',
        'source_commit': commit,
        'generated_utc': datetime.now(timezone.utc).isoformat(),
        'hardware_validated': False,
        'warning': ('Build success is not proof of device compatibility. '
                    'These are separate bootloader, partition and application images, '
                    'not a single flash-at-0x0 image. Use PlatformIO upload.'),
        'profiles': files,
    }
    (output / 'MANIFEST.json').write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    checksums = []
    for profile, images in files.items():
        for name, item in images.items():
            checksums.append(f"{item['sha256']}  {item['path']}")
    (output / 'SHA256SUMS.txt').write_text('\n'.join(checksums) + '\n', encoding='utf-8')
    (output / 'READ-ME-FIRST.txt').write_text(
        'STACH Stage 12 release candidate. Not physically validated.\n'
        'Select only a firmware for your EXACT hardware revision.\n'
        'The 1.43C board is NOT supported by these profiles.\n'
        'Do NOT flash firmware.bin at offset 0x0: it is an app image only.\n'
        'Recommended flashing: connect board by USB and use PlatformIO\n'
        '  pio run -e stach-waveshare-sh8601 -t upload --upload-port COM5\n'
        'Replace the profile/COM port after confirming your board.\n'
        'Full guide: STAGE12-HARDWARE-TESTS.md in the source archive.\n',
        encoding='utf-8',
    )
    return manifest


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project-dir', type=Path, default=Path(__file__).resolve().parent.parent)
    parser.add_argument('--output-dir', type=Path)
    args = parser.parse_args()
    root = args.project_dir.resolve()
    target = (args.output_dir or root / 'dist').resolve()
    manifest = package(root, target, os.getenv('GITHUB_SHA', 'unpublished'))
    print(f"STACH release bundle created: {len(manifest['profiles'])} firmware profiles")
    print(f'Artifact path: {target}')


if __name__ == '__main__':
    main()
