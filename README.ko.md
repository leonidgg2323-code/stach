# STACH

[Русский](README.md) · [English](README.en.md) · [简体中文](README.zh-CN.md) · [繁體中文](README.zh-TW.md) · [Español](README.es.md) · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · [Français](README.fr.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Bahasa Indonesia](README.id.md) · [اردو](README.ur.md) · [Deutsch](README.de.md) · [日本語](README.ja.md) · **한국어** · [Türkçe](README.tr.md) · [Tiếng Việt](README.vi.md) · [Italiano](README.it.md) · [فارسی](README.fa.md) · [Polski](README.pl.md) · [Українська](README.uk.md)

STACH는 **Waveshare ESP32-S3-Touch-AMOLED-1.75** 를 주머니 속의 동반자로 바꿔줍니다. 눈은 손가락을 따라가고 기울어짐에 반응합니다. 오른쪽으로 스와이프하여 게임을 하거나 휴대폰을 사용하여 표정, 밝기, 동작, 표시 이름을 변경할 수 있습니다. 보드는 자체 Wi-Fi 네트워크와 제어판을 호스팅합니다.

![STACH eye animation](preview/stach-smart-eyes.gif)

**충전 애니메이션**

![충전 애니메이션](preview/stach-charging.gif)

## Waveshare 1.75에 설치

이 지침은 `stach-eyes-175` PlatformIO 환경을 사용하는 Windows에서 테스트되었습니다. 이는 라운드 466 × 466 CO5300 디스플레이, 16MB 플래시 및 8MB PSRAM 를 위해 `src/main.cpp` 에서 전체 애플리케이션을 구축합니다. 플래싱하기 전에 보드나 포장에 있는 모델을 확인하십시오. 1.43, 1.43B 및 1.75C 개정판에는 하드웨어가 다릅니다.

### 1. 컴퓨터를 준비하세요

