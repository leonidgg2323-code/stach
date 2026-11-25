# STACH

[Русский](README.md) · [English](README.en.md) · [简体中文](README.zh-CN.md) · [繁體中文](README.zh-TW.md) · [Español](README.es.md) · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · [Français](README.fr.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · **Bahasa Indonesia** · [اردو](README.ur.md) · [Deutsch](README.de.md) · [日本語](README.ja.md) · [한국어](README.ko.md) · [Türkçe](README.tr.md) · [Tiếng Việt](README.vi.md) · [Italiano](README.it.md) · [فارسی](README.fa.md) · [Polski](README.pl.md) · [Українська](README.uk.md)

STACH mengubah **Waveshare ESP32-S3-Touch-AMOLED-1.75** menjadi teman saku. Matanya mengikuti jari Anda dan merespons kemiringan. Geser ke kanan untuk bermain game, atau gunakan ponsel Anda untuk mengubah ekspresi, kecerahan, perilaku, dan nama tampilannya. Papan ini memiliki jaringan Wi-Fi dan panel kontrolnya sendiri.

![STACH eye animation](preview/stach-smart-eyes.gif)

## Instal pada Waveshare 1.75

Petunjuk ini diuji pada Windows dengan lingkungan `stach-eyes-175` PlatformIO. Ia membangun aplikasi lengkap dari `src/main.cpp` untuk layar bulat 466 × 466 CO5300, flash 16 MB, dan PSRAM 8 MB. Periksa model di papan atau kemasannya sebelum melakukan flashing. Revisi 1.43, 1.43B, dan 1.75C memiliki hardware yang berbeda.

### 1. Siapkan komputer Anda

Anda memerlukan Python 3.13 dengan peluncur `py`, Git untuk Windows, kabel data USB, dan koneksi internet untuk versi pertama. [Python untuk Windows](https://www.python.org/downloads/windows/) · [Git untuk Windows](https://git-scm.com/downloads/win)

Buka PowerShell dan klon proyek ke folder baru. Jalur pendek dengan karakter ASCII berfungsi baik dengan alat ESP32.

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

Alternatifnya, pilih Kode **→ Unduh ZIP** di GitHub dan ekstrak arsipnya. Buka PowerShell di folder yang berisi `platformio.ini` dan jalankan dua perintah terakhir di atas. Perintah di bawah ini memanggil Python lingkungan virtual secara langsung.

### 2. Instal dependensi dan bangun

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

Versi pertama mengunduh rantai alat ESP32 dan dapat memakan waktu beberapa menit. Build ini menerapkan patch kompatibilitas perpustakaan mouse Bluetooth secara otomatis. Tunggu `stach-eyes-175  SUCCESS` sebelum mengunggah.

### 3. Temukan port serial dan flash papannya

Hubungkan papan dengan USB dan daftarkan port yang tersedia.

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

Identifikasi port yang muncul saat Anda menyambungkan papan dan menghilang saat Anda mencabutnya. Ganti `COM3` dalam contoh dengan port Anda. Tutup semua program yang menggunakannya, termasuk monitor serial Arduino IDE.

Pengunggahan **menggantikan firmware di papan.** Untuk mempertahankan opsi mengembalikan menu pabrik, simpan cadangan flash lengkap di luar repositori sebelum mengunggah. Cadangan lengkap untuk model ini adalah **16,777,216 bytes**. Gunakan nama file baru jika Anda sudah memiliki cadangan. Gambar tersebut mungkin berisi pengaturan dan kredensial, jadi jaga kerahasiaannya.

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

Untuk menginstal STACH, jalankan perintah unggah.

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

Papan dimulai ulang setelah pemeriksaan checksum dan pesan `SUCCESS`. PlatformIO menulis bootloader, tabel partisi, dan aplikasi ke alamatnya masing-masing. File `firmware.bin` di direktori build hanya berisi aplikasi.

### 4. Daftarkan akun pemilik

1. Saat boot pertama, pindai **1/2 JOIN WI-FI** dengan ponsel Anda. Bergabunglah dengan jaringan `Stach-XXXXXX`. Jika ponsel Anda melaporkan tidak memiliki akses internet, pilih **Tetap terhubung**.
2. Layar beralih ke **2/2 REGISTER** saat telepon terhubung. Jika kode pertama masih muncul, ketuk tampilannya sekali. Pindai kode kedua dengan telepon yang sama.
3. Pilih nama pengguna yang terdiri dari 3–32 huruf (A–Z atau a–z), angka, tanda hubung, atau garis bawah, dan kata sandi minimal 12 karakter. Simpan detail login Anda. Akun dan pengaturan disimpan di papan.
4. Di **Аккаунт владельца** (Akun pemilik), Anda dapat mengubah nama yang ditampilkan di panel atau keluar. Untuk kunjungan selanjutnya, bergabunglah dengan Wi-Fi papan dan buka [panel kontrol STACH lokal](http://192.168.4.1/).

Dewan menerima satu akun pemilik. Pendaftaran memerlukan kode satu kali yang ditampilkan pada tampilan fisik. Perintah kontrol memerlukan sesi pemilik dan token permintaan. Panel menggunakan HTTP pada jaringan Wi-Fi lokal papan. Masuk lagi setelah papan dimulai ulang.

## Gerakan dan permainan

| Tindakan | Hasil |
|---|---|
| Sentuh mata atau miringkan papan | Mata mengikuti jari atau kemiringan Anda. |
| Geser ke kanan dari mata | Buka menu permainan. |
| Gesek ke kiri dalam permainan | Kembali ke menu. Geser ke kiri lagi untuk kembali ke mata. |
| Tahan layar mata sekitar satu detik | Tunjukkan kode QR Wi-Fi. Ketuk untuk beralih antara kode jaringan dan panel. Tahan untuk kembali ke mata. |
| Geser ke atas dari mata | Mulai mode tetikus Bluetooth. Pasangkan dengan **Stach Mouse** di komputer Anda dan tahan papan untuk kalibrasi. |
| Sentuh layar dalam mode mouse | Ketuk untuk mengeklik kiri, ketuk dua kali untuk mengeklik kanan, gesek untuk menggulir, atau tahan untuk keluar. |

Di **Targets**, capai target sebanyak yang Anda bisa dalam 20 detik. Di **Sums**, jawab lima pertanyaan tambahan. Kedua game tersebut menunjukkan skor Anda dan menyimpan yang terbaik pribadi Anda. **Dice** melempar sebuah dadu bersisi enam, dan **Coin** melempar sebuah koin. Setiap hasil berasal dari generator nomor acak perangkat keras ESP32. Ketuk lagi setelah animasi selesai untuk melakukan roll atau flip baru.

## Tampilan dan kekuatan

Panel telepon mencakup pemilihan ekspresi, joystick pandangan, kontrol kedipan dan kemiringan, kecerahan mata, dan pengaturan seberapa banyak mata bergerak sendiri. Pengaturan tampilan tetap ada saat dimulai ulang. Anda dapat mengaktifkan mode mouse setelah startup.

Pada papan 1,75, **AXP2101** melaporkan persentase baterai dan status pengisian daya. Indikator menunjukkan `USB` ketika board menggunakan daya kabel dan tidak mendeteksi baterai. Dengan mengaktifkan penghematan daya, kecepatan penyegaran mata turun setelah 45 detik tidak ada aktivitas dan layar mati setelah 5 menit. Dengan pengisian daya yang dilaporkan sebesar 15% atau kurang, interval tersebut turun menjadi 10 detik dan 2 menit. Sentuh layar untuk membangunkannya. Permainan, kode QR, mode mouse, dan pengisian daya aktif membuat tampilan tetap terjaga.

## Pemecahan masalah dan pemulihan

| Gejala | Apa yang harus dicoba |
|---|---|
| Tidak ada port COM baru yang muncul | Coba kabel data USB dan port USB lain, lalu jalankan kembali `platformio device list`. |
| Port tidak dapat dibuka atau melaporkan `Access denied` | Tutup monitor serial. Hubungkan kembali papan dan periksa nomor COM-nya. |
| Pembangunannya gagal | Periksa apakah folder Anda saat ini berisi `platformio.ini`, lalu ulangi kedua perintah di langkah 2. |
| Layar tetap kosong | Periksa model papan dan nama lingkungan di log unggahan. Untuk papan 1,75, garis sukses harus mencakup `stach-eyes-175`. |
| Ponsel tidak dapat membuka panel | Periksa apakah masih di `Stach-XXXXXX` dan buka `http://192.168.4.1/`. Pendaftaran pertama kali memerlukan kode QR kedua. |

Untuk melihat diagnostik, buka monitor serial pada 115200 baud.

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

Kirim `I` untuk laporan inisialisasi perangkat keras, `S` untuk mencantumkan perangkat I²C, atau `D` untuk pengujian tampilan lima detik. Tekan **Ctrl+C** untuk menutup monitor.

Untuk memulihkan cadangan lengkap Anda ke papan yang sama, tutup monitor dan verifikasi ukuran gambar terlebih dahulu. Perintah berikut juga memulihkan pengaturan dan data akun yang terdapat dalam cadangan.

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**Butuh bantuan?** Hubungi `twiinkprostolox123` di Discord atau [@def0kk (Telegram)](https://t.me/def0kk). Sertakan model papan, lingkungan PlatformIO, dan pesan kesalahan. Jaga kerahasiaan kata sandi, kode QR pendaftaran, dan cadangan flash lengkap Anda.

## Sejarah dan perkembangan proyek

- **Tahap 9 — Kekuatan.** Profil 1,43 yang lebih lama mengukur dan memfilter tegangan ADC pada GPIO4. Memperkirakan muatan memerlukan pemeriksaan skema untuk revisi spesifik dan kalibrasi terhadap tegangan baterai yang diukur. Daya USB saja tidak mengkonfirmasi pengisian daya. Profil 1,75 membaca AXP2101 secara langsung.
- **Tahap 10 — Mode dan personalisasi.** Beralih antara wajah, mouse Bluetooth, QR Wi-Fi, dan panel QR dari layar atau melalui `POST /api/mode`. Kecerahan mata dan aktivitas tatapan berkisar antara 20% hingga 100%. Kontrol mouse BLE memerlukan profil perangkat keras.
- **Tahap 11 — Keandalan dan keamanan.** Firmware memeriksa asal permintaan HTTP, melindungi perintah dengan token permintaan, membatasi permintaan, dan memulihkan titik akses Wi-Fi. Profil 1.75 juga memerlukan login pemilik. [Catatan keamanan tahap 11](STAGE11-SECURITY.md)
- **Tahap 12 — Pembuatan dan diagnostik.** GitHub Actions memeriksa kode dan paket yang dibuat dengan checksum. `GET /api/hardware` melaporkan hasil inisialisasi perangkat lunak. Output tampilan, respons sentuhan, dan pemasangan dapat diuji pada papan fisik. [Bangun dan lepaskan](STAGE12-RELEASE.md) · [Tes perangkat keras](STAGE12-HARDWARE-TESTS.md) · [Diagnostik perangkat keras](STAGE2-HARDWARE-DIAGNOSTICS.md)

Titik masuk firmware adalah `src/main.cpp`. Header ada di `include/`, dan sumber panel kontrolnya adalah `web/stach-panel.html`. Setelah mengedit panel, perbarui `preview/stach-panel.html` agar cocok, lalu jalankan `python tools/build_web_header.py`. Jalankan `python tools/check_panel.py` untuk memeriksa halaman tersemat dan kontrak API (memerlukan Node.js). Di Linux, jalankan `sh tools/test_core.sh` untuk pengujian inti portabel. Jalankan `python tools/test_ble_patch.py` untuk memeriksa kompatibilitas patch BLE.

STACH adalah proyek independen yang terinspirasi oleh genre Pip-KeyPet. PlatformIO menginstal perpustakaan pihak ketiga, dan proyek menerapkan patch kompatibilitas ke ESP32 BLE Mouse 0.3.1. [Sumber daya Waveshare 1,75](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) · [Referensi Waveshare 1.43](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
