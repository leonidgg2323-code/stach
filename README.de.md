# STACH

[Русский](README.md) · [English](README.en.md) · [简体中文](README.zh-CN.md) · [繁體中文](README.zh-TW.md) · [Español](README.es.md) · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · [Français](README.fr.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Bahasa Indonesia](README.id.md) · [اردو](README.ur.md) · **Deutsch** · [日本語](README.ja.md) · [한국어](README.ko.md) · [Türkçe](README.tr.md) · [Tiếng Việt](README.vi.md) · [Italiano](README.it.md) · [فارسی](README.fa.md) · [Polski](README.pl.md) · [Українська](README.uk.md)

STACH verwandelt einen **Waveshare ESP32-S3-Touch-AMOLED-1.75** in einen Taschenbegleiter. Seine Augen folgen Ihrem Finger und reagieren auf Neigung. Wischen Sie nach rechts, um Spiele zu spielen, oder verwenden Sie Ihr Telefon, um dessen Ausdruck, Helligkeit, Verhalten und Anzeigenamen zu ändern. Das Board beherbergt ein eigenes Wi-Fi-Netzwerk und ein eigenes Bedienfeld.

![STACH eye animation](preview/stach-smart-eyes.gif)

**Ladeanimation**

![Ladeanimation](preview/stach-charging.gif)

## Auf einem Waveshare 1.75 installieren

Diese Anweisungen wurden unter Windows mit der `stach-eyes-175` PlatformIO -Umgebung getestet. Es erstellt die vollständige Anwendung von `src/main.cpp` für das runde 466 × 466- CO5300 -Display, 16 MB Flash und 8 MB PSRAM. Überprüfen Sie vor dem Flashen das Modell auf der Platine oder der Verpackung. Die Revisionen 1.43, 1.43B und 1.75C verfügen über unterschiedliche Hardware.

### 1. Bereiten Sie Ihren Computer vor

