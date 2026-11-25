# STACH

[Русский](README.md) · **English** · [简体中文](README.zh-CN.md) · [繁體中文](README.zh-TW.md) · [Español](README.es.md) · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · [Français](README.fr.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Bahasa Indonesia](README.id.md) · [اردو](README.ur.md) · [Deutsch](README.de.md) · [日本語](README.ja.md) · [한국어](README.ko.md) · [Türkçe](README.tr.md) · [Tiếng Việt](README.vi.md) · [Italiano](README.it.md) · [فارسی](README.fa.md) · [Polski](README.pl.md) · [Українська](README.uk.md)

STACH turns a **Waveshare ESP32-S3-Touch-AMOLED-1.75** into a pocket companion. Its eyes follow your finger and respond to tilt. Swipe right to play games, or use your phone to change its expression, brightness, behavior, and display name. The board hosts its own Wi-Fi network and control panel.

![STACH eye animation](preview/stach-smart-eyes.gif)

**Charging animation**

![Charging animation](preview/stach-charging.gif)

## Install on a Waveshare 1.75

These instructions were tested on Windows with the `stach-eyes-175` PlatformIO environment. It builds the full application from `src/main.cpp` for the round 466 × 466 CO5300 display, 16 MB of flash, and 8 MB of PSRAM. Check the model on the board or its packaging before flashing. The 1.43, 1.43B, and 1.75C revisions have different hardware.

### 1. Prepare your computer

