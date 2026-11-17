# STACH — Stage 9: Battery & Power Saver

**Target hardware:** Waveshare ESP32-S3-Touch-AMOLED-1.43 / compatible 1.43B configuration (SH8601 and CO5300 build profiles). **Not** the 1.43C variant; that is a different board. This is an extension of Stage 8, not a replacement.

## New features

1. Top-of-face battery icon drawn directly on the 466 × 466 AMOLED. Shows `?` when battery capacity cannot be trusted. After calibration, it shows an **approximate** percentage. The neutral eyes stay matte white with black pupils and no eyebrows, and gaze movement stays in the center.
2. `BAT_ADC` GPIO4 readings using `analogReadMilliVolts` (5 samples, median, then exponential low-pass filter). The ADC voltage alone is **not** automatically classified as battery voltage or battery percentage. USB/system measurements must not be misrepresented as cell charge.
3. User-driven calibration, optional and potentially unsafe if done with wrong assumptions: **only** after checking the specific revision's schematic and with a multimeter verifying that GPIO4 tracks the **cell voltage**, measure battery millivolts, use the web control panel's advanced calibration field, and save the ADC-to-cell factor in NVS. Any percent is merely a **voltage-based estimate**, NOT a fuel-gauge measurement or precise SoC reading. If uncertain, **do not calibrate**; `?` is the safe behavior.
4. Power-saving policy, enabled by default: 10 Hz face updates initially, 2.5 Hz after 45 seconds with no real interaction, black AMOLED face after 5 minutes idle. With known charge ≤15%, these intervals shorten to 10 seconds and 2 minutes. Touch or a POST from the phone wakes the face. Mouse and QR screens remain awake, as does a verified active charging animation. BLE and Wi-Fi remain active; this is **not hardware deep sleep** and does not claim a measured battery-runtime improvement.
5. New local API: `POST /api/power/saver?enabled=0|1`, `POST /api/power/wake`, `POST /api/power/calibrate?cellMv=3900`. `GET /api/status` now includes `batteryAdcMv`, `batteryMv`, `batteryCalibrated`, `screenMode`, and `powerSaver`. The web panel includes a new energy card, on-demand wake, optional manual calibration, live voltage indicators, and a battery percentage only when valid.
6. Charging status is **UNKNOWN by default**, even when connected to a computer. Therefore no automatic cheerful charging reaction or green charging sweep is falsely activated. Hardware charging-status detection is optional via a verified `STACH_CHARGE_STATUS_PIN` build flag (active-low), NOT enabled on the default standard board. Never assign an arbitrary pin based on the charge LED. When a real charger status signal is later wired and verified, the existing charging emotion and animation activate automatically. The 'sleepy after a long time not charged' behavior is now gated on known charging status, so it will not activate solely because charging detection is missing.
7. Previous controls, mouse, gestures, QR and eyes are retained.

## Important limitations

- **No physical Waveshare or Li-ion battery was connected during this stage.** The battery divider source and exact charger pin must be validated on the specific hardware revision. The wiki lists `BAT_ADC` GPIO4 and a charge indicator but its ADC demo measures *system voltage (~4.9V when USB-powered)*. That demo does not establish precise lithium SoC.
- No safe/verified display brightness control is implemented. The generic Arduino_GFX pointer lacks `setBrightness`. The energy saver reduces redraws and shows black pixels; a real AMOLED standby command, BLE duty cycle optimization, RTC wake, power-rail changes, and battery calibration belong to final hardware integration. Display blacking alone must not be called deep sleep.
- Charging, battery percentage and power consumption measurements **cannot be asserted as working on the board** without physical test.
- The standard board's GPIO4 and I2C pin assignments are not suitable for the distinct 1.43C hardware variant.

## Testing

- `sh tools/test_core.sh` — all portable tests including Stage 9 battery interpolation, calibration rules, filtering and 32-bit time-wrap-safe saver policy.
- `python3 tools/check_panel.py` — embedded web page and API names; `python3 tools/test_ble_patch.py` — BLE library compatibility patch tests.
- `python3 tools/browser_smoke.py` — optional full Chromium UI smoke, screenshots at 390 × 844 and 1280 × 900 (requires installed Playwright and Chromium).
- `python3 tools/build_firmware.py` — installs dependencies and builds network, SH8601 and CO5300 profiles under PlatformIO, including the Stage-8 BLE compatibility patch. GitHub Actions executes this on push.
- An extra local `g++ -fsyntax-only` check was run for all three profiles using lightweight **mock Arduino APIs** (not a real ESP32 toolchain); these mocks are deliberately *not* shipped as production source.

## Install with GitHub Desktop

1. Unzip the **contents** of this archive into the existing local `stach` repository folder, replacing files. Keep the `.github` folder and all project directories.
2. In GitHub Desktop select `Commit to main`, then `Push origin`.
3. Watch the GitHub Actions `Stach core checks` run. Green tests and a full green firmware build confirm compilability, **not physical board function**.
4. Device programming and battery/charger calibration remain in the final hardware-verification stage.

## Credits

Independent Stach firmware concept inspired by the Pip-KeyPet genre. Core code developed for Stach; retains prior project attributions and no third-party BLE libraries are included verbatim. The patched BLE Mouse 0.3.1 is installed by PlatformIO and adapted by `tools/patch_ble_mouse.py`.

Reference: <https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43>
