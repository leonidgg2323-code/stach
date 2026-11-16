# Stach — Stage 1: Foundation

Early ESP32-S3 **network-only** prototype for the Waveshare 1.43-inch AMOLED project. **Not a validated board firmware.**

## Design requirements
Black background, two white eyes with black pupils, battery at top; no clock, mouth, blush or decorative elements. Eye-only emotions. Wi-Fi QR pairing and BLE mouse are planned future stages.

## Current implementation
- WPA2 Wi-Fi AP with device-specific SSID and 16-character random hexadecimal password persisted in NVS
- Wi-Fi QR **payload** printed to serial (QR matrix is NOT displayed on the AMOLED)
- Phone web UI at `http://192.168.4.1` and emotion API
- Portable core module with unit tests and GitHub Actions workflow

## Tests
Run `sh tools/test_core.sh` on a machine with a C++17 compiler. `pio run` builds the network-only firmware with PlatformIO; CI will attempt both tests and build after workflow is uploaded to GitHub. Local hardware validation has NOT been performed.

## Security notes
Wi-Fi credentials are printed to USB serial for initial setup; avoid sharing serial logs. Local HTTP API is accessible to clients joined to the Stach Wi-Fi network. No cloud services or remote internet control. QR scan only joins Wi-Fi; open the URL separately.

## Future work
Waveshare-specific display, touch, QMI8658, battery, actual QR rendering and Bluetooth HID are NOT implemented in Stage 1. Do not flash this generic profile as a final Waveshare firmware.

Inspired by [NovikovVisuals/Pip-KeyPet](https://github.com/NovikovVisuals/Pip-KeyPet); no upstream source or assets copied.

## Verification record (2026-11-16)

- Portable core: 25 assertions passed with system C++17 compiler.
- Firmware build: **not verified locally** because PlatformIO and ESP32 packages are not available in the offline execution environment. GitHub Actions is configured to build on push.
- Physical hardware: not available, no hardware validation.
- GitHub publication: connector returned HTTP 403 for writes. The ZIP is the authoritative Stage 1 delivery until uploaded.

The stage 1 scope is network foundation only; it is **not** a complete Waveshare firmware.
