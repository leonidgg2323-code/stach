# STACH

[Русский](README.md) · [English](README.en.md) · [简体中文](README.zh-CN.md) · [繁體中文](README.zh-TW.md) · [Español](README.es.md) · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · [Français](README.fr.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Bahasa Indonesia](README.id.md) · [اردو](README.ur.md) · [Deutsch](README.de.md) · [日本語](README.ja.md) · [한국어](README.ko.md) · [Türkçe](README.tr.md) · [Tiếng Việt](README.vi.md) · [Italiano](README.it.md) · [فارسی](README.fa.md) · **Polski** · [Українська](README.uk.md)

STACH zmienia **Waveshare ESP32-S3-Touch-AMOLED-1.75** w kieszonkowego towarzysza. Jego oczy podążają za Twoim palcem i reagują na przechylenie. Przesuń w prawo, aby zagrać w gry, lub użyj telefonu, aby zmienić jego wygląd, jasność, zachowanie i wyświetlaną nazwę. Płyta obsługuje własną sieć Wi-Fi i panel sterowania.

![STACH eye animation](preview/stach-smart-eyes.gif)

## Zainstaluj na Waveshare 1.75

Instrukcje te zostały przetestowane w systemie Windows ze środowiskiem `stach-eyes-175` PlatformIO. Tworzy pełną aplikację z `src/main.cpp` dla okrągłego wyświetlacza 466 × 466 CO5300, 16 MB pamięci flash i 8 MB PSRAM. Przed flashowaniem sprawdź model na płytce lub jego opakowanie. Wersje 1.43, 1.43B i 1.75C mają inny sprzęt.

### 1. Przygotuj swój komputer

