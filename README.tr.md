# STACH

[Русский](README.md) · [English](README.en.md) · [简体中文](README.zh-CN.md) · [繁體中文](README.zh-TW.md) · [Español](README.es.md) · [हिन्दी](README.hi.md) · [العربية](README.ar.md) · [Français](README.fr.md) · [বাংলা](README.bn.md) · [Português](README.pt.md) · [Bahasa Indonesia](README.id.md) · [اردو](README.ur.md) · [Deutsch](README.de.md) · [日本語](README.ja.md) · [한국어](README.ko.md) · **Türkçe** · [Tiếng Việt](README.vi.md) · [Italiano](README.it.md) · [فارسی](README.fa.md) · [Polski](README.pl.md) · [Українська](README.uk.md)

STAC, **Waveshare ESP32-S3-Touch-AMOLED-1.75** 'yi cep arkadaşına dönüştürür. Gözleri parmağınızı takip eder ve eğime tepki verir. Oyun oynamak için sağa kaydırın veya ifadesini, parlaklığını, davranışını ve görünen adını değiştirmek için telefonunuzu kullanın. Kart kendi Wi-Fi ağına ve kontrol paneline sahiptir.

![STACH eye animation](preview/stach-smart-eyes.gif)

## Waveshare 1.75'e yükleyin

Bu talimatlar Windows'ta `stach-eyes-175` PlatformIO ortamıyla test edilmiştir. Yuvarlak 466 × 466 CO5300 ekran, 16 MB flash ve 8 MB PSRAM için `src/main.cpp` 'den tam uygulamayı oluşturur. Yanıp sönmeden önce karttaki modeli veya ambalajını kontrol edin. 1.43, 1.43B ve 1.75C revizyonları farklı donanımlara sahiptir.

### 1. Bilgisayarınızı hazırlayın

İlk derleme için `py` başlatıcısı, Windows için Git, bir USB veri kablosu ve bir internet bağlantısı ile Python 3.13'e ihtiyacınız var. [Windows için Python](https://www.python.org/downloads/windows/) · [Windows için Git](https://git-scm.com/downloads/win)

PowerShell'i açın ve projeyi yeni bir klasöre kopyalayın. ASCII karakterleri içeren kısa bir yol, ESP32 araçlarıyla iyi çalışır.

```powershell
cd $env:USERPROFILE
git clone https://github.com/leonidgg2323-code/stach.git stach-firmware
cd stach-firmware
py -3.13 -m venv .venv
.\.venv\Scripts\python.exe -m pip install "platformio==6.2.0"
```

Alternatif olarak GitHub'da **Code → ZIP ZIP** 'yi İndir seçeneğini seçin ve arşivi çıkarın. `platformio.ini` 'yi içeren klasörde PowerShell'i açın ve yukarıdaki son iki komutu çalıştırın. Aşağıdaki komutlar doğrudan sanal ortamın Python'unu çağırır.

### 2. Bağımlılıkları yükleyin ve oluşturun

```powershell
.\.venv\Scripts\python.exe -m platformio pkg install -e stach-eyes-175
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175
```

İlk derleme ESP32 araç zincirini indirir ve birkaç dakika sürebilir. Yapı, Bluetooth fare kitaplığı uyumluluk düzeltme ekini otomatik olarak uygular. Yüklemeden önce `stach-eyes-175  SUCCESS` 'yi bekleyin.

### 3. Seri bağlantı noktasını bulun ve kartı flaşlayın

Kartı USB ile bağlayın ve mevcut bağlantı noktalarını listeleyin.

```powershell
.\.venv\Scripts\python.exe -m platformio device list
```

Anakartı taktığınızda görünen ve fişi çıkardığınızda kaybolan bağlantı noktasını tanımlayın. Örneklerdeki `COM3` 'yi bağlantı noktanızla değiştirin. Arduino IDE seri monitörü dahil, onu kullanan herhangi bir programı kapatın.

**Yükleme, karttaki bellenimin yerini alır.** Fabrika menüsünü geri yükleme seçeneğini korumak için, yüklemeden önce tam flaş yedeklemesini deponun dışına kaydedin. Bu model için tam bir yedek **16,777,216 bytes** 'dir. Zaten bir yedeğiniz varsa yeni bir dosya adı kullanın. Resimde ayarlar ve kimlik bilgileri bulunabilir; bu nedenle onu gizli tutun.

