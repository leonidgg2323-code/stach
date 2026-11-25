// STACH native QSPI LCD recovery diagnostic for Waveshare ESP32-S3 Touch AMOLED 1.43 / 1.43-B.
// Diagnostic only: no Wi-Fi, BLE, NVS, or main STACH app. No erase or partition writes.
// Command timing / QSPI framing based on the Apache-2.0 Waveshare first-party
// 07_LVGL_Test: esp_lcd_sh8601.c, lcd_bsp.c, esp_lcd_sh8601.h.
// https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.43/
#ifdef STACH_NATIVE_BRINGUP
#include <Arduino.h>
#include <Wire.h>
#include <esp_err.h>
#include <esp_heap_caps.h>
#include <driver/spi_master.h>
#include <esp_lcd_panel_io.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <string.h>

namespace {
constexpr int LCD_CS=9, LCD_CLK=10, LCD_D0=11, LCD_D1=12, LCD_D2=13, LCD_D3=14;
constexpr int LCD_RST=21, LCD_EN=42, I2C_SDA=47, I2C_SCL=48;
constexpr int W=466, H=466, STRIP_H=8;
constexpr int SPEED=20000000; // conservative 20 MHz vs Waveshare's nominal 40 MHz
esp_lcd_panel_io_handle_t io=nullptr;
SemaphoreHandle_t transferDone=nullptr;
uint16_t *dmaPixels=nullptr;
bool ready=false;
unsigned frame=0;
uint32_t frameStart=0;
uint8_t panelId=0;
uint8_t panelKind=0; // 1 = SH8601, 2 = CO5300

// QSPI commands use 32-bit opcode 02 00 CMD 00, color transfer 32 00 2C 00.
int qspiCommand(uint8_t cmd) {return (0x02u<<24) | (unsigned(cmd)<<8);}
int qspiPixels(uint8_t cmd) {return (0x32u<<24) | (unsigned(cmd)<<8);}

bool ok(esp_err_t result, const char* stage) {
  if(result!=ESP_OK) Serial.printf("[NATIVE][ERROR] %s: %s (%d)\n",stage,esp_err_to_name(result),(int)result);
  return result==ESP_OK;
}

bool lcdCmd(uint8_t cmd, const uint8_t* p=nullptr,size_t len=0) {
  return io && ok(esp_lcd_panel_io_tx_param(io,qspiCommand(cmd),p,len),"lcd command");
}

bool onColorDone(esp_lcd_panel_io_handle_t,esp_lcd_panel_io_event_data_t*,void* ctx) {
  BaseType_t woken=pdFALSE;
  xSemaphoreGiveFromISR(static_cast<SemaphoreHandle_t>(ctx),&woken);
  return woken==pdTRUE;
}

void sendSingle(uint8_t b) {
  for(int bit=0;bit<8;++bit){
    digitalWrite(LCD_D0,(b&0x80)?HIGH:LOW);
    b=uint8_t(b<<1);
    digitalWrite(LCD_CLK,LOW);
    delayMicroseconds(2);
    digitalWrite(LCD_CLK,HIGH);
    delayMicroseconds(2);
  }
}

// Read 0xDA as in the factory ID probe; 0xFF can mean pulled-up/disconnected.
uint8_t readId() {
  const int outputPins[]={LCD_CS,LCD_CLK,LCD_D0,LCD_D1,LCD_D2,LCD_D3,LCD_RST};
  for(int p:outputPins){pinMode(p,OUTPUT);digitalWrite(p,HIGH);}
  digitalWrite(LCD_RST,HIGH);delay(120);
  digitalWrite(LCD_RST,LOW);delay(120);
  digitalWrite(LCD_RST,HIGH);delay(120);
  digitalWrite(LCD_CS,LOW);
  for(uint8_t c:{uint8_t(0x03),uint8_t(0x00),uint8_t(0xDA),uint8_t(0x00)})sendSingle(c);
  uint8_t id=0;
  for(unsigned i=0;i<8;++i){
    digitalWrite(LCD_CLK,LOW);
    pinMode(LCD_D0,INPUT_PULLUP);
    delayMicroseconds(2);
    id=uint8_t((id<<1) | (digitalRead(LCD_D0)?1:0));
    pinMode(LCD_D0,OUTPUT);
    digitalWrite(LCD_CLK,HIGH);
    delayMicroseconds(2);
  }
  digitalWrite(LCD_CS,HIGH);
  Serial.printf("[LCD-ID] 0xDA -> 0x%02X; 86=SH8601, FF=possible CO5300 OR floating line\n",id);
  return id;
}

void scanI2C() {
  if(!Wire.begin(I2C_SDA,I2C_SCL,100000)){
    Serial.println("[I2C] Wire.begin failed");return;
  }
  unsigned count=0;
  for(uint8_t a=8;a<=0x77;++a){
    Wire.beginTransmission(a);
    if(Wire.endTransmission()==0){Serial.printf("[I2C] ACK 0x%02X\n",a);++count;}
  }
  Serial.printf("[I2C] devices=%u (expected touch=0x38, RTC=0x51, IMU=0x6A or 0x6B)\n",count);
}

bool initBus() {
  spi_bus_config_t bus={};
  bus.data0_io_num=LCD_D0;
  bus.data1_io_num=LCD_D1;
  bus.sclk_io_num=LCD_CLK;
  bus.data2_io_num=LCD_D2;
  bus.data3_io_num=LCD_D3;
  bus.max_transfer_sz=W*STRIP_H*2+16;
  if(!ok(spi_bus_initialize(SPI2_HOST,&bus,SPI_DMA_CH_AUTO),"spi_bus_initialize"))return false;
  transferDone=xSemaphoreCreateBinary();
  if(!transferDone){Serial.println("[NATIVE][ERROR] semaphore alloc");return false;}
  esp_lcd_panel_io_spi_config_t conf={};
  conf.cs_gpio_num=LCD_CS;
  conf.dc_gpio_num=-1;
  conf.spi_mode=0;
  conf.pclk_hz=SPEED;
  conf.trans_queue_depth=4;
  conf.on_color_trans_done=onColorDone;
  conf.user_ctx=transferDone;
  conf.lcd_cmd_bits=32;
  conf.lcd_param_bits=8;
  conf.flags.quad_mode=true;
  if(!ok(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST,&conf,&io),"new_panel_io_spi"))return false;
  dmaPixels=(uint16_t*)heap_caps_malloc(W*STRIP_H*2,MALLOC_CAP_DMA|MALLOC_CAP_INTERNAL);
  if(!dmaPixels){Serial.println("[NATIVE][ERROR] DMA buffer alloc");return false;}
  return true;
}

