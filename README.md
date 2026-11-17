# STACH — Stage 5: Touch + Gestures + IMU (standard Waveshare 1.43 / 1.43-B)

Minimal round AMOLED companion. Design remains: flat black, exactly two white anti-aliased eyes and black pupils, **no highlights/glow**, neutral eyes even and equal-size, eyebrows **only** when expressing an emotion; no surprised expression.

## Eye centering correction (v3)

Previous Stage 5 moved the whole pair up to 47 pixels on a 466-pixel round screen.
The browser gesture demo also retained the last swipe direction indefinitely.
**Both issues are fixed.** The whole pair now moves just 18px for a glance
(and at most 20px horizontally / 12px vertically, even after combined tilt and
swipes). Pupils still look around; the nearer eye becomes subtly larger.
The updated browser gesture demonstration automatically recenters after 1.6s.
This central-area restriction is enforced in the renderer itself, not just in
preview code. All brows still appear only with emotions; flat-white eyes remain
without glow. Fixed versions of all preview animations are included.

## Stage 5 changes

- `include/stach_touch.h`: independent FT3168/FT6146 register decoding and gesture recognizer, no Arduino dependencies. Outputs `tap`, `double_tap`, `long_press`, `swipe_left/right/up/down`.
- `include/stach_touch_hal.h`: I2C touch read on the **existing shared Wire bus**, at 0x38; controller read via registers 0x02–0x06 every 20 ms. Failed I2C reads are not treated as taps.
- `include/stach_motion.h`: acceleration-based tilt smoothing, safe rejection of freefall/impact samples, temporary and smoothly released gaze in response to swipes. Raw gyro data is also available for future Bluetooth HID mouse mode.
- `include/stach_imu.h`: QMI8658 acceleration **and** gyroscope reads, from 12-byte axis register block at 0x35, with enabled 6DOF and little-endian autoincrement. Requires hardware orientation/range calibration before release.
- `src/main.cpp`: hardware touch/IMU poll, event-driven curious reaction, eye glance feedback, `GET /api/status` now reports `touchReady`, `imuReady`, `lastGesture`, `lastGestureMs`, `tiltX`, `tiltY`, `gyroX/Y/Z`. `network-prototype` continues to work (sensor availability false). Existing web UI displays sensor status.
- `test/touch_gestures_test.cpp`: portable unit tests for touch protocol, gesture classification, rollover, motion filtering and gaze.
- `preview/stach-touch-gestures.html`: standalone browser visual demonstration. **Simulation**, not physical hardware; JS gestures are independent of C++.

## Behavior

- One finger tap, double tap, or long hold: causes *curious* reaction (because the user touched the device). Double tap also centers the gaze override.
- One finger swipe: glances in its direction, then smoothly returns (gesture takes priority for 1.6 seconds).
- Tilt (accelerometer): adds a small eye offset. Motion over 1.3g or under 0.75g is ignored by the tilt filter; freefall/impact detection in `stach_behavior.h` still handles *angry*.
- Sensors are never claimed as connected when on the network-only build. No BLE mouse reports or QR UI are enabled in this stage.
- The touch origin and axis direction assume rotation 0 and 466×466 as configured in `stach_display.h`. A real board may need axis inversion or swapping.

## Build / test

CI uses `.github/workflows/check.yml`: `sh tools/test_core.sh` and `pio run -e stach-network-prototype -e stach-waveshare-sh8601 -e stach-waveshare-co5300`.

To run local pure C++ tests on Linux/macOS/WSL: `sh tools/test_core.sh`. To build full firmware after uploading through GitHub Desktop, open the new run in GitHub Actions. The last successful pre-stage-5 commit was `b8ff7eb6f906aff5f81fecc97f5b9b59fc0e301d`. The current archive must be uploaded for its own CI build to run.

**Important:** A 1.43C model has different components (including CST820 touch) and cannot use this firmware unchanged. Board-specific AMOLED, touch orientation, IMU axes and event thresholds remain subject to real hardware testing at the final integration step. Never drop the device on purpose to test impact detection.

## Install via GitHub Desktop

Extract the **contents** of the archive into your existing cloned `stach` repository, replace files. GitHub Desktop: **Commit to main → Push origin**. Visit `https://github.com/leonidgg2323-code/stach/actions` to inspect the latest results. The connected GitHub API has allowed reading but returned HTTP 403 for writes; no remote commit is claimed.

## Datasheet references

- Waveshare product: https://www.waveshare.com/product/esp32-s3-touch-amoled-1.43.htm
- Waveshare documentation: https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-1.43/Arduino
- QMI8658 register documentation: https://files.waveshare.com/wiki/common/QMI8658A_Datasheet_Rev_A.pdf
