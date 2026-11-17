# STACH Stage 11 — Reliability & Security

## Security model (important)

The ESP32 runs a **password-protected Wi-Fi access point** and a **plain HTTP** portal at `http://192.168.4.1`. This release strengthens the HTTP API against **cross-site request forgery (CSRF)** and simple request floods. It **does not provide end-to-end encryption** or separate authorization between people who know the Wi-Fi password. **Anyone admitted to the Stach AP can obtain a session token and operate the device.** Do not share the QR code or Wi-Fi password with untrusted people.

### Changes

- Every mutating HTTP endpoint requires `X-Stach-Token` (32 random hex characters, generated with ESP RNG on each boot). The browser obtains it from same-origin `GET /api/session`; no manual user action or token persistence is needed. It is never stored in NVS and never printed on serial. Reloading after a device restart obtains the new token.
- All 21 web/endpoint handlers use a centralized guard. Unexpected `Host`, cross-site `Origin`, and unexpected `Sec-Fetch-Site` are rejected. Allowed portal address: `http://192.168.4.1` (port 80).
- No permissive CORS headers; CSP limits the panel to same-origin connections; anti-framing, no-sniff, no-store and no-referrer headers are set.
- Max 512-byte HTTP request body, 8 query/body parameters, 120-character path; fixed-memory per-IP request-rate gate: burst 30 and replenish up to 25 requests/sec per active AP client. Abusive requests return 429; input over-limit returns 413. These are application-level checks, *not* a defense against radio jamming, pre-parser memory exhaustion, or a fully malicious Wi-Fi client.
- AP startup retries every 20 seconds if configuration/start failed or Wi-Fi AP mode/IP becomes invalid. The retry does not erase preferences; it is not a guarantee of recovering from every radio/driver fault.
- `GET /api/health` exposes uptime, internal free/minimum heap and AP status/station count (no passwords, session tokens, or device secrets). Existing `/api/status` remains compatible.
- Mouse HID remains opt-in and returns to Face after reboot; requests still pass the active-mode / BLE-connected guards from Stage 8.

### Compatibility

- No change to the approved display style: flat black background, matte white oval eyes and black pupils, neutral face without eyebrows, centered movement.
- Offline `preview/stach-panel.html` remains a demo; it does not connect to the board. The real device page requests the token automatically.
- Direct scripts calling POST API methods must **first GET** `/api/session` with `Host: 192.168.4.1`, then supply `X-Stach-Token` on POST, and must be connected to the Stach AP.
- Portal access by arbitrary hostnames (including `stach.local`) is intentionally rejected. Use `http://192.168.4.1`.

### Check and test

- `sh tools/test_core.sh` (portable C++ tests including Stage 11 token/origin/host/rate-gate cases).
- `python3 tools/security_integration_check.py` (verifies route guards and client-side token integration).
- `python3 tools/check_panel.py` (identical embedded web app, JavaScript syntax and API contract).
- `python3 tools/test_ble_patch.py` (BLE compatibility patch tests).
- `python3 tools/browser_smoke.py` (optional actual Chromium UI demo on phone and desktop sizes).
- `python3 tools/browser_security_smoke.py` (optional actual Chromium testing with MOCK HTTP responses, validates token headers; not a network vulnerability scan).
- `python3 tools/build_firmware.py` (full PlatformIO build via GitHub Actions after push; not run inside this environment).

### Suggested manual physical tests (final stage)

1. Upload to the supported Waveshare revision after GitHub Actions is green; connect to Stach Wi-Fi using the physical QR; open `http://192.168.4.1`.
2. Verify eyes, emotions, mouse mode and QR still respond to commands from the portal.
3. Restart the device. With a browser page left open, the first POST may receive 403, and a refreshed page must work. It should not accidentally send mouse input while reconnecting.
4. From a machine on the AP: a POST without `X-Stach-Token` must be 403; a valid token should allow it. Use only your own devices and your own network.
5. Attempt a request with an unexpected HTTP `Host`: it must be rejected. A foreign `Origin` or `Sec-Fetch-Site: cross-site` must also be rejected.
6. Monitor `GET /api/health` during prolonged use; compare internal free/minimum heap and test repeated Wi-Fi disconnect/reconnect without restarting. Do not claim 24-hour stability without a 24-hour run.

**Not completed / not asserted:** physical touch/BLE/AMOLED validation, real network adversarial audit, Wi-Fi RF fault recovery, battery calibration, charger-status detection, or long-duration field tests. All remain in the final hardware integration and validation stage.
