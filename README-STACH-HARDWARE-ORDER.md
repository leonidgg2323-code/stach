# STACH — Waveshare 1.43 power/I2C startup order fix (experimental)

This update fixes one confirmed discrepancy between STACH's initialization order and Waveshare's original factory driver. Before this patch STACH probed the I2C peripherals before enabling AMOLED power on GPIO42. This patch initializes the display and lets the power settle *before* probing I2C and starting IMU/touch drivers. It also provides serial command `S` to show I2C ACK addresses, not just `Wire.begin()==true`.

The display still uses Arduino_GFX in this patch. **No guarantee of visible graphics; this is not a complete display-driver fix.** The original factory image is already proven to display on the user's board. Keep a validated original full 16,777,216-byte firmware backup safely stored.

## Windows: apply to existing `C:\stach-build`

1. Extract archive preserving `src` and `include` subdirectories. Approve replacement of `src\main.cpp` and `include\stach_hardware.h`. Do not replace any other files.
2. Close serial monitor with Ctrl+C. In PowerShell:

```powershell
cd "$env:USERPROFILE\stach-build"
python -m platformio run -e stach-waveshare-sh8601
```

3. Only if compile succeeds, and the existing 16 MB factory backup is safe, flash:

```powershell
python -m platformio run -e stach-waveshare-sh8601 -t upload --upload-port COM3
python -m platformio device monitor -p COM3 -b 115200
```

4. When connected, send `S` in the serial monitor. Collect `[I2C] ACK 0x...` and `[HWSELFTEST]`, and note whether the screen actually shows eyes.

This is a developer validation build: it has not been compiled on the user's PC, nor tested on the physical panel. If the display remains blank, **stop reflashing** and restore the verified factory firmware when desired.
