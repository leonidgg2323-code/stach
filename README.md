# STACH — Stage 8: Bluetooth HID Mouse

Independent eye-companion project for **Waveshare ESP32-S3 Touch AMOLED 1.43** (standard / B). This archive contains the whole Stage 7 project plus Stage 8's **Bluetooth Low Energy HID mouse** implementation, rather than a standalone patch. Screen panel alternatives are available (`stach-waveshare-sh8601`, `stach-waveshare-co5300`) but physical compatibility depends on the exact board revision. Do **not** use these profiles for the 1.43C without checking the schematic.

## New features

- **Stach Mouse**: BLE HID mouse reports sent directly to the paired Windows/Mac/Linux computer; **no desktop helper app** is needed (computer must support BLE). Pair in Windows **Settings → Bluetooth & devices → Add device → Bluetooth → Stach Mouse**.
- Explicit opt-in: **swipe up** on Stach's face to enter Mouse Mode, or tap **Enable mouse** in its Wi-Fi control panel. **Long press** on the display (650 ms) to leave Mouse Mode, or turn it off in the panel. No HID input is generated while Mouse Mode is off or Bluetooth is disconnected. This does not require the PC to join the Stach Wi-Fi network.
- First pairing: after enabling Mouse Mode, look for Bluetooth device `Stach Mouse`. Once paired, put Stach on a flat, still surface or hold completely still for roughly 48 consecutive IMU samples (~1 s) while the built-in gyroscope calibrates. The screen displays status `HOLD DEVICE STILL` until calibration finishes.
- **Air mouse:** turn/tilt Stach to move the cursor. Default horizontal axis: gyroscope Z; vertical axis: gyroscope X. You can switch horizontal axis Z→Y, invert X/Y, and change sensitivity (0.5×–4×) in the web panel. Exact axis orientation must be calibrated with the **real** board.
- **Touch gestures in Mouse Mode:** single tap = left click, double tap = right click, swipe up/down = vertical scrolling, swipe left/right = horizontal scrolling, long press = return to face. Single-tap action occurs *after* a ~350 ms double-tap disambiguation window. No accidental click before a second tap.
- **Phone control:** the Stach panel at `http://192.168.4.1/` has on/off, status, calibration, click buttons, wheel buttons, and a small touchpad that forwards relative cursor motion to BLE. Phone must join Stach's own Wi-Fi AP; only a paired BLE computer receives inputs.
- Cursor safety: gyroscope bias calibration, dead zone, low-pass smoothing, bounded velocities and per-update HID deltas, no jumps after reconnection or stalled sampling.

## Technical approach

`include/stach_mouse.h` implements portable (host-testable) state, calibration, relative motion calculations and gesture-action mapping. `include/stach_ble_mouse.h` owns the Arduino BLE HID transport, using the PlatformIO `t-vk/ESP32 BLE Mouse@0.3.1` dependency. `include/stach_mouse_ui.h` draws the mouse mode on the circular AMOLED. `src/main.cpp` integrates BLE, QMI8658 readings, FT3168 touch gesture events and Wi-Fi APIs. `web/stach-panel.html` and `include/stach_web.h` (generated) contain exactly the same phone panel as `preview/stach-panel.html`.

Bluetooth starts **lazily**, on the first entry into Mouse Mode. In `ESP32 BLE Mouse 0.3.1`, `BleMouse::end()` is a no-op; BLE advertising may continue when you exit mouse mode. **All HID send methods are individually gated by `engine.active()` and connection state**. Rebooting Stach exits Mouse Mode completely. A future lifecycle stage can replace the BLE stack if stopping advertising is needed.

## Builds and testing

- `sh tools/test_core.sh` runs the existing tests and new **mouse policy** tests on host C++17.
- `python3 tools/check_panel.py` checks the embedded panel, JS syntax and endpoint references.
- `pio run -e stach-network-prototype -e stach-waveshare-sh8601 -e stach-waveshare-co5300` is the PlatformIO build check in `.github/workflows/check.yml`.
- In the extracted ZIP, double click `preview/stach-panel.html` to try offline demo controls. The demo cannot actually move your PC cursor.
- No full PlatformIO build or on-device Bluetooth pairing tests were performed **at the time of packaging**. GitHub Actions will check full compilation after you push, and the real device and Windows pairing still require hardware verification. Do not intentionally drop the hardware to test fall detection.

### Source-to-release workflow with GitHub Desktop

1. Extract the **contents** of this ZIP into the cloned `stach` repository root; replace older files.
2. In GitHub Desktop commit e.g. `Stage 8: BLE HID Mouse`, then click **Push origin**.
3. Open `https://github.com/leonidgg2323-code/stach/actions` and check the new three-profile build. If it fails, share the job's compiler error (not just the red cross).
4. Choose the exact AMOLED controller corresponding to your board revision before flashing. Physical BLE, calibration and display are not verified by passing cloud compilation alone.

## REST interface (from the Stach Wi-Fi AP only)

All changing requests use `POST`; the local AP password is the initial access boundary. Do **not** expose the control panel to the internet.

| URL | Form fields | Effect |
|---|---|---|
| `/api/status` | GET | Includes `mouseMode`, `mouseBleStarted`, `mouseConnected`, `mouseCalibrated`, sensitivity and orientation settings |
| `/api/mouse/mode` | `enabled=1` or `0` | Enable or disable Mouse Mode. Prototype board returns HTTP 503. |
| `/api/mouse/calibrate` | (none) | Reset gyro calibration; requires Mouse Mode. |
| `/api/mouse/config` | `key=sensitivity&value=1.25` | Save sensitivity 0.5–4.0 |
| `/api/mouse/config` | `key=invertX|invertY|axisYX&value=0|1` | Save axis mapping / inversion |
| `/api/mouse/click` | `button=left|right|middle` | BLE click, only when enabled and connected |
| `/api/mouse/scroll` | `direction=up|down|left|right` | BLE scroll; only when enabled and connected |
| `/api/mouse/move` | `dx=-40..40&dy=-40..40` | Relative BLE movement; only when enabled and connected |

**Known limitations:** no verified on-device calibration yet; sensor mounting orientation may need inversion. The old BLE library doesn't stop advertising on exit. BLE HID bonding is delegated to the library/OS, not an independent authorization protocol. The phone's local HTTP endpoints are not hardened against a malicious page running on an already-connected network; keep Stach's Wi-Fi password private and don't port-forward the AP. Battery measurement and charging detection are still pending later stages.

## Credits

Uses [T-vK/ESP32-BLE-Mouse](https://github.com/T-vK/ESP32-BLE-Mouse), a third-party library. Stach remains an independent project inspired by eye companions, including [Pip-KeyPet](https://github.com/NovikovVisuals/Pip-KeyPet); no code from that project is included.
