# STACH — Stage 6: Wi-Fi QR onboarding

Minimal companion on the Waveshare ESP32-S3 Touch AMOLED 1.43" (466 × 466). This archive **retains the centered, non-glowing eye style** and stage-5 touch and gesture firmware. This revision adds a physical QR screen and paired network/portal onboarding.

## Phone pairing

1. Boot the Stach board. It starts an independent WPA2 Wi-Fi network named `Stach-XXXXXX` with an individual 16-character random hexadecimal password stored in ESP32 NVS.
2. **Long-press** Stach's touchscreen (~650 ms) on the eye screen to reveal the **Wi-Fi QR**. Scan it using your phone's camera and choose to join the network. A phone can report **"No Internet"**: that's normal, because this local access point has no internet uplink.
3. **Tap** Stach once to show the **portal QR** containing the actual web address (normally `http://192.168.4.1/`). Scan it or type the URL into your browser **while connected to Stach Wi-Fi**.
4. Tap to switch the two QR codes, double-tap or long-press to return to the eyes. The QR screen closes automatically after 90 seconds without an interaction.
5. Once connected, the web page also has three remote controls: display Wi-Fi QR, display portal QR, return to face. This works only after your phone has already joined the Stach Wi-Fi network.

Some mobile operating systems will **not automatically open a browser after joining Wi-Fi** from a QR. This is why the second code exists. The two QR codes are intentionally distinct. Never rely on the phone having an internet connection to Stach.

## QR specifications

- Uses `ricmoo/QRCode@0.0.1` for QR matrix generation. MIT license: https://github.com/ricmoo/QRCode
- QR **version 7, ECC medium**, fixed 45 × 45 modules and true 4-module white quiet zone.
- Positive black-on-white, 5-pixel square module = 265 px outer square centered at (100,100) on the 466 × 466 circular display. All four corners are inside the circular panel.
- The QR is redrawn **once per screen change**, not every animation frame. The eye engine resumes when closing the QR.
- No server or internet request is required to construct or display a QR.
- The saved per-device password is **not** included in HTTP `/api/status` or printed into USB serial logs. It is encoded in the on-device Wi-Fi QR only; anyone able to see that QR can join the local AP.

## HTTP API

`GET /api/status`: includes non-secret `ssid`, `ip`, `pairingScreen`, sensors and expression.

`POST /api/pairing?mode=wifi|portal|face`: display a pairing screen or resume the face (HTTP 400 on invalid mode). **The API is accessible to Wi-Fi clients on the local AP; it is not an internet-exposed authenticated service.** Do not attach untrusted users to the Stach AP.

`POST /api/expression?value=neutral|angry|sleepy|curious|happy`: earlier expression override.

## Included files

- `include/stach_pairing.h`: portable UI state machine, timeout and QR layout; native C++ tests.
- `include/stach_qr_display.h`: QR encoder integration and QSPI panel rendering (hardware profiles only).
- `src/main.cpp`: Wi-Fi+portal setup, HTTP pairing API, touch gestures, QR/face modes.
- `platformio.ini`: adds QRCode dependency for both Waveshare profiles; network-only profile needs none.
- `test/pairing_test.cpp`: 33 pairing/password/layout assertions.
- `preview/qr-wifi-example.png`, `preview/qr-portal-example.png`: sample QR images with **fake demo credentials**; both decoded with OpenCV before packaging.
- `preview/stach-pairing-preview.html`: offline clickable demonstration for Windows browser.

## Build and verification

Run portable tests with `sh tools/test_core.sh` on Linux/macOS/WSL. GitHub Actions additionally builds the three ESP32-S3 profiles on push (`stach-network-prototype`, `stach-waveshare-sh8601`, `stach-waveshare-co5300`). Once uploaded, open https://github.com/leonidgg2323-code/stach/actions to inspect the actual PlatformIO result.

**Verified in development**: native C++ tests, QR data & geometry checks, example QR decoding, JavaScript syntax checks and ZIP integrity. **Not verified in development**: full new PlatformIO firmware build, actual AMOLED output, physical touch gestures or phone scan on the real board. These require GitHub Actions and then hardware. The project-wide hardware validation stage remains at the end by request.

## Board compatibility

The current GPIO map and panel profiles target **Waveshare ESP32-S3-Touch-AMOLED-1.43 standard/B**, not the differently wired C model. Choose the real LCD controller profile (SH8601 or CO5300). Refer to Waveshare's PCB revision and schematic prior to flashing. A working USB power supply and display are needed to verify the QR in practice.

## Upload via GitHub Desktop

Copy the **contents** of this ZIP into your cloned `stach` folder, replacing files. Open GitHub Desktop -> Changes -> Commit to main -> Push origin. A green CI run verifies build, not the physical board. No need to upload zipped binary files to the repository.
