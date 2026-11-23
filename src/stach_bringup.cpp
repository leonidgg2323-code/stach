// STACH WaveShare 1.43/B hardware bring-up, NOT the final STACH firmware.
// The factory image backup MUST remain safe. No writes to storage/NVS are made.
// Pins and panel-ID sequence follow Waveshare's 1.43 Arduino LVGL example.
#ifdef STACH_BRINGUP
#include <Arduino.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>

static constexpr int LCD_CS=9, LCD_CLK=10, LCD_D0=11, LCD_D1=12, LCD_D2=13, LCD_D3=14;
static constexpr int LCD_RST=21, LCD_EN=42, I2C_SDA=47, I2C_SCL=48;
static constexpr int WIDTH=466, HEIGHT=466;

static Arduino_DataBus* dataBus=nullptr;
static Arduino_GFX* gfx=nullptr;
static bool gfxReady=false;
static uint8_t rawId=0;
static uint32_t lastDraw=0;
static uint8_t frame=0;

static void sendByteOneLane(uint8_t value) {
  for(int i=0;i<8;++i) {
    digitalWrite(LCD_D0,(value&0x80)?HIGH:LOW);
    value<<=1;
    digitalWrite(LCD_CLK,LOW);
    delayMicroseconds(2);
    digitalWrite(LCD_CLK,HIGH);
    delayMicroseconds(2);
  }
}

// Non-destructive display ID probe based on Waveshare's read_lcd_id_bsp.c:
// 03 00 DA 00, then sample data lane 0. It resets ONLY the LCD controller.
static uint8_t readPanelId() {
  const int outputs[]={LCD_CS,LCD_CLK,LCD_D0,LCD_D1,LCD_D2,LCD_D3,LCD_RST};
  for(int pin:outputs) { pinMode(pin,OUTPUT); digitalWrite(pin,HIGH); }
  digitalWrite(LCD_RST,HIGH);delay(120);
  digitalWrite(LCD_RST,LOW);delay(120);
  digitalWrite(LCD_RST,HIGH);delay(120);
  digitalWrite(LCD_CS,LOW);
  sendByteOneLane(0x03);
  sendByteOneLane(0x00);
  sendByteOneLane(0xDA);
  sendByteOneLane(0x00);
  uint8_t result=0;
  for(int i=0;i<8;++i) {
    digitalWrite(LCD_CLK,LOW);
    pinMode(LCD_D0,INPUT_PULLUP);
    delayMicroseconds(2);
    result=uint8_t((result<<1) | (digitalRead(LCD_D0)?1:0));
    pinMode(LCD_D0,OUTPUT);
    digitalWrite(LCD_CLK,HIGH);
    delayMicroseconds(2);
  }
  digitalWrite(LCD_CS,HIGH);
  Serial.printf("[LCD-ID] register 0xDA: 0x%02X (0x86=SH8601, 0xFF=possible CO5300, other=unknown)\n",result);
  // 0xFF can also be a disconnected/pulled-up data lane. Never assert ID proof.
  return result;
}

static void scanI2C() {
  Serial.println("[I2C] Scanning addresses 0x08..0x77 (GPIO47 SDA, GPIO48 SCL)");
  unsigned count=0;
  for(uint8_t addr=0x08;addr<=0x77;++addr) {
    Wire.beginTransmission(addr);
    const uint8_t result=Wire.endTransmission();
    if(result==0) { Serial.printf("[I2C] ACK 0x%02X\n",addr);++count; }
  }
  Serial.printf("[I2C] devices=%u; expected 0x38 TOUCH, 0x51 RTC, 0x6A/0x6B IMU\n",count);
  if(count==0) Serial.println("[I2C] WARNING no ACK. This is NOT proof of hardware damage.");
}

static void pattern(uint8_t stage) {
  if(!gfxReady)return;
  if(stage==0) {
    gfx->fillScreen(0xFFFF);
    Serial.println("[LCD] TEST white full-screen");
  } else if(stage==1) {
    gfx->fillScreen(0x0000);
    gfx->fillRect(90,120,90,90,0xF800);
    gfx->fillRect(188,120,90,90,0x07E0);
    gfx->fillRect(286,120,90,90,0x001F);
    Serial.println("[LCD] TEST red green blue blocks");
  } else {
    gfx->fillScreen(0x0000);
    gfx->fillRoundRect(117,164,92,145,45,0xFFFF);
    gfx->fillRoundRect(257,164,92,145,45,0xFFFF);
    gfx->fillCircle(171,237,22,0x0000);
    gfx->fillCircle(311,237,22,0x0000);
    Serial.println("[LCD] TEST white eyes on black");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1400); // USB CDC monitor can attach after power-on.
  Serial.println("[STACH-BRINGUP] v0.1 test ONLY. Factory backup remains on PC.");
  Serial.printf("[MEM] flash=%lu psram=%lu\n",(unsigned long)ESP.getFlashChipSize(),(unsigned long)ESP.getPsramSize());
  // Waveshare defines GPIO42 as AMOLED_EN. Enable it before peripheral checks.
  pinMode(LCD_EN,OUTPUT);
  digitalWrite(LCD_EN,HIGH);
  delay(250);
  pinMode(I2C_SDA,INPUT_PULLUP);
  pinMode(I2C_SCL,INPUT_PULLUP);
  const bool started=Wire.begin(I2C_SDA,I2C_SCL,100000);
  Serial.printf("[I2C] Wire.begin=%d\n",started?1:0);
  if(started)scanI2C();
  rawId=readPanelId();
  dataBus=new Arduino_ESP32QSPI(LCD_CS,LCD_CLK,LCD_D0,LCD_D1,LCD_D2,LCD_D3);
  if(rawId==0xFF) {
    Serial.println("[LCD] Candidate CO5300; ID 0xFF alone is inconclusive");
    auto* panel=new Arduino_CO5300(dataBus,LCD_RST,0,WIDTH,HEIGHT);
    gfx=panel;
    gfxReady=panel->begin(10000000); // conservative 10 MHz vs default.
    if(gfxReady)panel->setBrightness(255);
  } else {
    Serial.println(rawId==0x86?"[LCD] Candidate SH8601":"[LCD] Unidentified, trying SH8601 at 10MHz");
    auto* panel=new Arduino_SH8601(dataBus,LCD_RST,0,WIDTH,HEIGHT);
    gfx=panel;
    gfxReady=panel->begin(10000000);
    if(gfxReady)panel->setBrightness(255);
  }
  Serial.printf("[LCD] gfx begin=%s. This is only a software result.\n",gfxReady?"OK":"FAILED");
  if(gfxReady) pattern(0);
  lastDraw=millis();
  Serial.println("[CMD] I = rescan I2C, D = restart color test. Look at display.");
}

void loop() {
  if(Serial.available()){
    const char key=char(Serial.read());
    if(key=='I'||key=='i')scanI2C();
    if(key=='D'||key=='d') {frame=0;pattern(frame);lastDraw=millis();}
  }
  if(gfxReady && uint32_t(millis()-lastDraw)>=2000){
    lastDraw=millis();frame=(frame+1)%3;pattern(frame);
  }
  delay(20);
}

#endif // STACH_BRINGUP: compiled out of all production firmware profiles.
