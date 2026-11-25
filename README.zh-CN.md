# STACH

[Русский](README.md) · [English](README.en.md) · **简体中文** · [繁體中文](README.zh-TW.md) · [Español](README.es.md) · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · [Français](README.fr.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Bahasa Indonesia](README.id.md) · [اردو](README.ur.md) · [Deutsch](README.de.md) · [日本語](README.ja.md) · [한국어](README.ko.md) · [Türkçe](README.tr.md) · [Tiếng Việt](README.vi.md) · [Italiano](README.it.md) · [فارسی](README.fa.md) · [Polski](README.pl.md) · [Українська](README.uk.md)

STACH 将 **Waveshare ESP32-S3-Touch-AMOLED-1.75** 变成袖珍伴侣。它的眼睛跟随您的手指并对倾斜做出反应。向右滑动即可玩游戏，或使用手机更改其表情、亮度、行为和显示名称。该主板拥有自己的 Wi-Fi 网络和控制面板。

![STACH 眼睛动画](preview/stach-smart-eyes.gif)

**充电动画**

![充电动画](preview/stach-charging.gif)

## 在 Waveshare 1.75 上安装

这些指令在 Windows 上使用 `stach-eyes-175` PlatformIO 环境进行了测试。它从 `src/main.cpp` 构建完整的应用程序，用于圆形 466 × 466 CO5300 显示屏、16 MB 闪存和 8 MB PSRAM。刷机前请检查开发板或其包装上的型号。 1.43、1.43B 和 1.75C 修订版具有不同的硬件。

### 1. 准备你的电脑

