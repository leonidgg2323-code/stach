# STACH

[Русский](README.md) · [English](README.en.md) · [简体中文](README.zh-CN.md) · [繁體中文](README.zh-TW.md) · [Español](README.es.md) · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · [Français](README.fr.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Bahasa Indonesia](README.id.md) · [اردو](README.ur.md) · [Deutsch](README.de.md) · **日本語** · [한국어](README.ko.md) · [Türkçe](README.tr.md) · [Tiếng Việt](README.vi.md) · [Italiano](README.it.md) · [فارسی](README.fa.md) · [Polski](README.pl.md) · [Українська](README.uk.md)

STACH は **Waveshare ESP32-S3-Touch-AMOLED-1.75** をポケット コンパニオンに変えます。その目はあなたの指に従い、傾きに反応します。右にスワイプしてゲームをプレイしたり、携帯電話を使用して表情、明るさ、動作、表示名を変更したりできます。このボードは独自の Wi-Fi ネットワークとコントロール パネルをホストします。

![STACH eye animation](preview/stach-smart-eyes.gif)

**充電アニメーション**

![充電アニメーション](preview/stach-charging.gif)

## Waveshare 1.75 にインストールする

これらの手順は、 `stach-eyes-175` PlatformIO 環境の Windows でテストされました。これは、円形 466 × 466 CO5300 ディスプレイ、16 MB のフラッシュ、および 8 MB の PSRAM 用の完全なアプリケーションを `src/main.cpp` から構築します。フラッシュする前に、ボードまたはそのパッケージ上のモデルを確認してください。 1.43、1.43B、および 1.75C リビジョンにはハードウェアが異なります。

### 1. コンピューターを準備する