Do pierwszej kompilacji potrzebujesz Pythona 3.13 z programem uruchamiającym `py`, Git dla Windows, kabla USB do transmisji danych i połączenia internetowego. [Python dla Windowsa](https://www.python.org/downloads/windows/) · [Git dla Windowsa](https://git-scm.com/downloads/win)

Otwórz PowerShell i sklonuj projekt do nowego folderu. Krótka ścieżka ze znakami ASCII dobrze współpracuje z narzędziami ESP32.

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

Alternatywnie wybierz Kod **→ Pobierz ZIP** na GitHub i rozpakuj archiwum. Otwórz PowerShell w folderze zawierającym `platformio.ini` i uruchom dwa ostatnie polecenia powyżej. Poniższe polecenia bezpośrednio wywołują Pythona w środowisku wirtualnym.

### 2. Zainstaluj zależności i skompiluj

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

Pierwsza kompilacja pobiera zestaw narzędzi ESP32 i może zająć kilka minut. Kompilacja automatycznie stosuje poprawkę zgodności biblioteki myszy Bluetooth. Poczekaj na `stach-eyes-175  SUCCESS` przed przesłaniem.

### 3. Znajdź port szeregowy i sflashuj płytkę

Podłącz płytkę przez USB i wyświetl listę dostępnych portów.

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

Zidentyfikuj port, który pojawia się po podłączeniu płytki i znika po jej odłączeniu. Zastąp `COM3` w przykładach swoim portem. Zamknij dowolny program, który go używa, w tym monitor szeregowy Arduino IDE.

**Wgranie zastępuje firmware na płycie.** Aby zachować możliwość przywrócenia menu fabrycznego, przed przesłaniem zapisz pełną kopię zapasową flash poza repozytorium. Kompletną kopią zapasową dla tego modelu jest **16,777,216 bytes**. Jeśli masz już kopię zapasową, użyj nowej nazwy pliku. Obraz może zawierać ustawienia i dane uwierzytelniające, więc zachowaj jego prywatność.

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

Aby zainstalować STACH, uruchom polecenie upload.

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

Płyta uruchamia się ponownie po sprawdzeniu sumy kontrolnej i pojawieniu się komunikatu `SUCCESS`. PlatformIO zapisuje program ładujący, tablicę partycji i aplikację pod ich odpowiednimi adresami. Plik `firmware.bin` w katalogu kompilacji zawiera tylko aplikację.

### 4. Zarejestruj konto właściciela

1. Przy pierwszym uruchomieniu przeskanuj telefonem **1/2 JOIN WI-FI**. Dołącz do sieci `Stach-XXXXXX`. Jeśli Twój telefon zgłasza brak dostępu do Internetu, wybierz **Pozostań w kontakcie**.
2. Po podłączeniu telefonu wyświetlacz przełącza się na **2/2 REGISTER**. Jeśli pierwszy kod nadal się wyświetla, dotknij raz wyświetlacza. Zeskanuj drugi kod tym samym telefonem.
3. Wybierz nazwę użytkownika składającą się z 3–32 liter (A–Z lub a–z), cyfr, łączników lub podkreśleń oraz hasła składającego się z co najmniej 12 znaków. Zapisz swoje dane logowania. Konto i ustawienia przechowywane są na tablicy.
4. W **Аккаунт владельца** (konto właściciela) możesz zmienić nazwę wyświetlaną w panelu lub się wylogować. W przypadku późniejszych wizyt połącz się z Wi-Fi płyty i otwórz [lokalny panel sterowania STACH](http://192.168.4.1/).

Zarząd akceptuje jedno konto właściciela. Rejestracja wymaga podania jednorazowego kodu widocznego na fizycznym wyświetlaczu. Polecenia sterujące wymagają sesji właściciela i tokenu żądania. Panel korzysta z protokołu HTTP w lokalnej sieci Wi-Fi płyty. Zaloguj się ponownie po ponownym uruchomieniu tablicy.

## Gesty i gry

| Akcja | Wynik |
|---|---|
| Dotknij oczu lub przechyl planszę | Oczy podążają za palcem lub jego pochyleniem. |
| Przesuń palcem w prawo od oczu | Otwórz menu gier. |
| Przesuń w lewo w grze | Wróć do menu. Przesuń ponownie w lewo, aby wrócić do oczu. |
| Przytrzymaj osłonę oka przez około sekundę | Pokaż kod QR Wi-Fi. Stuknij, aby przełączać się między kodami sieci i panelu. Przytrzymaj, aby powrócić do oczu. |
| Przesuń w górę od oczu | Uruchom tryb myszy Bluetooth. Sparuj z **Stach Mouse** na swoim komputerze i przytrzymaj płytkę nieruchomo w celu kalibracji. |
| Dotknij ekranu w trybie myszy | Stuknij, aby kliknąć lewym przyciskiem myszy, dwukrotnie dotknij, aby kliknąć prawym przyciskiem myszy, przesuń, aby przewinąć lub przytrzymaj, aby wyjść. |

W **Targets** uderz jak najwięcej celów w ciągu 20 sekund. W **Sums** odpowiedz na pięć dodatkowych pytań. Obie gry pokazują Twój wynik i zapisują Twój rekord życiowy. **Dice** rzuca sześciościenną kostką, a **Coin** rzuca monetą. Każdy wynik pochodzi ze sprzętowego generatora liczb losowych ESP32. Stuknij ponownie po zakończeniu animacji, aby wykonać nowy rzut lub przewrót.

## Wyświetlacz i zasilanie

Panel telefonu zawiera wybór wyrazu twarzy, joystick spojrzenia, elementy sterujące mruganiem i pochylaniem, jasność oczu oraz ustawienie zakresu samodzielnego poruszania oczami. Ustawienia wyglądu pozostają niezmienne po ponownym uruchomieniu. Możesz włączyć tryb myszy po uruchomieniu.

Na płycie 1.75 **AXP2101** raportuje procent baterii i stan ładowania. Wskaźnik pokazuje `USB`, gdy płyta działa na zasilaniu kablowym i nie wykrywa baterii. Po włączeniu oszczędzania energii częstotliwość odświeżania oczu spada po 45 sekundach bezczynności, a ekran wyłącza się po 5 minutach. Przy zgłoszonym naładowaniu wynoszącym 15% lub mniej odstępy te spadają do 10 sekund i 2 minut. Dotknij ekranu, aby go obudzić. Gry, kody QR, tryb myszy i aktywne ładowanie sprawiają, że wyświetlacz nie śpi.

## Rozwiązywanie problemów i odzyskiwanie

| Objaw | Czego spróbować |
|---|---|
| Nie pojawia się żaden nowy port COM | Wypróbuj kabel USB do transmisji danych i inny port USB, a następnie uruchom ponownie `platformio device list`. |
| Nie można otworzyć portu lub zgłasza `Access denied` | Zamknij monitor szeregowy. Podłącz ponownie płytkę i sprawdź jej numer COM. |
| Kompilacja kończy się niepowodzeniem | Sprawdź, czy bieżący folder zawiera `platformio.ini`, a następnie powtórz oba polecenia w kroku 2. |
| Ekran pozostaje pusty | Sprawdź model płyty i nazwę środowiska w dzienniku przesyłania. W przypadku płyty 1,75 linia sukcesu powinna obejmować `stach-eyes-175`. |
| Telefon nie może otworzyć panelu | Sprawdź, czy nadal znajduje się na `Stach-XXXXXX` i otwórz `http://192.168.4.1/`. Rejestracja po raz pierwszy wymaga drugiego kodu QR. |

Aby wyświetlić diagnostykę, otwórz monitor szeregowy przy szybkości 115200 bodów.

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

Wyślij `I`, aby uzyskać raport inicjalizacji sprzętu, `S`, aby wyświetlić listę urządzeń I²C, lub `D`, aby uzyskać pięciosekundowy test wyświetlacza. Naciśnij **Ctrl+C**, aby zamknąć monitor.

Aby przywrócić pełną kopię zapasową na tę samą płytkę, zamknij monitor i najpierw sprawdź rozmiar obrazu. Poniższe polecenie przywraca również ustawienia i dane konta zawarte w kopii zapasowej.

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**Potrzebujesz pomocy?** Skontaktuj się z `twiinkprostolox123` na Discordzie lub [@def0kk (Telegram)](https://t.me/def0kk). Dołącz model płyty, środowisko PlatformIO i komunikat o błędzie. Zachowaj prywatność swojego hasła, rejestracyjnego kodu QR i pełnej kopii zapasowej flash.

## Historia projektu i rozwój

- **Stopień 9 — Moc.** Starsze profile 1.43 mierzą i filtrują napięcie ADC na GPIO4. Oszacowanie naładowania wymaga sprawdzenia schematu pod kątem konkretnej wersji i kalibracji w oparciu o zmierzone napięcie akumulatora. Samo zasilanie USB nie potwierdza ładowania. Profil 1,75 odczytuje bezpośrednio AXP2101.
- **Etap 10 — Tryby i personalizacja.** Przełączaj między twarzą, myszą Bluetooth, Wi-Fi QR i panelem QR z poziomu wyświetlacza lub poprzez `POST /api/mode`. Jasność oczu i aktywność spojrzenia wahają się od 20% do 100%. Sterowanie myszą BLE wymaga profilu sprzętowego.
- **Etap 11 — Niezawodność i bezpieczeństwo.** Oprogramowanie sprzętowe sprawdza pochodzenie żądań HTTP, chroni polecenia za pomocą tokena żądania, ogranicza szybkość żądań i odzyskuje punkt dostępu Wi-Fi. Profil 1.75 wymaga również logowania właściciela. [Uwagi dotyczące bezpieczeństwa etapu 11](STAGE11-SECURITY.md)
- **Etap 12 — kompilacje i diagnostyka.** GitHub Actions sprawdza kompilację kodu i pakietów za pomocą sum kontrolnych. `GET /api/hardware` raportuje wyniki inicjalizacji oprogramowania. Wyjście wyświetlacza, reakcję na dotyk i parowanie można przetestować na płycie fizycznej. [Kompiluj i wypuszczaj](STAGE12-RELEASE.md) · [Testy sprzętu](STAGE12-HARDWARE-TESTS.md) · [Diagnostyka sprzętu](STAGE2-HARDWARE-DIAGNOSTICS.md)

Punkt wejścia oprogramowania sprzętowego to `src/main.cpp`. Nagłówki znajdują się w `include/`, a źródłem panelu sterowania jest `web/stach-panel.html`. Po edycji panelu zaktualizuj `preview/stach-panel.html`, aby pasował, a następnie uruchom `python tools/build_web_header.py`. Uruchom `python tools/check_panel.py`, aby sprawdzić osadzoną stronę i kontrakt API (wymaga Node.js). W systemie Linux uruchom `sh tools/test_core.sh`, aby wykonać przenośne testy rdzenia. Uruchom `python tools/test_ble_patch.py`, aby sprawdzić zgodność poprawki BLE.

STACH to niezależny projekt inspirowany gatunkiem Pip-KeyPet. PlatformIO instaluje biblioteki innych firm, a projekt stosuje poprawkę zgodności z ESP32 BLE Mouse 0.3.1. [Zasoby Waveshare 1.75](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) · [Odniesienie do Waveshare 1.43](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
