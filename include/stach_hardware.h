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
inline Probe probe() {
  Probe p;
  p.psram=psramFound();
  p.freePsram=ESP.getFreePsram();
  p.i2c=Wire.begin(board::i2c_sda,board::i2c_scl,100000);
  if(p.i2c) {
    p.imu=i2cPresent(board::qmi8658_address_a)||i2cPresent(board::qmi8658_address_b);
    p.rtc=i2cPresent(board::pcf85063_address);
    p.touch=i2cPresent(board::ft3168_address);
  }
  analogReadResolution(12);
  p.adcRaw=analogRead(board::battery_adc);
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
