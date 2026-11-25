# STACH

[Русский](README.md) · [English](README.en.md) · [简体中文](README.zh-CN.md) · [繁體中文](README.zh-TW.md) · [Español](README.es.md) · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · [Français](README.fr.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Bahasa Indonesia](README.id.md) · [اردو](README.ur.md) · [Deutsch](README.de.md) · [日本語](README.ja.md) · [한국어](README.ko.md) · [Türkçe](README.tr.md) · **Tiếng Việt** · [Italiano](README.it.md) · [فارسی](README.fa.md) · [Polski](README.pl.md) · [Українська](README.uk.md)

STACH biến **Waveshare ESP32-S3-Touch-AMOLED-1.75** thành người bạn đồng hành bỏ túi. Đôi mắt của nó dõi theo ngón tay của bạn và phản ứng với độ nghiêng. Vuốt sang phải để chơi trò chơi hoặc sử dụng điện thoại của bạn để thay đổi biểu thức, độ sáng, hành vi và tên hiển thị. Bo mạch chủ lưu trữ mạng Wi-Fi và bảng điều khiển riêng.

![STACH eye animation](preview/stach-smart-eyes.gif)

## Cài đặt trên Waveshare 1.75

Các hướng dẫn này đã được thử nghiệm trên Windows với môi trường `stach-eyes-175` PlatformIO. Nó xây dựng ứng dụng đầy đủ từ `src/main.cpp` cho màn hình tròn 466 × 466 CO5300, 16 MB flash và 8 MB PSRAM. Kiểm tra mô hình trên bảng hoặc bao bì của nó trước khi nhấp nháy. Các phiên bản 1.43, 1.43B và 1.75C có phần cứng khác nhau.

### 1. Chuẩn bị máy tính của bạn

