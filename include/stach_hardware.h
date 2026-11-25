#pragma once
#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#include <esp_heap_caps.h>
#include "stach_board.h"
namespace stach { namespace hardware {
struct Probe { bool psram=false, i2c=false, imu=false, rtc=false, touch=false; int adcRaw=-1; uint32_t freePsram=0; };
inline bool i2cPresent(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission()==0;
}
// Scan the already initialized hardware I2C bus; command S is available
// over Serial. Reports actual ACK responses instead of Wire.begin success.
inline void scanI2C() {
  unsigned devices=0;
  for(uint8_t address=0x08; address<=0x77; ++address) {
    Wire.beginTransmission(address);
    if(Wire.endTransmission()==0) {
      Serial.printf("[I2C] ACK 0x%02X\n",address);
      ++devices;
    }
  }
  Serial.printf("[I2C] devices=%u (touch=0x%02X RTC=0x51 IMU=0x6A/0x6B)\n",devices,board::touch_address);
}
inline Probe probe() {
  Probe p;
  p.psram=psramFound();
  p.freePsram=ESP.getFreePsram();
  p.i2c=Wire.begin(board::i2c_sda,board::i2c_scl,100000);
  if(p.i2c) {
    p.imu=i2cPresent(board::qmi8658_address_a)||i2cPresent(board::qmi8658_address_b);
    p.rtc=i2cPresent(board::pcf85063_address);
    p.touch=i2cPresent(board::touch_address);
  }
#ifndef STACH_EYES_175
  analogReadResolution(12);
  p.adcRaw=analogRead(board::battery_adc);
#endif
  return p;
}
inline void logProbe(const Probe& p) {
  Serial.printf("[HW] PSRAM=%s free=%lu bytes\n",p.psram?"yes":"no",(unsigned long)p.freePsram);
  Serial.printf("[HW] I2C=%s IMU=%s RTC=%s TOUCH=%s\n",p.i2c?"ok":"failed",p.imu?"seen":"not seen",p.rtc?"seen":"not seen",p.touch?"seen":"not seen");
  Serial.printf("[HW] BAT_ADC raw=%d (not calibrated; not battery percent)\n",p.adcRaw);
  Serial.println("[HW] AMOLED initialization follows probe");
}
}}
#endif
