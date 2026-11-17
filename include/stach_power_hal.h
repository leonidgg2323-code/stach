#pragma once
#ifdef STACH_WAVESHARE_BOARD
#include <Arduino.h>
#include "stach_board.h"
#include "stach_power.h"
#ifndef STACH_CHARGE_STATUS_PIN
#define STACH_CHARGE_STATUS_PIN -1
#endif
namespace stach { namespace power {
// Charge status is intentionally UNKNOWN without a board-verified pin.
// Do NOT infer charging from BAT_ADC or USB serial connection.
inline void beginHardware() {
  analogReadResolution(12);
#if STACH_CHARGE_STATUS_PIN >= 0
  pinMode(STACH_CHARGE_STATUS_PIN, INPUT_PULLUP);
#endif
}
inline int adcSampleMilliVolts() {
  // Median of 5 calibrated ADC pin voltage samples, not battery voltage.
  int samples[5];
  for(int i=0;i<5;++i) samples[i]=int(analogReadMilliVolts(board::battery_adc));
  for(int i=1;i<5;++i){int x=samples[i],j=i-1;while(j>=0 && samples[j]>x){samples[j+1]=samples[j];--j;}samples[j+1]=x;}
  return samples[2];
}
inline bool readCharging(bool& charging) {
#if STACH_CHARGE_STATUS_PIN >= 0
  // Active-LOW, must be confirmed against exact schematic and verified in hardware.
  charging=(digitalRead(STACH_CHARGE_STATUS_PIN)==LOW);
  return true;
#else
  charging=false;
  return false;
#endif
}
}} // namespace stach::power
#endif