Bạn cần Python 3.13 với trình khởi chạy `py`, Git cho Windows, cáp dữ liệu USB và kết nối Internet cho bản dựng đầu tiên. [Python cho Windows](https://www.python.org/downloads/windows/) · [Git cho Windows](https://git-scm.com/downloads/win)

Mở PowerShell và sao chép dự án vào một thư mục mới. Một đường dẫn ngắn với các ký tự ASCII hoạt động tốt với các công cụ ESP32.

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

Ngoài ra, chọn Mã **→ Tải xuống ZIP** trên GitHub và giải nén kho lưu trữ. Mở PowerShell trong thư mục chứa `platformio.ini` và chạy hai lệnh cuối cùng ở trên. Các lệnh bên dưới gọi trực tiếp Python của môi trường ảo.

### 2. Cài đặt các phụ thuộc và xây dựng

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

Bản dựng đầu tiên tải xuống chuỗi công cụ ESP32 và có thể mất vài phút. Bản dựng tự động áp dụng bản vá tương thích thư viện chuột Bluetooth. Đợi `stach-eyes-175  SUCCESS` trước khi tải lên.

### 3. Tìm cổng nối tiếp và flash bảng

Kết nối bo mạch bằng USB và liệt kê các cổng có sẵn.

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

Xác định cổng xuất hiện khi bạn cắm bo mạch và biến mất khi bạn rút phích cắm. Thay thế `COM3` trong các ví dụ bằng cổng của bạn. Đóng bất kỳ chương trình nào đang sử dụng nó, bao gồm cả màn hình nối tiếp Arduino IDE.

**Tải lên sẽ thay thế chương trình cơ sở trên bo mạch.** Để giữ tùy chọn khôi phục menu gốc, hãy lưu bản sao lưu flash đầy đủ bên ngoài kho lưu trữ trước khi tải lên. Bản sao lưu hoàn chỉnh cho mô hình này là **16,777,216 bytes**. Sử dụng tên tệp mới nếu bạn đã có bản sao lưu. Hình ảnh có thể chứa cài đặt và thông tin xác thực, vì vậy hãy giữ nó ở chế độ riêng tư.

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

Để cài đặt STACH, hãy chạy lệnh tải lên.

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

Bảng khởi động lại sau khi kiểm tra tổng kiểm tra và thông báo `SUCCESS`. PlatformIO ghi bộ tải khởi động, bảng phân vùng và ứng dụng vào địa chỉ tương ứng của chúng. Tệp `firmware.bin` trong thư mục bản dựng chỉ chứa ứng dụng.

### 4. Đăng ký tài khoản chủ sở hữu

1. Trong lần khởi động đầu tiên, hãy quét **1/2 JOIN WI-FI** bằng điện thoại của bạn. Tham gia mạng `Stach-XXXXXX`. Nếu điện thoại của bạn báo rằng không có quyền truy cập internet, hãy chọn **Luôn kết nối**.
2. Màn hình chuyển sang **2/2 REGISTER** khi điện thoại kết nối. Nếu mã đầu tiên vẫn hiển thị, hãy chạm vào màn hình một lần. Quét mã thứ hai bằng cùng một điện thoại.
3. Chọn tên người dùng gồm 3–32 chữ cái (A–Z hoặc a–z), chữ số, dấu gạch ngang hoặc dấu gạch dưới và mật khẩu có ít nhất 12 ký tự. Lưu chi tiết đăng nhập của bạn. Tài khoản và cài đặt được lưu trữ trên bảng.
4. Trong **Аккаунт владельца** (Tài khoản chủ sở hữu), bạn có thể thay đổi tên hiển thị trong bảng hoặc đăng xuất. Đối với những lần truy cập sau, hãy kết nối Wi-Fi của bo mạch và mở [bảng điều khiển STACH cục bộ](http://192.168.4.1/).

Hội đồng chấp nhận một tài khoản chủ sở hữu. Việc đăng ký yêu cầu mã một lần hiển thị trên màn hình vật lý. Các lệnh điều khiển yêu cầu phiên chủ sở hữu và mã thông báo yêu cầu. Bảng điều khiển sử dụng HTTP trên mạng Wi-Fi cục bộ của bo mạch. Đăng nhập lại sau khi bảng khởi động lại.

## Cử chỉ và trò chơi

| hành động | kết quả |
|---|---|
| Chạm vào mắt hoặc nghiêng bảng | Mắt dõi theo ngón tay hoặc độ nghiêng của bạn. |
| Vuốt sang phải từ mắt | Mở menu trò chơi. |
| Vuốt sang trái trong trò chơi | Quay trở lại thực đơn. Vuốt sang trái một lần nữa để quay lại đôi mắt. |
| Giữ màn hình mắt trong khoảng một giây | Hiển thị mã QR Wi-Fi. Nhấn để chuyển đổi giữa mã mạng và mã bảng điều khiển. Giữ để quay trở lại mắt. |
| Vuốt lên từ mắt | Bắt đầu chế độ chuột Bluetooth. Ghép nối với **Stach Mouse** trên máy tính của bạn và giữ nguyên bo mạch để hiệu chỉnh. |
| Chạm vào màn hình ở chế độ chuột | Nhấn để nhấp chuột trái, nhấn đúp để nhấp chuột phải, vuốt để cuộn hoặc giữ để thoát. |

Trong **Targets**, tấn công càng nhiều mục tiêu càng tốt trong 20 giây. Trong **Sums**, trả lời năm câu hỏi bổ sung. Cả hai trò chơi đều hiển thị điểm số của bạn và tiết kiệm cá nhân của bạn tốt nhất. **Dice** tung xúc xắc sáu mặt và **Coin** tung đồng xu. Mỗi kết quả đến từ bộ tạo số ngẫu nhiên phần cứng ESP32. Nhấn lại sau khi hoạt ảnh kết thúc để thực hiện cuộn hoặc lật mới.

## Màn hình và sức mạnh

Bảng điều khiển của điện thoại bao gồm lựa chọn biểu cảm, cần điều khiển nhìn, điều khiển nhấp nháy và nghiêng, độ sáng của mắt cũng như cài đặt mức độ mắt tự di chuyển. Cài đặt giao diện vẫn tồn tại khi khởi động lại. Bạn có thể kích hoạt chế độ chuột sau khi khởi động.

Trên bảng 1.75, **AXP2101** báo cáo phần trăm pin và trạng thái sạc. Đèn báo hiển thị `USB` khi bo mạch chạy bằng nguồn cáp và không phát hiện thấy pin. Khi bật tính năng tiết kiệm năng lượng, tốc độ làm mới mắt sẽ giảm sau 45 giây không hoạt động và màn hình sẽ tắt sau 5 phút. Với mức sạc được báo cáo là 15% hoặc ít hơn, khoảng thời gian đó giảm xuống còn 10 giây và 2 phút. Chạm vào màn hình để đánh thức nó. Trò chơi, mã QR, chế độ chuột và tính năng sạc đang hoạt động giúp màn hình luôn hoạt động.

## Khắc phục sự cố và phục hồi

| triệu chứng | Thử cái gì |
|---|---|
| Không có cổng COM mới xuất hiện | Hãy thử cáp dữ liệu USB và một cổng USB khác, sau đó chạy lại `platformio device list`. |
| Không mở được cổng hoặc báo `Access denied` | Đóng màn hình nối tiếp. Kết nối lại bo mạch và kiểm tra số COM của nó. |
| Việc xây dựng thất bại | Kiểm tra xem thư mục hiện tại của bạn có chứa `platformio.ini` hay không, sau đó lặp lại cả hai lệnh ở bước 2. |
| Màn hình vẫn trống | Kiểm tra mẫu bo mạch và tên môi trường trong nhật ký tải lên. Đối với bảng 1.75, dòng thành công phải bao gồm `stach-eyes-175`. |
| Điện thoại không mở được bảng điều khiển | Kiểm tra xem nó có còn trên `Stach-XXXXXX` không và mở `http://192.168.4.1/`. Đăng ký lần đầu yêu cầu mã QR thứ hai. |

Để xem chẩn đoán, hãy mở màn hình nối tiếp ở tốc độ 115200 baud.

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

Gửi `I` để lấy báo cáo khởi tạo phần cứng, `S` để liệt kê các thiết bị I²C hoặc `D` để kiểm tra hiển thị trong 5 giây. Nhấn **Ctrl+C** để đóng màn hình.

Để khôi phục bản sao lưu đầy đủ của bạn vào cùng một bảng, hãy đóng màn hình và xác minh kích thước hình ảnh trước. Lệnh sau cũng khôi phục cài đặt và dữ liệu tài khoản có trong bản sao lưu.

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**Cần trợ giúp?** Liên hệ với `twiinkprostolox123` trên Discord hoặc [@def0kk (Telegram)](https://t.me/def0kk). Bao gồm mô hình bo mạch, môi trường PlatformIO và thông báo lỗi. Giữ mật khẩu, mã QR đăng ký và bản sao lưu flash đầy đủ của bạn ở chế độ riêng tư.

## Lịch sử và phát triển dự án

- **Giai đoạn 9 - Sức mạnh.** Cấu hình 1.43 cũ hơn đo và lọc điện áp ADC trên GPIO4. Việc ước tính mức sạc yêu cầu kiểm tra sơ đồ để sửa đổi cụ thể và hiệu chỉnh theo điện áp pin đo được. Chỉ riêng nguồn USB không xác nhận việc sạc. Cấu hình 1.75 đọc trực tiếp AXP2101.
- **Giai đoạn 10 - Các chế độ và cá nhân hóa.** Chuyển đổi giữa khuôn mặt, chuột Bluetooth, Wi-Fi QR và QR bảng điều khiển từ màn hình hoặc qua `POST /api/mode`. Độ sáng của mắt và hoạt động nhìn dao động từ 20% đến 100%. Điều khiển chuột BLE yêu cầu cấu hình phần cứng.
- **Giai đoạn 11 - Độ tin cậy và bảo mật.** Phần sụn kiểm tra nguồn gốc yêu cầu HTTP, bảo vệ các lệnh bằng mã thông báo yêu cầu, yêu cầu giới hạn tốc độ và khôi phục điểm truy cập Wi-Fi. Cấu hình 1.75 cũng yêu cầu chủ sở hữu đăng nhập. [Ghi chú bảo mật giai đoạn 11](STAGE11-SECURITY.md)
- **Giai đoạn 12 - Xây dựng và chẩn đoán.** GitHub Actions kiểm tra mã và các gói được xây dựng bằng tổng kiểm tra. `GET /api/hardware` báo cáo kết quả khởi tạo phần mềm. Đầu ra màn hình, phản hồi cảm ứng và ghép nối có thể được kiểm tra trên bảng vật lý. [Xây dựng và phát hành](STAGE12-RELEASE.md) · [Kiểm tra phần cứng](STAGE12-HARDWARE-TESTS.md) · [Chẩn đoán phần cứng](STAGE2-HARDWARE-DIAGNOSTICS.md)

Điểm vào chương trình cơ sở là `src/main.cpp`. Các tiêu đề nằm trong `include/` và nguồn của bảng điều khiển là `web/stach-panel.html`. Sau khi chỉnh sửa bảng điều khiển, hãy cập nhật `preview/stach-panel.html` cho phù hợp, sau đó chạy `python tools/build_web_header.py`. Chạy `python tools/check_panel.py` để kiểm tra trang được nhúng và hợp đồng API (yêu cầu Node.js). Trên Linux, chạy `sh tools/test_core.sh` để kiểm tra lõi di động. Chạy `python tools/test_ble_patch.py` để kiểm tra tính tương thích của bản vá BLE.

STACH là một dự án độc lập lấy cảm hứng từ thể loại Pip-KeyPet. PlatformIO cài đặt thư viện của bên thứ ba và dự án áp dụng bản vá tương thích cho ESP32 BLE Mouse 0.3.1. [Tài nguyên Waveshare 1.75](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) · [Tham khảo Waveshare 1.43](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