Für den ersten Build benötigen Sie Python 3.13 mit dem `py` -Launcher, Git für Windows, ein USB-Datenkabel und eine Internetverbindung. [Python für Windows](https://www.python.org/downloads/windows/) · [Git für Windows](https://git-scm.com/downloads/win)

Öffnen Sie PowerShell und klonen Sie das Projekt in einen neuen Ordner. Ein kurzer Pfad mit ASCII-Zeichen funktioniert gut mit den ESP32-Tools.

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

Alternativ können Sie **Code → ZIP** auf GitHub herunterladen und das Archiv extrahieren. Öffnen Sie PowerShell im Ordner mit `platformio.ini` und führen Sie die letzten beiden Befehle oben aus. Die folgenden Befehle rufen Python der virtuellen Umgebung direkt auf.

### 2. Installieren Sie die Abhängigkeiten und erstellen Sie

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

Der erste Build lädt die ESP32-Toolchain herunter und kann mehrere Minuten dauern. Der Build wendet den Kompatibilitätspatch für die Bluetooth-Mausbibliothek automatisch an. Warten Sie vor dem Hochladen auf `stach-eyes-175  SUCCESS`.

### 3. Suchen Sie den seriellen Port und flashen Sie die Platine

Schließen Sie das Board per USB an und listen Sie die verfügbaren Ports auf.

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

Identifizieren Sie den Anschluss, der angezeigt wird, wenn Sie die Platine einstecken, und verschwindet, wenn Sie sie herausziehen. Ersetzen Sie `COM3` in den Beispielen durch Ihren Port. Schließen Sie alle Programme, die es verwenden, einschließlich des seriellen Arduino IDE-Monitors.

**Durch das Hochladen wird die Firmware auf der Platine ersetzt.** Um die Option zum Wiederherstellen des Werksmenüs beizubehalten, speichern Sie vor dem Hochladen ein vollständiges Flash-Backup außerhalb des Repositorys. Ein vollständiges Backup für dieses Modell ist **16,777,216 bytes**. Verwenden Sie einen neuen Dateinamen, wenn Sie bereits über ein Backup verfügen. Das Bild enthält möglicherweise Einstellungen und Anmeldeinformationen. Halten Sie es daher privat.

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

Um STACH zu installieren, führen Sie den Upload-Befehl aus.

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

Das Board startet nach der Prüfsummenprüfung und der `SUCCESS` -Meldung neu. PlatformIO schreibt den Bootloader, die Partitionstabelle und die Anwendung an ihre jeweiligen Adressen. Die `firmware.bin` -Datei im Build-Verzeichnis enthält nur die Anwendung.

### 4. Registrieren Sie das Eigentümerkonto

1. Scannen Sie **1/2 JOIN WI-FI** beim ersten Start mit Ihrem Telefon. Treten Sie dem `Stach-XXXXXX` -Netzwerk bei. Wenn Ihr Telefon meldet, dass es keinen Internetzugang hat, wählen Sie **Bleiben Sie in Verbindung**.
2. Die Anzeige wechselt zu **2/2 REGISTER**, wenn das Telefon eine Verbindung herstellt. Wenn der erste Code immer noch angezeigt wird, tippen Sie einmal auf das Display. Scannen Sie den zweiten Code mit demselben Telefon.
3. Wählen Sie einen Benutzernamen mit 3–32 Buchstaben (A–Z oder a–z), Ziffern, Bindestrichen oder Unterstrichen und ein Passwort mit mindestens 12 Zeichen. Speichern Sie Ihre Anmeldedaten. Das Konto und die Einstellungen werden auf dem Board gespeichert.
4. In **Аккаунт владельца** (Eigentümerkonto) können Sie den im Panel angezeigten Namen ändern oder sich abmelden. Für spätere Besuche verbinden Sie sich mit dem WLAN des Boards und öffnen Sie das [lokales STACH-Bedienfeld](http://192.168.4.1/).

Das Board akzeptiert ein Besitzerkonto. Für die Registrierung ist der auf dem physischen Display angezeigte Einmalcode erforderlich. Steuerbefehle erfordern eine Eigentümersitzung und ein Anforderungstoken. Das Panel verwendet HTTP im lokalen Wi-Fi-Netzwerk des Boards. Melden Sie sich nach dem Neustart des Boards erneut an.

## Gesten und Spiele

| Aktion | Ergebnis |
|---|---|
| Berühren Sie die Augen oder neigen Sie das Brett | Die Augen folgen Ihrem Finger oder der Neigung. |
| Wischen Sie von den Augen nach rechts | Öffnen Sie das Spielemenü. |
| Wischen Sie in einem Spiel nach links | Zurück zum Menü. Wischen Sie erneut nach links, um zu den Augen zurückzukehren. |
| Halten Sie den Augenschirm etwa eine Sekunde lang gedrückt | Zeigen Sie den WLAN-QR-Code an. Tippen Sie hier, um zwischen Netzwerk- und Bedienfeldcodes zu wechseln. Halten Sie gedrückt, um zu den Augen zurückzukehren. |
| Wischen Sie von den Augen nach oben | Starten Sie den Bluetooth-Mausmodus. Koppeln Sie es mit **Stach Mouse** auf Ihrem Computer und halten Sie die Platine zur Kalibrierung ruhig. |
| Berühren Sie den Bildschirm im Mausmodus | Tippen Sie, um mit der linken Maustaste zu klicken, doppeltippen Sie, um mit der rechten Maustaste zu klicken, wischen Sie, um zu scrollen, oder halten Sie gedrückt, um den Vorgang zu beenden. |

Triff in **Targets** so viele Ziele wie möglich in 20 Sekunden. Beantworten Sie in **Sums** fünf Zusatzfragen. Bei beiden Spielen wird Ihr Punktestand angezeigt und Ihre persönliche Bestleistung gespeichert. **Dice** würfelt mit einem sechsseitigen Würfel und **Coin** wirft eine Münze. Jedes Ergebnis stammt vom ESP32-Hardware-Zufallszahlengenerator. Tippen Sie erneut, nachdem die Animation beendet ist, um einen neuen Wurf oder Wurf durchzuführen.

## Anzeige und Leistung

Das Telefonbedienfeld umfasst eine Ausdrucksauswahl, einen Blick-Joystick, Blinzel- und Neigungssteuerung, Augenhelligkeit und eine Einstellung dafür, wie stark sich die Augen selbstständig bewegen. Die Darstellungseinstellungen bleiben auch nach Neustarts bestehen. Sie können den Mausmodus nach dem Start aktivieren.

Auf der 1,75-Karte meldet der **AXP2101** den Batterieprozentsatz und den Ladestatus. Die Anzeige zeigt `USB` an, wenn die Platine mit Kabelstrom betrieben wird und keine Batterie erkennt. Bei aktiviertem Energiesparmodus sinkt die Augenaktualisierungsrate nach 45 Sekunden Inaktivität und der Bildschirm schaltet sich nach 5 Minuten aus. Bei einer gemeldeten Ladung von 15 % oder weniger sinken diese Intervalle auf 10 Sekunden und 2 Minuten. Berühren Sie den Bildschirm, um ihn zu aktivieren. Spiele, QR-Codes, Mausmodus und aktives Laden halten das Display wach.

## Fehlerbehebung und Wiederherstellung

| Symptom | Was Sie versuchen sollten |
|---|---|
| Es erscheint kein neuer COM-Port | Versuchen Sie es mit einem USB-Datenkabel und einem anderen USB-Anschluss und führen Sie dann `platformio device list` erneut aus. |
| Der Port kann nicht geöffnet werden oder meldet `Access denied` | Schließen Sie den seriellen Monitor. Schließen Sie die Platine wieder an und überprüfen Sie ihre COM-Nummer. |
| Der Build schlägt fehl | Überprüfen Sie, ob Ihr aktueller Ordner `platformio.ini` enthält, und wiederholen Sie dann beide Befehle in Schritt 2. |
| Der Bildschirm bleibt leer | Überprüfen Sie das Board-Modell und den Umgebungsnamen im Upload-Protokoll. Für das 1,75-Board sollte die Erfolgszeile `stach-eyes-175` enthalten. |
| Das Telefon kann das Bedienfeld nicht öffnen | Überprüfen Sie, ob es sich noch auf `Stach-XXXXXX` befindet, und öffnen Sie `http://192.168.4.1/`. Bei der Erstregistrierung ist der zweite QR-Code erforderlich. |

Um Diagnosen anzuzeigen, öffnen Sie den seriellen Monitor mit 115200 Baud.

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

Senden Sie `I` für einen Hardware-Initialisierungsbericht, `S`, um I²C-Geräte aufzulisten, oder `D` für einen fünf Sekunden dauernden Anzeigetest. Drücken Sie **Ctrl+C**, um den Monitor zu schließen.

Um Ihr vollständiges Backup auf derselben Karte wiederherzustellen, schließen Sie zunächst den Monitor und überprüfen Sie die Bildgröße. Der folgende Befehl stellt auch die im Backup enthaltenen Einstellungen und Kontodaten wieder her.

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**Brauchen Sie Hilfe?** Kontaktieren Sie `twiinkprostolox123` auf Discord oder [@def0kk (Telegram)](https://t.me/def0kk). Geben Sie das Platinenmodell, die PlatformIO -Umgebung und die Fehlermeldung an. Halten Sie Ihr Passwort, Ihren Registrierungs-QR-Code und Ihr vollständiges Flash-Backup vertraulich.

## Projektgeschichte und Entwicklung

- **Stufe 9 – Leistung.** Die älteren 1.43-Profile messen und filtern die ADC-Spannung auf GPIO4. Um die Ladung abzuschätzen, muss der Schaltplan für die jeweilige Revision überprüft und anhand einer gemessenen Batteriespannung kalibriert werden. Die USB-Stromversorgung allein bestätigt den Ladevorgang nicht. Das 1,75-Profil liest den AXP2101 direkt.
- **Stufe 10 – Modi und Personalisierung.** Wechseln Sie zwischen Gesicht, Bluetooth-Maus, Wi-Fi-QR und Panel-QR über das Display oder über `POST /api/mode`. Augenhelligkeit und Blickaktivität liegen zwischen 20 % und 100 %. Für die BLE-Maussteuerung ist ein Hardwareprofil erforderlich.
- **Stufe 11 – Zuverlässigkeit und Sicherheit.** Die Firmware überprüft die Herkunft von HTTP-Anfragen, schützt Befehle mit einem Anfrage-Token, begrenzt Anfragen und stellt den WLAN-Zugangspunkt wieder her. Für das 1.75-Profil ist außerdem ein Eigentümer-Login erforderlich. [Sicherheitshinweise zu Stufe 11](STAGE11-SECURITY.md)
- **Stufe 12 – Builds und Diagnose.** GitHub Actions überprüft den Code und verpackt Builds mit Prüfsummen. `GET /api/hardware` meldet Software-Initialisierungsergebnisse. Displayausgabe, Berührungsreaktion und Kopplung können auf der physischen Platine getestet werden. [Erstellen und veröffentlichen](STAGE12-RELEASE.md) · [Hardwaretests](STAGE12-HARDWARE-TESTS.md) · [Hardwarediagnose](STAGE2-HARDWARE-DIAGNOSTICS.md)

Der Firmware-Einstiegspunkt ist `src/main.cpp`. Header befinden sich in `include/` und die Bedienfeldquelle ist `web/stach-panel.html`. Nachdem Sie das Panel bearbeitet haben, aktualisieren Sie `preview/stach-panel.html` entsprechend und führen Sie dann `python tools/build_web_header.py` aus. Führen Sie `python tools/check_panel.py` aus, um die eingebettete Seite und den API-Vertrag zu überprüfen (erfordert Node.js). Führen Sie unter Linux `sh tools/test_core.sh` für die tragbaren Kerntests aus. Führen Sie `python tools/test_ble_patch.py` aus, um die BLE-Patch-Kompatibilität zu überprüfen.

STACH ist ein unabhängiges Projekt, das vom Pip-KeyPet -Genre inspiriert ist. PlatformIO installiert Bibliotheken von Drittanbietern und das Projekt wendet einen Kompatibilitätspatch auf ESP32 BLE Mouse 0.3.1 an. [Waveshare 1.75 Ressourcen](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) · [Referenz zu Waveshare 1.43](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
