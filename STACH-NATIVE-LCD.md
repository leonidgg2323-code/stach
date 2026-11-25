# STACH: native AMOLED recovery test v0.2 (Waveshare 1.43 / 1.43-B)

## Why this exists

The user's exact board restores and displays factory firmware successfully, but STACH's Arduino_GFX profiles and the Arduino_GFX bring-up tool failed to draw anything. The board runs STACH and prints USB diagnostics, so a hardware failure is not supported by the observations. This prototype tests the **vendor's ESP-IDF esp_lcd QSPI command protocol**, not another Arduino_GFX variant.

This is a **diagnostic firmware, not finished STACH**; it deliberately has no Wi-Fi, BLE, touch controls, or eye animation. It cannot be claimed as working until the user sees pixels on the board. STACH source and settings remain unchanged in the repo unless the user replaces them.

## Grounding

Adapted from official Waveshare ESP32-S3-Touch-AMOLED-1.43 `07_LVGL_Test` esp_lcd_sh8601.h, lcd_bsp.c, esp_lcd_sh8601.c (Apache-2.0 licensed upstream files):

- Same 1.43 pins: CS9, SCLK10, D0-D3 11..14, RESET21, AMOLED_EN42.
- esp_lcd SPI2_HOST QSPI, `.quad_mode=true`, 32-bit LCD commands, 8-bit parameters.
- Command frame `0x02 00 cmd 00` and color frame `0x32 00 2C 00`.
- Vendor SH8601/CO5300 init sequences with explicit `0x11`, `0x29`, `0x51 = 0xFF`.
- Draws through DMA with completion callback, checking SDK errors; **SDK return success still does not imply physical panel ACK**.
- Panel ID read is indicative only. `0xFF` can be an unconnected/pulled-up line; use the `1` and `2` manual override to compare both sequences in one diagnostic image.

Official source: https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.43/tree/main/examples/arduino/examples/07_LVGL_Test

## Safe test on Windows

Keep factory backup 16,777,216 bytes in its secure location. Copy ZIP contents to `C:\stach-build` and run:

```powershell
cd "$env:USERPROFILE\stach-build"
python -m platformio run -e stach-waveshare-native-lcd
```

If and **only if** the compile succeeds, and the actual board is Waveshare ESP32-S3-Touch-AMOLED-1.43-B (not C), close Serial Monitor, then:

```powershell
python -m platformio run -e stach-waveshare-native-lcd -t upload --upload-port COM3
python -m platformio device monitor -p COM3 -b 115200
```

A repeated sequence of white screen / RGB color blocks / eyes should appear if native ESP-LCD communication succeeds. On USB monitor, `1` selects SH8601 vendor commands, `2` selects CO5300, `D` redraws, `I` rescans I2C. Try only one switch after checking initial picture, record any errors. If there is no image after both, restore the proven factory backup and stop tests; obtaining boot, LCD-ID, I2C and SPI errors will support the next investigation.

**Do not use `erase-flash`, change the offsets manually, or expose the full factory image publicly.** Uploading this diagnostic replaces the factory image on device, although the backed-up file remains untouched. No change is made to the user's GitHub remotely.

### If an error occurs

- `Could not open COM3`: close Serial Monitor with Ctrl+C; confirm COM port in Device Manager.
- `[NATIVE][ERROR]`: include the first error line and context. Errors may indicate a library compatibility issue with the local ESP32 Arduino core.
- `SUCCESS` but blank screen: code compiles/uploads, **not physically validated**; restore factory and do not assume a successful fix.

To restore the already-validated factory image, close Serial Monitor and — after **confirming your copied backup is exactly 16,777,216 bytes** — run:

```powershell
python -m esptool --chip esp32s3 --port COM3 --baud 115200 write-flash 0x0 .\stach-factory-backup.bin
```

Only use this command if the known factory backup copy is in the current working directory and passes size verification. Do not run blindly without the file.

## Validation limitations

Static source review and host-only mock C++ syntax checks can be run without ESP32 toolchain; they are not hardware or real PlatformIO validation. There is no guarantee this image will work on the physical board. Once native drawing works, migrate that display transport to STACH's eye renderer and separately diagnose I2C peripherals.