// Factory Waveshare sequences; don't claim panel was electrically acknowledged.
bool initPanel(uint8_t kind) {
  panelKind=kind;
  frame=0;
  ready=false;
  pinMode(LCD_RST,OUTPUT);
  digitalWrite(LCD_RST,LOW);delay(15);
  digitalWrite(LCD_RST,HIGH);delay(160);
  Serial.printf("[NATIVE] Initializing %s via esp_lcd QSPI\n",kind==1?"SH8601":"CO5300");
  if(!lcdCmd(0x11))return false;
  delay(kind==1?120:80);
  if(kind==1){
    const uint8_t tearLine[]={0x01,0xD1};
    const uint8_t zero=0;
    const uint8_t ctrl=0x20;
    if(!lcdCmd(0x44,tearLine,2)||!lcdCmd(0x35,&zero,1)||!lcdCmd(0x53,&ctrl,1))return false;
    delay(10);
  }else{
    const uint8_t mode=0x80,ctrl=0x20,hbm=0xFF;
    if(!lcdCmd(0xC4,&mode,1)||!lcdCmd(0x53,&ctrl,1)||!lcdCmd(0x63,&hbm,1))return false;
    delay(10);
  }
  const uint8_t minBrightness=0;
  if(!lcdCmd(0x51,&minBrightness,1)||!lcdCmd(0x29))return false;
  delay(15);
  const uint8_t maxBrightness=0xFF;
  if(!lcdCmd(0x51,&maxBrightness,1))return false;
  // Match factory driver RGB565 output (colmod=0x55).
  const uint8_t rgb565=0x55;
  if(!lcdCmd(0x3A,&rgb565,1))return false;
  ready=true;
  Serial.println("[NATIVE] init commands sent (visual confirmation required)");
  return true;
}