```powershell
.\.venv\Scripts\python.exe -m pip install "esptool>=5,<6"
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 read-flash 0 ALL "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

STACH'ı yüklemek için yükleme komutunu çalıştırın.

```powershell
.\.venv\Scripts\python.exe -m platformio run -e stach-eyes-175 -t upload --upload-port COM3
```

Sağlama toplamı kontrolünden ve `SUCCESS` mesajından sonra kart yeniden başlatılır. PlatformIO, önyükleyiciyi, bölüm tablosunu ve uygulamayı ilgili adreslerine yazar. Derleme dizinindeki `firmware.bin` dosyası yalnızca uygulamayı içerir.

### 4. Sahip hesabını kaydedin

1. İlk açılışta telefonunuzla **1/2 JOIN WI-FI** dosyasını tarayın. `Stach-XXXXXX` ağına katılın. Telefonunuz internet erişiminin olmadığını bildiriyorsa **Bağlı kalın** öğesini seçin.
2. Telefon bağlandığında ekran **2/2 REGISTER** olarak değişir. İlk kod hâlâ gösteriliyorsa ekrana bir kez dokunun. İkinci kodu aynı telefonla tarayın.
3. 3-32 harften (A-Z veya a-z), rakamlardan, kısa çizgilerden veya alt çizgilerden oluşan bir kullanıcı adı ve en az 12 karakterden oluşan bir şifre seçin. Giriş bilgilerinizi kaydedin. Hesap ve ayarlar panoda saklanır.
4. **Аккаунт владельца** 'de (Sahip hesabı), panelde gösterilen adı değiştirebilir veya oturumu kapatabilirsiniz. Daha sonraki ziyaretler için yönetim kurulunun Wi-Fi ağına katılın ve [yerel STCH kontrol paneli](http://192.168.4.1/) 'yi açın.

Yönetim kurulu bir sahip hesabını kabul eder. Kayıt, fiziksel ekranda gösterilen tek seferlik kodu gerektirir. Kontrol komutları bir sahip oturumu ve bir istek belirteci gerektirir. Panel, kartın yerel Wi-Fi ağında HTTP kullanıyor. Pano yeniden başlatıldıktan sonra tekrar oturum açın.

## Hareketler ve oyunlar

| Eylem | Sonuç |
|---|---|
| Gözlere dokunun veya tahtayı eğin | Gözler parmağınızı veya eğimi takip eder. |
| Gözlerden sağa doğru kaydırın | Oyunlar menüsünü açın. |
| Oyunda sola kaydırın | Menüye dönün. Gözlere dönmek için tekrar sola kaydırın. |
| Göz ekranını yaklaşık bir saniye basılı tutun | Wi-Fi QR kodunu gösterin. Ağ ve panel kodları arasında geçiş yapmak için dokunun. Gözlere dönmek için basılı tutun. |
| Gözlerden yukarı kaydırın | Bluetooth fare modunu başlatın. Bilgisayarınızdaki **Stach Mouse** ile eşleştirin ve kalibrasyon için kartı sabit tutun. |
| Fare modunda ekrana dokunun | Sol tıklamak için dokunun, sağ tıklamak için çift dokunun, kaydırmak için kaydırın veya çıkmak için basılı tutun. |

**Targets** 'de 20 saniye içinde vurabildiğiniz kadar hedefi vurun. **Sums** 'de beş ek soruyu yanıtlayın. Her iki oyun da puanınızı gösterir ve kişisel rekorunuzu kaydeder. **Dice** altı yüzlü bir zar atar ve **Coin** yazı tura atar. Her sonuç ESP32 donanım rastgele sayı üretecinden gelir. Yeni bir döndürme veya çevirme için animasyon bittikten sonra tekrar dokunun.

## Ekran ve güç

Telefon panelinde ifade seçimi, bakış joystick'i, göz kırpma ve eğme kontrolleri, göz parlaklığı ve gözlerin kendi başına ne kadar hareket edeceğine ilişkin bir ayar bulunur. Görünüm ayarları yeniden başlatmalarda korunur. Başlatma sonrasında fare modunu etkinleştirebilirsiniz.

1.75 kartında **AXP2101** pil yüzdesini ve şarj durumunu bildirir. Kart kablo gücüyle çalıştığında ve pil olmadığını tespit ettiğinde gösterge `USB` 'yi gösterir. Güç tasarrufu etkinleştirildiğinde, 45 saniye boyunca herhangi bir işlem yapılmadığında göz yenileme hızı düşer ve ekran 5 dakika sonra kapanır. %15 veya daha az olduğu bildirilen şarj durumunda bu aralıklar 10 saniye 2 dakikaya düşer. Uyandırmak için ekrana dokunun. Oyunlar, QR kodları, fare modu ve aktif şarj, ekranı uyanık tutar.

## Sorun giderme ve kurtarma

| Belirti | Ne denemeli |
|---|---|
| Yeni COM bağlantı noktası görünmüyor | Bir USB veri kablosunu ve başka bir USB bağlantı noktasını deneyin, ardından `platformio device list` 'yi yeniden çalıştırın. |
| Bağlantı noktası açılamıyor veya `Access denied` hatası veriyor | Seri monitörü kapatın. Kartı yeniden bağlayın ve COM numarasını kontrol edin. |
| Yapı başarısız oluyor | Geçerli klasörünüzün `platformio.ini` içerdiğini kontrol edin ve ardından 2. adımdaki her iki komutu da tekrarlayın. |
| Ekran boş kalıyor | Yükleme günlüğündeki pano modelini ve ortam adını kontrol edin. 1.75 kurulu için başarı çizgisi `stach-eyes-175` 'yi içermelidir. |
| Telefon paneli açamıyor | Hala `Stach-XXXXXX` üzerinde olup olmadığını kontrol edin ve `http://192.168.4.1/` öğesini açın. İlk kayıtta ikinci QR kodu gereklidir. |

