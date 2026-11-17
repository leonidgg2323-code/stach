# STACH Stage 10 — Verification report

Date: 2026-11-17

## Tests run locally in this environment

- C++17 `sh tools/test_core.sh`: all 11 existing portable test programs and new `test/modes_test.cpp` PASS; `modes_test` includes **792 Stage-10 assertions**.
- `python3 tools/check_panel.py`: PASS; 59 unique DOM ids; JavaScript passes `node --check`; HTML identical between web panel, offline preview and firmware header; no external scripts or assets.
- `python3 tools/test_ble_patch.py`: PASS, 8 BLE patch compatibility tests.
- `python3 tools/browser_smoke.py`: PASS in headless Chromium at 390 x 844 and 1280 x 900; modes, settings, gesture preview, emotion control, previous features; no page errors; no horizontal overflow at mobile size.
- C++17 `g++ -fsyntax-only` of `src/main.cpp` for network-only, SH8601, and CO5300 profiles using **mock Arduino APIs**: PASS. This is NOT a genuine ESP32 toolchain build.
- ZIP: verified decompression and CRC after packaging.

## External status and limitations

- Previous GitHub Stage 9 run: success, https://github.com/leonidgg2323-code/stach/actions/runs/44063503382
- Stage 10 new PlatformIO build: NOT YET RUN; user must push archive contents for the GitHub Actions pipeline to run.
- AMOLED/touch/charger/BLE physical validation: NOT RUN (requires exact supported board, battery, Windows PC for BLE pairing).
- Eye brightness is flat RGB565 pixel intensity; not verified AMOLED hardware current or PWM brightness.
- The software preferences use ESP32 NVS and are loaded at setup; real power-cycle persistence verification requires physical hardware.
- After reboot Mouse Mode is OFF by design (HID safety).

## Source provenance

Stage 10 project is derived from the full Stage 9 Stach source, with new mode integration, appearance controls, portable tests, embedded web UI and documentation. No private tokens or device passwords are bundled.
