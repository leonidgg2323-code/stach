# Stach — ESP32-S3 AMOLED project (early prototype)

A minimalist two-eye companion inspired by the Pip-KeyPet concept, targeted at the **Waveshare ESP32-S3 Touch AMOLED 1.43-inch**.

## Current status

**Prototype, NOT a complete firmware port.** This repository currently implements an ESP32-S3 Wi-Fi access point, per-device persistent random password, browser control panel, and an emotion-control API. It does **not yet** drive the Waveshare AMOLED display, draw the QR code on the device, read battery percentage, or animate the physical eyes. The HTML face is only a browser preview.

## Design specification

- Main screen: pure black, exactly two white eyes with black pupils, battery indicator at top.
- No clock, mouth, blush, cheeks, hearts, or decorative elements.
- Animations and emotion states should alter the eyes only.
- Product name: Stach.
- QR onboarding: a Wi-Fi QR code on the device for joining the Stach AP; then browse to `http://192.168.4.1`.
- A second QR code with the portal URL may be provided for convenience. Scanning a Wi-Fi QR code does not automatically open the browser on all phones.

## Build prototype

Install VS Code + PlatformIO. Open this directory and run `pio run`. The generic ESP32-S3 board profile is for initial **network-side development only**; verify Waveshare-specific flash, PSRAM, USB, power and display settings before flashing a real device.

On first boot the serial console prints the unique Wi-Fi SSID, password and Wi-Fi QR payload. Connect a phone to the SSID and open `http://192.168.4.1`. `/api/status` returns state and `POST /api/expression?value=angry` changes the stored expression in RAM.

## Required next hardware work

1. Confirm Waveshare's exact 1.43-inch display revision and vendor example project.
2. Port the vendor's AMOLED QSPI panel initialization, LVGL flush callback and backlight/power controls.
3. Implement 466x466 eye renderer and animations; connect the expression API to rendering.
4. Render the `qrPayload` string as an actual QR image on the AMOLED screen using a QR library.
5. Integrate touch, battery ADC/PMIC and charging-state measurement using documented board drivers.
6. Validate display, touch, battery, sleep/wake and QR scanning on physical hardware.

## Attribution

Inspired by [NovikovVisuals/Pip-KeyPet](https://github.com/NovikovVisuals/Pip-KeyPet). This prototype contains no copied Pip-KeyPet source code or graphic assets. Check upstream license and preserve notices if later importing code/assets.
