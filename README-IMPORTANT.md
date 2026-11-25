# STACH Canvas CO5300 — experimental repair, not a verified final fix

This update preserves STACH (main firmware with Wi-Fi, BLE, face UI, QR, touch and IMU code) and replaces the **display path** with the canvas approach used by a demonstrated working Waveshare 1.43 CO5300 Arduino project. It introduces a separate PlatformIO environment so existing project settings are preserved.

### What changed
- `Arduino_CO5300` with column offsets `(6,0,6,0)` as in the demonstrated project.
- QSPI interface explicitly non-shared, `Arduino_Canvas` framebuffer.
- `begin(80000000)`, rotation 0, explicit `flush()` on startup, test pattern and periodically during rendering.
- Existing hardware power-first + `S` I2C diagnostic from preceding STACH patch retained.

Reference: https://github.com/thelastoutpostworkshop/waveshare_esp32s3_1.43_amoled_xwing_game

### Important limits
The example's CO5300 controller is **not verified for the user's individual panel**. The hardware has already shown an entirely blank screen with previous Arduino_GFX and esp_lcd attempts. This patch has been structurally checked, but not fully built or tested on the real ESP32-S3. It must NOT be described as certainly working. An ESP32-S3 factory backup of 16,777,216 bytes must be retained separately. Windows Defender need not be disabled.

### Use
1. Extract this ZIP with folder structure into `C:\stach-build` replacing the three files. Make a copy of the old files first if you want to keep local modifications.
2. Close the COM3 serial monitor (Ctrl+C).
3. In PowerShell:

```powershell
cd "$env:USERPROFILE\stach-build"
python -m platformio run -e stach-waveshare-canvas-co5300
```

4. **Only if build SUCCESS** and you choose to test the new code:

```powershell
python -m platformio run -e stach-waveshare-canvas-co5300 -t upload --upload-port COM3
```

5. If the screen is blank, stop flashing. Do not erase flash. Restore the known-good backup (after closing serial monitor):

```powershell
python -m esptool --chip esp32s3 --port COM3 --baud 115200 write-flash 0x0 .\stach-factory-backup.bin
```

### Feedback
Observe physically whether the eyes are visible. If still black, send the boot log; do not repeat uploads. This build does not solve `I2C devices=0` on its own.