Tanılamayı görüntülemek için seri monitörü 115200 baud'da açın.

```powershell
.\.venv\Scripts\python.exe -m platformio device monitor -p COM3 -b 115200
```

Donanım başlatma raporu için `I` 'yi, I²C cihazlarını listelemek için `S` 'yi veya beş saniyelik ekran testi için `D` 'yi gönderin. Monitörü kapatmak için **Ctrl+C** tuşuna basın.

Tam yedeklemenizi aynı panele geri yüklemek için monitörü kapatın ve önce görüntü boyutunu doğrulayın. Aşağıdaki komut ayrıca yedeklemede bulunan ayarları ve hesap verilerini de geri yükler.

```powershell
.\.venv\Scripts\python.exe -m esptool --chip esp32s3 --port COM3 write-flash 0x0 "$env:USERPROFILE\stach-before-install.flash-backup.bin"
```

**Yardıma mı ihtiyacınız var?** Discord veya [@def0kk (Telegram)](https://t.me/def0kk) üzerinden `twiinkprostolox123` ile iletişime geçin. Anakart modelini, PlatformIO ortamını ve hata mesajını ekleyin. Şifrenizi, kayıt QR kodunuzu ve tam flash yedeklemenizi gizli tutun.

## Proje geçmişi ve gelişimi

- **Aşama 9 — Güç.** Eski 1.43 profilleri GPIO4 üzerindeki ADC voltajını ölçer ve filtreler. Şarj tahmini, spesifik revizyon için şemanın kontrol edilmesini ve ölçülen akü voltajına göre kalibrasyon yapılmasını gerektirir. USB gücü tek başına şarjı onaylamaz. 1,75 profili AXP2101'i doğrudan okur.
- **Aşama 10 — Modlar ve kişiselleştirme.** Ekrandan veya `POST /api/mode` aracılığıyla yüz, Bluetooth fare, Wi-Fi QR ve panel QR arasında geçiş yapın. Göz parlaklığı ve bakış aktivitesi %20 ile %100 arasında değişir. BLE fare kontrolü bir donanım profili gerektirir.
- **Aşama 11 — Güvenilirlik ve güvenlik.** Ürün yazılımı, HTTP istek kökenlerini kontrol eder, komutları bir istek belirteci ile korur, istekleri hız sınırlar ve Wi-Fi erişim noktasını kurtarır. 1.75 profili aynı zamanda sahip girişi gerektirir. [Aşama 11 güvenlik notları](STAGE11-SECURITY.md)
- **Aşama 12 — Derlemeler ve tanılama.** GitHub Actions, sağlama toplamlarıyla oluşturulan kodları ve paketleri kontrol eder. `GET /api/hardware`, yazılım başlatma sonuçlarını rapor eder. Ekran çıkışı, dokunma tepkisi ve eşleştirme fiziksel kartta test edilebilir. [Oluşturun ve yayınlayın](STAGE12-RELEASE.md) · [Donanım testleri](STAGE12-HARDWARE-TESTS.md) · [Donanım teşhisi](STAGE2-HARDWARE-DIAGNOSTICS.md)

Ürün yazılımı giriş noktası `src/main.cpp` 'dir. Başlıklar `include/` 'de bulunur ve kontrol paneli kaynağı `web/stach-panel.html` 'dir. Paneli düzenledikten sonra `preview/stach-panel.html` 'yi eşleşecek şekilde güncelleyin ve ardından `python tools/build_web_header.py` komutunu çalıştırın. Gömülü sayfayı ve API sözleşmesini kontrol etmek için `python tools/check_panel.py` 'yi çalıştırın (Node.js gerektirir). Linux'ta taşınabilir çekirdek testleri için `sh tools/test_core.sh` 'yi çalıştırın. BLE yaması uyumluluğunu kontrol etmek için `python tools/test_ble_patch.py` 'yi çalıştırın.

STACH, Pip-KeyPet türünden ilham alan bağımsız bir projedir. PlatformIO üçüncü taraf kitaplıkları yükler ve proje, ESP32 BLE Mouse 0.3.1 'ye bir uyumluluk yaması uygular. [Waveshare 1.75 kaynakları](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75) · [Waveshare 1.43 referansı](https://www.waveshare.com/wiki/ESP32-S3-Touch-AMOLED-1.43)