最初のビルドには、Python 3.13 と `py` ランチャー、Git for Windows、USB データ ケーブル、およびインターネット接続が必要です。 [Windows 用の Python](https://www.python.org/downloads/windows/) ・ [Windows 用の Git](https://git-scm.com/downloads/win)

PowerShell を開き、プロジェクトのクローンを新しいフォルダーに作成します。 ASCII 文字を含む短いパスは、ESP32 ツールで適切に機能します。

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

または、「 **コード」→「GitHub で ZIP** をダウンロード」を選択し、アーカイブを解凍します。 `platformio.ini` を含むフォルダーで PowerShell を開き、上記の最後の 2 つのコマンドを実行します。以下のコマンドは、仮想環境の Python を直接呼び出します。

### 2. 依存関係をインストールしてビルドする

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

最初のビルドでは ESP32 ツールチェーンがダウンロードされ、数分かかる場合があります。ビルドでは、Bluetooth マウス ライブラリの互換性パッチが自動的に適用されます。アップロードする前に、 `stach-eyes-175  SUCCESS` を待ってください。

### 3. シリアルポートを見つけてボードをフラッシュします

USB でボードを接続し、利用可能なポートをリストします。

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

ボードを接続すると表示され、取り外すと非表示になるポートを特定します。例の `COM3` を実際のポートに置き換えます。 Arduino IDE シリアル モニタなど、それを使用しているプログラムをすべて閉じます。

**アップロードすると、ボード上のファームウェアが置き換えられます。** 工場出荷時のメニューを復元するオプションを維持するには、アップロードする前に完全なフラッシュ バックアップをリポジトリの外部に保存します。このモデルの完全なバックアップは **16,777,216 bytes** です。すでにバックアップがある場合は、新しいファイル名を使用してください。画像には設定や認証情報が含まれる可能性があるため、非公開にしてください。

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

STACH をインストールするには、アップロード コマンドを実行します。

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

ボードは、チェックサム チェックと `SUCCESS` メッセージの後に再起動します。 PlatformIO は、ブートローダー、パーティション テーブル、アプリケーションをそれぞれのアドレスに書き込みます。ビルド ディレクトリ内の `firmware.bin` ファイルには、アプリケーションのみが含まれています。

### 4. 所有者アカウントを登録する

1. 最初の起動時に、携帯電話で **1/2 JOIN WI-FI** をスキャンします。 `Stach-XXXXXX` ネットワークに参加します。電話機がインターネットにアクセスできないと報告した場合は、[ **接続を維持する]** を選択します。
2. 電話が接続されると、表示が **2/2 REGISTER** に切り替わります。最初のコードがまだ表示されている場合は、ディスプレイを 1 回タップします。同じ電話で 2 番目のコードをスキャンします。
3. 3 ～ 32 文字 (A ～ Z または a ～ z)、数字、ハイフン、またはアンダースコアからなるユーザー名と、12 文字以上のパスワードを選択します。ログインの詳細を保存します。アカウントと設定はボードに保存されます。
4. **Аккаунт владельца** （オーナーアカウント）では、パネルに表示される名前の変更やサインアウトが可能です。後でアクセスするには、ボードの Wi-Fi に接続し、 [ローカルSTACHコントロールパネル](http://192.168.4.1/) を開きます。

ボードは 1 つの所有者アカウントを受け入れます。登録には、物理​​ディスプレイに表示されるワンタイム コードが必要です。制御コマンドには所有者セッションとリクエスト トークンが必要です。パネルはボードのローカル Wi-Fi ネットワークで HTTP を使用します。ボードが再起動したら、再度サインインします。

## ジェスチャーとゲーム

| アクション | 結果 |
|---|---|
| 目をタッチするかボードを傾けます | 目は指や傾きに追従します。 |
| 目から右にスワイプ | ゲームメニューを開きます。 |
| ゲーム内で左にスワイプ | メニューに戻ります。もう一度左にスワイプすると目に戻ります。 |
| アイスクリーンを約 1 秒間押し続けます | Wi-Fi QRコードを表示します。タップしてネットワーク コードとパネル コードを切り替えます。長押しすると目に戻ります。 |
| 目から上にスワイプ | Bluetooth マウス モードを開始します。コンピューター上で **Stach Mouse** とペアリングし、キャリブレーションのためにボードを静止させます。 |
| マウスモードで画面をタッチする | タップして左クリック、ダブルタップして右クリック、スワイプしてスクロール、長押しして終了します。 |

**Targets** では、20 秒以内にできるだけ多くのターゲットを攻撃します。 **Sums** で、5 つの追加の質問に答えます。どちらのゲームでもスコアが表示され、自己ベストが保存されます。 **Dice** は 6 面のサイコロを振り、 **Coin** はコインを投げます。各結果は、ESP32 ハードウェア乱数ジェネレーターから得られます。アニメーションが終了したらもう一度タップすると、新しいロールまたはフリップが表示されます。

## ディスプレイと電源

電話パネルには、表情の選択、視線ジョイスティック、まばたきと傾きのコントロール、目の明るさ、目が自動的に動く量の設定が含まれています。外観設定は再起動後も保持されます。起動後にマウスモードを有効にすることができます。

1.75 ボードでは、 **AXP2101** がバッテリーの割合と充電ステータスを報告します。ボードがケーブル電源で動作し、バッテリーが検出されない場合、インジケーターには `USB` が表示されます。省電力を有効にすると、非アクティブ状態が 45 秒続くと目のリフレッシュ レートが低下し、5 分後に画面がオフになります。報告されている充電量が 15% 以下の場合、これらの間隔は 10 秒と 2 分に短縮されます。画面をタッチしてスリープを解除します。ゲーム、QR コード、マウス モード、およびアクティブな充電により、ディスプレイはスリープ状態のままになります。

## トラブルシューティングとリカバリ

| 症状 | 何を試すか |
|---|---|
| 新しいCOMポートは表示されません | USB データ ケーブルと別の USB ポートを試してから、 `platformio device list` を再度実行します。 |
| ポートを開けないか、 `Access denied` が報告される | シリアルモニターを閉じます。ボードを再接続し、COM 番号を確認します。 |
| ビルドが失敗する | 現在のフォルダーに `platformio.ini` が含まれていることを確認し、手順 2 の両方のコマンドを繰り返します。 |
| 画面が空白のままになる | アップロードログでボードモデルと環境名を確認してください。 1.75 ボードの場合、成功ラインには `stach-eyes-175` が含まれている必要があります。 |
| 電話機がパネルを開けません | まだ `Stach-XXXXXX` 上にあることを確認し、 `http://192.168.4.1/` を開きます。初回登録には2つ目のQRコードが必要です。 |

診断を表示するには、115200 ボーでシリアル モニターを開きます。

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

ハードウェア初期化レポートの場合は `I`、I²C デバイスのリストの場合は `S`、5 秒間の表示テストの場合は `D` を送信します。 **Ctrl+C** を押してモニターを閉じます。

完全バックアップを同じボードに復元するには、まずモニターを閉じてイメージ サイズを確認します。次のコマンドは、バックアップに含まれる設定とアカウント データも復元します。

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**助けが必要ですか?** Discord または [@def0kk (Telegram)](https://t.me/def0kk) で `twiinkprostolox123` に連絡してください。ボードのモデル、 PlatformIO 環境、およびエラー メッセージが含まれます。パスワード、登録 QR コード、および完全なフラッシュ バックアップを非公開にしておきます。

## プロジェクトの歴史と開発

- **ステージ 9 — パワー。** 古い 1.43 プロファイルは、 GPIO4 の ADC 電圧を測定し、フィルタリングします。充電量を見積もるには、特定のリビジョンの回路図を確認し、測定されたバッテリー電圧に対して校正する必要があります。 USB 電源だけでは充電を確認できません。 1.75 プロファイルは AXP2101 を直接読み取ります。
- **ステージ 10 — モードとパーソナライゼーション。** ディスプレイまたは `POST /api/mode` 経由で、顔、Bluetooth マウス、Wi-Fi QR、およびパネル QR を切り替えます。目の明るさと注視活動の範囲は 20% から 100% です。 BLE マウス コントロールにはハードウェア プロファイルが必要です。
- **ステージ 11 — 信頼性とセキュリティ。** ファームウェアは、HTTP リクエストの発信元をチェックし、リクエスト トークンを使用してコマンドを保護し、リクエストのレートを制限し、Wi-Fi アクセス ポイントを回復します。 1.75 プロファイルには所有者のログインも必要です。 [ステージ 11 のセキュリティに関する注意事項](STAGE11-SECURITY.md)
- **ステージ 12 — ビルドと診断。** GitHub Actions は、チェックサムを使用してコードとパッケージのビルドをチェックします。 `GET /api/hardware` はソフトウェアの初期化結果を報告します。ディスプレイ出力、タッチ応答、ペアリングを物理ボード上でテストできます。 [ビルドしてリリースする](STAGE12-RELEASE.md) ・ [ハードウェアテスト](STAGE12-HARDWARE-TESTS.md) ・ [ハードウェア診断](STAGE2-HARDWARE-DIAGNOSTICS.md)

ファームウェアのエントリ ポイントは `src/main.cpp` です。ヘッダーは `include/` にあり、コントロール パネルのソースは `web/stach-panel.html` です。パネルを編集した後、 `preview/stach-panel.html` を更新して一致させ、 `python tools/build_web_header.py` を実行します。 `python tools/check_panel.py` を実行して、埋め込みページと API コントラクトを確認します (Node.js が必要)。 Linux では、ポータブル コア テストのために `sh tools/test_core.sh` を実行します。 `python tools/test_ble_patch.py` を実行して、BLE パッチの互換性を確認します。

STACH は、 Pip-KeyPet ジャンルからインスピレーションを得た独立したプロジェクトです。 PlatformIO はサードパーティのライブラリをインストールし、プロジェクトは互換性パッチを ESP32 BLE Mouse 0.3.1 に適用します。 [Waveshare 1.75 リソース](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) ・ [Waveshare 1.43 リファレンス](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