首次构建需要带有 `py` 启动器的 Python 3.13、Windows 版 Git、USB 数据线和互联网连接。 [适用于 Windows 的 Python](https://www.python.org/downloads/windows/) · [适用于 Windows 的 Git](https://git-scm.com/downloads/win)

打开 PowerShell 并将项目克隆到新文件夹中。包含 ASCII 字符的短路径适用于 ESP32 工具。

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

也可以在 GitHub 上选择 **Code → Download ZIP** 并解压存档。在包含 `platformio.ini` 的文件夹中打开 PowerShell 并运行上面的最后两个命令。下面的命令直接调用虚拟环境的Python。

### 2. 安装依赖项并构建

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

第一个构建会下载 ESP32 工具链，可能需要几分钟的时间。该版本会自动应用蓝牙鼠标库兼容性补丁。等待 `stach-eyes-175  SUCCESS` 后再上传。

### 3. 找到串口并烧写板子

通过 USB 连接开发板并列出可用端口。

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

识别插入板时出现并拔出板时消失的端口。将示例中的 `COM3` 替换为您的端口。关闭所有使用它的程序，包括 Arduino IDE 串口监视器。

**上传替换板上的固件。** 要保留恢复出厂菜单的选项，请在上传之前在存储库外部保存完整闪存备份。该模型的完整备份是 **16,777,216 字节**。如果您已有备份，请使用新的文件名。该图像可能包含设置和凭据，因此请保密。

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

要安装 STACH，请运行上传命令。

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

校验和检查和 `SUCCESS` 消息后，板重新启动。 PlatformIO 将引导加载程序、分区表和应用程序写入各自的地址。构建目录中的 `firmware.bin` 文件仅包含应用程序。

### 4. 注册设备所有者账号

1. 首次启动时，用手机扫描 **1/2 JOIN WI-FI**。加入 `Stach-XXXXXX` 网络。如果您的手机报告无法访问互联网，请选择 **保持连接**。
2. 手机连接后，显示切换为 **2/2 REGISTER**。如果仍然显示第一个代码，请点击显示屏一次。使用同一部手机扫描第二个代码。
3. 选择由 3-32 个字母（A-Z 或 a-z）、数字、连字符或下划线组成的用户名，以及至少 12 个字符的密码。保存您的登录详细信息。帐户和设置存储在板上。
4. 在 **Аккаунт владельца** （所有者帐户）中，您可以更改面板中显示的名称或注销。如需稍后访问，请连接主板的 Wi-Fi 并打开 [本地 STACH 控制面板](http://192.168.4.1/)。

每块开发板只能注册一个所有者账号。注册需要物理显示器上显示的一次性代码。控制命令需要所有者会话和请求令牌。该面板在开发板的本地 Wi-Fi 网络上使用 HTTP。待开发板重启后再次登录。

## 手势和游戏

| 行动 | 结果 |
|---|---|
| 触摸眼睛或倾斜开发板 | 眼睛跟随您的手指或倾斜。 |
| 在眼睛界面向右滑动 | 打开游戏菜单。 |
| 在游戏中向左滑动 | 返回菜单。再次向左滑动即可返回眼睛。 |
| 按住眼睛屏幕约一秒钟 | 显示 Wi-Fi 二维码。点击可在网络代码和面板代码之间切换。按住即可返回眼睛。 |
| 在眼睛界面向上滑动 | 启动蓝牙鼠标模式。与计算机上的 **Stach Mouse** 配对并保持电路板静止以进行校准。 |
| 在鼠标模式下触摸屏幕 | 点击左键单击、双击右键单击、滑动滚动或按住退出。 |

在 **Targets** 中，在 20 秒内击中尽可能多的目标。在 **Sums** 中，回答五道加法题。这两款游戏都会显示您的分数并保存您的个人最好成绩。 **Dice** 掷六面骰子， **Coin** 掷硬币。每个结果都来自 ESP32 硬件随机数生成器。动画完成后再次点击即可进行再次掷骰子或抛硬币。

## 显示及电源

手机面板包括表情选择、注视操纵杆、眨眼和倾斜控制、眼睛亮度以及眼睛自行移动量的设置。外观设置在重新启动后仍然保留。您可以在启动后启用鼠标模式。

在 1.75 板上， **AXP2101** 报告电池百分比和充电状态。当开发板使用电缆电源运行且检测到没有电池时，指示灯显示 `USB`。启用省电功能后，眼睛刷新率会在 45 秒不活动后下降，屏幕会在 5 分钟后关闭。当检测到的电量为 15% 或更少时，这些间隔会降至 10 秒和 2 分钟。触摸屏幕将其唤醒。游戏、二维码、鼠标模式和主动充电使显示屏保持唤醒状态。

## 故障排除和恢复

| 症状 | 处理方法 |
|---|---|
| 没有出现新的COM端口 | 尝试使用 USB 数据线和另一个 USB 端口，然后再次运行 `platformio device list`。 |
| 端口打不开或者报 `Access denied` | 关闭串口监视器。重新连接板并检查其 COM 号。 |
| 构建失败 | 检查当前文件夹是否包含 `platformio.ini`，然后重复步骤 2 中的两个命令。 |
| 屏幕保持空白 | 检查上传日志中的开发板型号和环境名称。对于 1.75 板，成功行应包括 `stach-eyes-175`。 |
| 手机打不开面板 | 检查它是否仍在 `Stach-XXXXXX` 上并打开 `http://192.168.4.1/`。首次注册需要第二个二维码。 |

要查看诊断信息，请以 115200 波特率打开串口监视器。

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

发送 `I` 获取硬件初始化报告，发送 `S` 列出 I²C 设备，或发送 `D` 进行五秒显示测试。按 **Ctrl+C** 关闭监视器。

要将完整备份恢复到原开发板，请先关闭串口监视器，并核对备份文件大小。以下命令还会恢复备份中包含的设置和帐户数据。

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**需要帮助吗？** 在 Discord 上联系 `twiinkprostolox123` 或 [@def0kk (Telegram)](https://t.me/def0kk)。包括开发板型号、 PlatformIO 环境以及错误信息。请勿公开密码、注册二维码或完整的闪存备份。

## 项目历史及发展

- **第 9 阶段 — 功率。** 较旧的 1.43 配置文件测量和过滤 GPIO4 上的 ADC 电压。估算电量需要检查原理图的具体版本并根据测量的电池电压进行校准。仅 接入 USB 电源不代表电池正在充电。 1.75配置文件直接读取AXP2101。
- **第 10 阶段 — 模式和个性化。** 从显示屏或通过 `POST /api/mode` 在面部、蓝牙鼠标、Wi-Fi QR 和面板 QR 之间切换。眼睛亮度和注视活动范围从 20% 到 100%。 BLE 鼠标控制需要硬件配置文件。
- **第 11 阶段 — 可靠性和安全性。** 固件检查 HTTP 请求来源、使用请求令牌保护命令、限制请求速率并恢复 Wi-Fi 接入点。 1.75 配置文件还需要所有者登录。 [第 11 阶段安全说明](STAGE11-SECURITY.md)
- **第 12 阶段 — 构建和诊断。** GitHub Actions 检查代码，并为打包后的构建产物生成校验和。 `GET /api/hardware` 报告软件初始化结果。显示输出、触摸响应和配对可以在物理板上进行测试。 [构建和发布](STAGE12-RELEASE.md) · [硬件测试](STAGE12-HARDWARE-TESTS.md) · [硬件诊断](STAGE2-HARDWARE-DIAGNOSTICS.md)

固件入口点是 `src/main.cpp`。标头位于 `include/`，控制面板源为 `web/stach-panel.html`。编辑面板后，更新 `preview/stach-panel.html` 以匹配，然后运行  `python tools/build_web_header.py`。运行 `python tools/check_panel.py` 来检查嵌入的页面和API契约（需要Node.js）。在 Linux 上，运行 `sh tools/test_core.sh` 进行可移植核心测试。运行 `python tools/test_ble_patch.py` 检查BLE补丁兼容性。

STACH 是一个独立项目，灵感来自 Pip-KeyPet 流派。 PlatformIO 安装第三方库，项目对 ESP32 BLE Mouse 0.3.1 应用了兼容性补丁。 [Waveshare 1.75 资源](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) · [Waveshare 1.43参考](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
