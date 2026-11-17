# STACH — Stage 7: Phone Control Center

A responsive **local phone web panel** for Waveshare ESP32-S3 Touch AMOLED 1.43". This project carries forward stages 1–6, including centered, anti-aliased flat-white eyes, eyebrows for emotions only, touchscreen gesture recognition, IMU support, and the two on-device QR pairing screens.

## Open the preview first (no board required)

Double-click `preview/stach-panel.html`. The page automatically enters **DEMO** mode if opened directly as a file. Try the emotion buttons, look joystick, QR-screen selector, automatic-look and tilt switches, and simulated charging / falling / low battery. Demo actions never modify hardware.

`preview/panel-390px.png` and `preview/panel-1280px.png` are screenshots taken in actual Chromium at phone and desktop widths. **They are previews, not screenshots of the real AMOLED.**

## Real device

1. Flash the project built for your exact supported **standard/B** Waveshare controller profile (SH8601 or CO5300; do **not** flash on a C revision without a matching verified BSP).
2. On the display, long-press to show the Wi-Fi QR. Scan it and join the device's `Stach-XXXXXX` WPA2 network. The phone may warn there is no internet, because this is a local access point.
3. Open **http://192.168.4.1/** on the phone, or use the portal QR on Stach. You should see the same panel as the offline demo (the same HTML is embedded in the firmware).
4. Use the controls; the panel refreshes status every 2.5 seconds. It shows an offline indicator if the device is unreachable and recovers automatically.

## Working API and behaviors

| Method + route | Purpose |
| --- | --- |
| `GET /` | Mobile-first panel embedded in the firmware; no CDN, external fonts, cloud, or internet needed |
| `GET /api/status` | SSID/IP, active emotion and pairing screen, gesture/IMU availability, uptime, clients, current toggles, known/unknown battery and charger state, manual override timer; **no Wi-Fi password** |
| `POST /api/expression?value=...` | One of `neutral`, `angry`, `sleepy`, `curious`, `happy`; manual override for 8 seconds |
| `POST /api/pairing?mode=...` | `face`, `wifi`, `portal`; device screen reverts after 90 s |
| `POST /api/gaze?x=...&y=...` | Normalized X and Y in `[-1,1]`; rejected if non-numeric/out-of-range; remote motion expires automatically after 2.4 seconds and is confined to the **center** of the circular screen |
| `GET /api/settings` | Current `idleMotion` and `tiltEnabled` switch states |
| `POST /api/settings?key=...&value=...` | Persist `idleMotion` or `tiltEnabled` as `0/1` in ESP32 NVS, with error handling |

**Design:** A black AMOLED surface with two solid-white eyes, black pupils, no reflections, glow, lips, or surprise expression. The panel preview uses browser CSS and represents an approximate animation; the device uses its existing RGB565 drawing engine.

**Battery/charging:** The status panel shows "Не определена" unless a *real* calibrated battery or charging-state provider has been connected. In the standalone preview, the demonstration controls are intentionally simulated. Falling/charging calibration and physical hardware validation remain in the final hardware stage. Brightness slider is intentionally **not shown** until the panel's supported brightness API has been verified.

**Security:** The portal uses plain local HTTP and relies on the Stach AP password to restrict access. Anyone who learns that password and joins the AP can use its control API. Do not connect untrusted people to the AP. No persistent password is served in `/api/status`, in the HTML, or through serial output. No remote/cloud control is included.

## Files added/changed

- `web/stach-panel.html`: the authoritative frontend source; open directly as an offline demo.
- `include/stach_web.h`: automatically generated PROGMEM copy of the exact HTML.
- `tools/build_web_header.py`: regenerate embedded HTML after frontend changes.
- `include/stach_control.h`: validated remote gaze + settings payload parsing; boundary-safe motion.
- `src/main.cpp`: wired web UI, new API routes, persisted toggle settings and centered remote control.
- `test/control_test.cpp`: 1,809 portable C++ assertions against invalid values, timing wraparound and eye movement bounds.
- `tools/check_panel.py`: DOM/API correspondence check and Node JavaScript syntax check.
- `tools/browser_smoke.py`: optional Chromium interaction tests with phone and desktop screenshots.
- `.github/workflows/check.yml`: includes the new portable web checks and builds all three PlatformIO environments.

## Reproduce tests

```sh
sh tools/test_core.sh
python3 tools/check_panel.py
```

Optional screenshot / UI smoke tests if Chromium and Playwright for Python are installed:

```sh
python3 tools/browser_smoke.py
```

CI also runs PlatformIO builds for `stach-network-prototype`, `stach-waveshare-sh8601`, `stach-waveshare-co5300`. A passing CI build confirms **compilation**, not the physical device, QR scanning on a phone, charger sensing or touch/IMU calibration.

## Upload to GitHub Desktop

Extract the **contents** of the ZIP into your locally cloned `stach` folder (not an extra nested directory). Replace matching files. In GitHub Desktop choose **Commit to main** and **Push origin**. Follow results at https://github.com/leonidgg2323-code/stach/actions. The ChatGPT GitHub connection has been read-only in practice (HTTP 403 for writes).

**Next independent stage:** 8, Bluetooth HID wireless mouse to control Windows; not implemented by this stage.