첫 번째 빌드를 위해서는 `py` 런처가 포함된 Python 3.13, Windows용 Git, USB 데이터 케이블 및 인터넷 연결이 필요합니다. [윈도우용 파이썬](https://www.python.org/downloads/windows/) · [윈도우용 힘내](https://git-scm.com/downloads/win)

PowerShell을 열고 프로젝트를 새 폴더에 복제합니다. ASCII 문자가 포함된 짧은 경로는 ESP32 도구에서 잘 작동합니다.

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

또는 **코드 → GitHub에서 ZIP** 다운로드를 선택하고 아카이브를 추출합니다. `platformio.ini` 가 포함된 폴더에서 PowerShell을 열고 위의 마지막 두 명령을 실행합니다. 아래 명령은 가상 환경의 Python을 직접 호출합니다.

### 2. 종속성을 설치하고 빌드합니다.

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

첫 번째 빌드에서는 ESP32 도구 체인을 다운로드하며 몇 분 정도 걸릴 수 있습니다. 빌드는 Bluetooth 마우스 라이브러리 호환성 패치를 자동으로 적용합니다. 업로드하기 전에 `stach-eyes-175  SUCCESS` 를 기다리세요.

### 3. 직렬 포트를 찾아 보드를 플래시하세요.

보드를 USB로 연결하고 사용 가능한 포트를 나열합니다.

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

보드를 꽂을 때 나타나고, 빼면 사라지는 포트를 식별합니다. 예제의 `COM3` 를 해당 포트로 바꾸십시오. Arduino IDE 직렬 모니터를 포함하여 이를 사용하는 모든 프로그램을 닫습니다.

**업로드는 보드의 펌웨어를 교체합니다.** 공장 메뉴 복원 옵션을 유지하려면 업로드하기 전에 저장소 외부에 전체 플래시 백업을 저장하십시오. 이 모델의 전체 백업은 **16,777,216 bytes** 입니다. 이미 백업이 있는 경우 새로운 파일 이름을 사용하십시오. 이미지에는 설정 및 자격 증명이 포함될 수 있으므로 비공개로 유지하세요.

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

STACH를 설치하려면 업로드 명령을 실행하십시오.

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

체크섬 확인 및 `SUCCESS` 메시지 후에 보드가 다시 시작됩니다. PlatformIO 는 부트로더, 파티션 테이블 및 애플리케이션을 해당 주소에 기록합니다. 빌드 디렉터리의 `firmware.bin` 파일에는 애플리케이션만 포함되어 있습니다.

### 4. 소유자 계정 등록

1. 처음 부팅할 때 휴대폰으로 **1/2 JOIN WI-FI** 를 스캔하세요. `Stach-XXXXXX` 네트워크에 가입하세요. 휴대폰에서 인터넷에 접속할 수 없다고 보고하는 경우 **연결 유지** 를 선택합니다.
2. 전화기가 연결되면 디스플레이가 **2/2 REGISTER** 로 전환됩니다. 첫 번째 코드가 계속 표시되면 디스플레이를 한 번 탭하세요. 동일한 전화기로 두 번째 코드를 스캔하세요.
3. 3~32자(A~Z 또는 a~z), 숫자, 하이픈, 밑줄로 구성된 사용자 이름과 12자 이상의 비밀번호를 선택하세요. 로그인 정보를 저장하세요. 계정과 설정은 보드에 저장됩니다.
4. **Аккаунт владельца** (소유자 계정)에서는 패널에 표시되는 이름을 변경하거나 로그아웃할 수 있습니다. 나중에 방문하려면 보드의 Wi-Fi에 연결하고 [로컬 STACH 제어판](http://192.168.4.1/) 를 엽니다.

보드는 하나의 소유자 계정을 허용합니다. 등록하려면 실제 디스플레이에 표시된 일회성 코드가 필요합니다. 제어 명령에는 소유자 세션과 요청 토큰이 필요합니다. 패널은 보드의 로컬 Wi-Fi 네트워크에서 HTTP를 사용합니다. 보드가 다시 시작된 후 다시 로그인하세요.

## 제스처와 게임

| 액션 | 결과 |
|---|---|
| 눈을 만지거나 보드를 기울여보세요 | 눈은 손가락이나 기울기를 따릅니다. |
| 눈에서 오른쪽으로 스와이프 | 게임 메뉴를 엽니다. |
| 게임에서 왼쪽으로 스와이프 | 메뉴로 돌아갑니다. 다시 왼쪽으로 스와이프하면 눈으로 돌아갑니다. |
| 아이스크린을 약 1초 동안 유지하세요. | Wi-Fi QR 코드를 보여주세요. 네트워크와 패널 코드 사이를 전환하려면 탭하세요. 길게 누르면 눈으로 돌아갑니다. |
| 눈에서 위로 스와이프 | 블루투스 마우스 모드를 시작합니다. 컴퓨터의 **Stach Mouse** 와 페어링하고 보정을 위해 보드를 가만히 잡으세요. |
| 마우스 모드에서 화면을 터치하세요 | 탭하여 왼쪽 클릭하고, 두 번 탭하여 오른쪽 클릭하고, 스와이프하여 스크롤하고, 길게 눌러 종료하세요. |

**Targets** 에서는 20초 안에 최대한 많은 목표물을 공격하세요. **Sums** 에서 5개의 덧셈 질문에 답하세요. 두 게임 모두 점수를 표시하고 개인 최고 기록을 저장합니다. **Dice** 는 6면체 주사위를 굴리고, **Coin** 는 동전을 던집니다. 각 결과는 ESP32 하드웨어 난수 생성기에서 나옵니다. 새로운 굴림이나 뒤집기를 위해 애니메이션이 끝난 후 다시 탭하세요.

## 디스플레이 및 전원

전화기 패널에는 표정 선택, 시선 조이스틱, 깜박임 및 기울이기 제어, 눈 밝기, 눈이 스스로 움직이는 정도에 대한 설정이 포함되어 있습니다. 모양 설정은 다시 시작해도 유지됩니다. 시작 후 마우스 모드를 활성화할 수 있습니다.

1.75 보드에서 **AXP2101** 는 배터리 비율과 충전 상태를 보고합니다. 보드가 케이블 전원으로 실행되고 배터리가 없음을 감지하면 표시기에 `USB` 가 표시됩니다. 절전 기능을 활성화하면 45초 동안 활동이 없으면 눈 새로 고침 빈도가 떨어지고 5분 후에 화면이 꺼집니다. 보고된 충전량이 15% 이하인 경우 해당 간격은 10초와 2분으로 줄어듭니다. 깨우려면 화면을 터치하세요. 게임, QR 코드, 마우스 모드 및 활성 충전이 디스플레이를 깨운 상태로 유지합니다.

## 문제 해결 및 복구

| 증상 | 무엇을 시도해야합니까? |
|---|---|
| 새 COM 포트가 나타나지 않습니다. | USB 데이터 케이블과 다른 USB 포트를 사용해 본 후 `platformio device list` 를 다시 실행해 보세요. |
| 포트를 열 수 없거나 `Access denied` 를 보고합니다. | 직렬 모니터를 닫습니다. 보드를 다시 연결하고 COM 번호를 확인하세요. |
| 빌드가 실패함 | 현재 폴더에 `platformio.ini` 가 포함되어 있는지 확인한 다음 2단계의 두 명령을 모두 반복하세요. |
| 화면이 계속 비어있습니다 | 업로드 로그에서 보드 모델과 환경 이름을 확인하세요. 1.75 보드의 경우 성공 라인에 `stach-eyes-175` 가 포함되어야 합니다. |
| 전화기가 패널을 열 수 없습니다 | 아직 `Stach-XXXXXX` 에 있는지 확인하고 `http://192.168.4.1/` 를 엽니다. 처음 등록하려면 두 번째 QR 코드가 필요합니다. |

진단을 보려면 115200 보드에서 직렬 모니터를 엽니다.

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

하드웨어 초기화 보고서의 경우 `I` 를 보내고, I²C 장치 목록을 보려면 `S` 를 보내고, 5초 디스플레이 테스트의 경우 `D` 를 보냅니다. **Ctrl+C** 를 눌러 모니터를 닫습니다.

전체 백업을 동일한 보드에 복원하려면 먼저 모니터를 닫고 이미지 크기를 확인하세요. 다음 명령은 백업에 포함된 설정 및 계정 데이터도 복원합니다.

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**도움이 필요하신가요?** Discord 또는 [@def0kk (Telegram)](https://t.me/def0kk) 에서 `twiinkprostolox123` 에 문의하세요. 보드 모델, PlatformIO 환경 및 오류 메시지를 포함합니다. 비밀번호, 등록 QR 코드, 전체 플래시 백업을 비공개로 유지하세요.

## 프로젝트 이력 및 개발

- **9단계 — 파워.** 이전 1.43 프로파일은 GPIO4 의 ADC 전압을 측정하고 필터링합니다. 충전량을 추정하려면 특정 개정판의 회로도를 확인하고 측정된 배터리 전압에 대해 보정해야 합니다. USB 전원만으로는 충전이 확인되지 않습니다. 1.75 프로파일은 AXP2101을 직접 읽습니다.
- **10단계 — 모드 및 개인화.** 디스플레이에서 또는 `POST /api/mode` 를 통해 얼굴, Bluetooth 마우스, Wi-Fi QR 및 패널 QR 간을 전환합니다. 눈 밝기와 시선 활동 범위는 20%~100%입니다. BLE 마우스 제어에는 하드웨어 프로필이 필요합니다.
- **11단계 — 신뢰성과 보안.** 펌웨어는 HTTP 요청 출처를 확인하고, 요청 토큰으로 명령을 보호하고, 요청 속도를 제한하고, Wi-Fi 액세스 포인트를 복구합니다. 1.75 프로필에는 소유자 로그인도 필요합니다. [11단계 보안 참고사항](STAGE11-SECURITY.md)
- **12단계 — 빌드 및 진단.** GitHub Actions는 체크섬을 사용하여 코드와 패키지 빌드를 확인합니다. `GET /api/hardware` 는 소프트웨어 초기화 결과를 보고합니다. 디스플레이 출력, 터치 반응, 페어링 등은 물리적 보드에서 테스트할 수 있습니다. [빌드 및 릴리스](STAGE12-RELEASE.md) · [하드웨어 테스트](STAGE12-HARDWARE-TESTS.md) · [하드웨어 진단](STAGE2-HARDWARE-DIAGNOSTICS.md)

펌웨어 진입점은 `src/main.cpp` 입니다. 헤더는 `include/` 에 있고 제어판 소스는 `web/stach-panel.html` 입니다. 패널을 편집한 후 일치하도록 `preview/stach-panel.html` 를 업데이트한 다음 `python tools/build_web_header.py` 를 실행합니다. `python tools/check_panel.py` 를 실행하여 포함된 페이지와 API 계약을 확인하세요(Node.js 필요). Linux에서는 휴대용 코어 테스트를 위해 `sh tools/test_core.sh` 를 실행합니다. `python tools/test_ble_patch.py` 를 실행하여 BLE 패치 호환성을 확인하세요.

STACH는 Pip-KeyPet 장르에서 영감을 받은 독립 프로젝트입니다. PlatformIO 는 타사 라이브러리를 설치하고 프로젝트는 ESP32 BLE Mouse 0.3.1 에 호환성 패치를 적용합니다. [Waveshare 1.75 리소스](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) · [Waveshare 1.43 참조](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