You need Python 3.13 with the `py` launcher, Git for Windows, a USB data cable, and an internet connection for the first build. [Python for Windows](https://www.python.org/downloads/windows/) · [Git for Windows](https://git-scm.com/downloads/win)

Open PowerShell and clone the project into a new folder. A short path with ASCII characters works well with the ESP32 tools.

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

Alternatively, choose **Code → Download ZIP** on GitHub and extract the archive. Open PowerShell in the folder containing `platformio.ini` and run the last two commands above. The commands below call the virtual environment’s Python directly.

### 2. Install the dependencies and build

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

The first build downloads the ESP32 toolchain and can take several minutes. The build applies the Bluetooth mouse library compatibility patch automatically. Wait for `stach-eyes-175  SUCCESS` before uploading.

### 3. Find the serial port and flash the board

Connect the board by USB and list the available ports.

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

Identify the port that appears when you plug the board in and disappears when you unplug it. Replace `COM3` in the examples with your port. Close any program using it, including the Arduino IDE serial monitor.

**Uploading replaces the firmware on the board.** To keep the option of restoring the factory menu, save a full flash backup outside the repository before uploading. A complete backup for this model is **16,777,216 bytes**. Use a fresh filename if you already have a backup. The image may contain settings and credentials, so keep it private.

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

To install STACH, run the upload command.

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

The board restarts after the checksum check and `SUCCESS` message. PlatformIO writes the bootloader, partition table, and application to their respective addresses. The `firmware.bin` file in the build directory contains only the application.

### 4. Register the owner account

1. On first boot, scan **1/2 JOIN WI-FI** with your phone. Join the `Stach-XXXXXX` network. If your phone reports that it has no internet access, choose **Stay connected**.
2. The display switches to **2/2 REGISTER** when the phone connects. If the first code is still showing, tap the display once. Scan the second code with the same phone.
3. Choose a username of 3–32 letters (A–Z or a–z), digits, hyphens, or underscores, and a password of at least 12 characters. Save your login details. The account and settings are stored on the board.
4. In **Аккаунт владельца** (Owner account), you can change the name shown in the panel or sign out. For later visits, join the board’s Wi-Fi and open the [local STACH control panel](http://192.168.4.1/).

The board accepts one owner account. Registration requires the one-time code shown on the physical display. Control commands require an owner session and a request token. The panel uses HTTP on the board’s local Wi-Fi network. Sign in again after the board restarts.

## Gestures and games

| Action | Result |
|---|---|
| Touch the eyes or tilt the board | The eyes follow your finger or the tilt. |
| Swipe right from the eyes | Open the games menu. |
| Swipe left in a game | Return to the menu. Swipe left again to return to the eyes. |
| Hold the eye screen for about a second | Show the Wi-Fi QR code. Tap to switch between the network and panel codes. Hold to return to the eyes. |
| Swipe up from the eyes | Start Bluetooth mouse mode. Pair with **Stach Mouse** on your computer and hold the board still for calibration. |
| Touch the screen in mouse mode | Tap to left-click, double-tap to right-click, swipe to scroll, or hold to exit. |

In **Targets**, hit as many targets as you can in 20 seconds. In **Sums**, answer five addition questions. Both games show your score and save your personal best. **Dice** rolls a six-sided die, and **Coin** flips a coin. Each result comes from the ESP32 hardware random number generator. Tap again after the animation finishes for a new roll or flip.

## Display and power

The phone panel includes expression selection, a gaze joystick, blink and tilt controls, eye brightness, and a setting for how much the eyes move on their own. Appearance settings persist across restarts. You can enable mouse mode after startup.

On the 1.75 board, the **AXP2101** reports battery percentage and charging status. The indicator shows `USB` when the board runs on cable power and detects no battery. With power saving enabled, the eye refresh rate drops after 45 seconds of inactivity and the screen turns off after 5 minutes. At a reported charge of 15% or less, those intervals drop to 10 seconds and 2 minutes. Touch the screen to wake it. Games, QR codes, mouse mode, and active charging keep the display awake.

## Troubleshooting and recovery

| Symptom | What to try |
|---|---|
| No new COM port appears | Try a USB data cable and another USB port, then run `platformio device list` again. |
| The port cannot be opened or reports `Access denied` | Close the serial monitor. Reconnect the board and check its COM number. |
| The build fails | Check that your current folder contains `platformio.ini`, then repeat both commands in step 2. |
| The screen stays blank | Check the board model and the environment name in the upload log. For the 1.75 board, the success line should include `stach-eyes-175`. |
| The phone cannot open the panel | Check that it is still on `Stach-XXXXXX` and open `http://192.168.4.1/`. First-time registration requires the second QR code. |

To view diagnostics, open the serial monitor at 115200 baud.

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

Send `I` for a hardware initialization report, `S` to list I²C devices, or `D` for a five-second display test. Press **Ctrl+C** to close the monitor.

To restore your full backup to the same board, close the monitor and verify the image size first. The following command also restores the settings and account data contained in the backup.

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**Need help?** Contact `twiinkprostolox123` on Discord or [@def0kk on Telegram](https://t.me/def0kk). Include the board model, the PlatformIO environment, and the error message. Keep your password, registration QR code, and full flash backup private.

## Project history and development

- **Stage 9 — Power.** The older 1.43 profiles measure and filter the ADC voltage on GPIO4. Estimating charge requires checking the schematic for the specific revision and calibrating against a measured battery voltage. USB power alone does not confirm charging. The 1.75 profile reads the AXP2101 directly.
- **Stage 10 — Modes and personalization.** Switch between the face, Bluetooth mouse, Wi-Fi QR, and panel QR from the display or via `POST /api/mode`. Eye brightness and gaze activity range from 20% to 100%. BLE mouse control requires a hardware profile.
- **Stage 11 — Reliability and security.** The firmware checks HTTP request origins, protects commands with a request token, rate-limits requests, and recovers the Wi-Fi access point. The 1.75 profile also requires an owner login. [Stage 11 security notes](STAGE11-SECURITY.md)
- **Stage 12 — Builds and diagnostics.** GitHub Actions checks the code and packages builds with checksums. `GET /api/hardware` reports software initialization results. Display output, touch response, and pairing can be tested on the physical board. [Build and release](STAGE12-RELEASE.md) · [Hardware tests](STAGE12-HARDWARE-TESTS.md) · [Hardware diagnostics](STAGE2-HARDWARE-DIAGNOSTICS.md)

The firmware entry point is `src/main.cpp`. Headers live in `include/`, and the control panel source is `web/stach-panel.html`. After editing the panel, update `preview/stach-panel.html` to match, then run `python tools/build_web_header.py`. Run `python tools/check_panel.py` to check the embedded page and API contract (requires Node.js). On Linux, run `sh tools/test_core.sh` for the portable core tests. Run `python tools/test_ble_patch.py` to check BLE patch compatibility.

STACH is an independent project inspired by the Pip-KeyPet genre. PlatformIO installs third-party libraries, and the project applies a compatibility patch to ESP32 BLE Mouse 0.3.1. [Waveshare 1.75 resources](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) · [Waveshare 1.43 reference](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