bool writeStrip(int y,int lines,const uint16_t* pixels) {
  const uint8_t xs[]={0x00,0x00,uint8_t((W-1)>>8),uint8_t((W-1)&0xFF)};
  const uint8_t ys[]={uint8_t(y>>8),uint8_t(y&0xFF),uint8_t((y+lines-1)>>8),uint8_t((y+lines-1)&0xFF)};
  if(!lcdCmd(0x2A,xs,4)||!lcdCmd(0x2B,ys,4))return false;
  // Color writes are asynchronous: keep DMA buffer unchanged until callback.
  xSemaphoreTake(transferDone,0);
  if(!ok(esp_lcd_panel_io_tx_color(io,qspiPixels(0x2C),pixels,W*lines*2),"tx_color"))return false;
  if(xSemaphoreTake(transferDone,pdMS_TO_TICKS(3000))!=pdTRUE){
    Serial.println("[NATIVE][ERROR] SPI DMA color timeout");return false;
  }
  return true;
}

uint16_t patternPixel(int x,int y,unsigned step) {
  if(step==0)return 0xFFFF;
  if(step==1){
    if(y>=120 && y<250){
      if(x>=77 && x<185)return 0xF800;
      if(x>=190 && x<298)return 0x07E0;
      if(x>=303 && x<411)return 0x001F;
    }
    return 0;
  }
  const float fy=float(y-230)/72.0f;
  const float fxl=float(x-168)/50.0f;
  const float fxr=float(x-298)/50.0f;
  const bool left=fxl*fxl+fy*fy<1.0f;
  const bool right=fxr*fxr+fy*fy<1.0f;
  if(!left && !right)return 0;
  const int cx=left?184:314;
  const int dx=x-cx,dy=y-240;
  return dx*dx+dy*dy<16*16?0:0xFFFF;
}

void drawPattern(unsigned step) {
  if(!ready)return;
  const char* name=step==0?"WHITE":step==1?"RGB":"EYES";
  Serial.printf("[NATIVE] draw=%s panel=%s\n",name,panelKind==1?"SH8601":"CO5300");
  for(int y=0;y<H;y+=STRIP_H){
    int lines=(H-y<STRIP_H)?H-y:STRIP_H;
    for(int j=0;j<lines;++j)for(int x=0;x<W;++x){
      const uint16_t color=patternPixel(x,y+j,step);
      // Controller expects high RGB565 byte first; ESP32 internal DMA buffers are LE.
      dmaPixels[j*W+x]=uint16_t((color>>8)|(color<<8));
    }
    if(!writeStrip(y,lines,dmaPixels)){
      Serial.println("[NATIVE][ERROR] drawing aborted");ready=false;break;
    }
  }
  Serial.printf("[NATIVE] draw %s done, software-side=%s\n",name,ready?"OK":"FAILED");
}
}

void setup() {
  Serial.begin(115200);
  delay(1400);
  Serial.println("[STACH-NATIVE] v0.2 — native Waveshare-like esp_lcd QSPI, DIAGNOSTIC ONLY");
  Serial.printf("[MEM] flash=%lu psram=%lu\n",(unsigned long)ESP.getFlashChipSize(),(unsigned long)ESP.getPsramSize());
  pinMode(LCD_EN,OUTPUT);digitalWrite(LCD_EN,HIGH);delay(150);
  scanI2C();
  panelId=readId();
  if(!initBus())return;
  if(!initPanel(panelId==0x86?1:2))return;
  drawPattern(0);
  frameStart=millis();
  Serial.println("[CMD] 1 = SH8601; 2 = CO5300; D = redraw; I = I2C scan");
}

void loop() {
  if(Serial.available()){
    const char c=char(Serial.read());
    if(c=='1'||c=='2'){
      const uint8_t which=(c=='1')?1:2;
      if(initPanel(which))drawPattern(0);
      frameStart=millis();
    }else if(c=='I'||c=='i')scanI2C();
    else if(c=='D'||c=='d'){
      frame=0;drawPattern(0);frameStart=millis();
    }
  }
  if(ready&&uint32_t(millis()-frameStart)>3000){
    frameStart=millis();frame=(frame+1)%3;drawPattern(frame);
  }
  delay(15);
}
#endif
