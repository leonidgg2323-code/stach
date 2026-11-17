#pragma once
#ifdef STACH_WAVESHARE_BOARD
#include <Arduino.h>
#include <Wire.h>
#include "stach_board.h"
#include "stach_touch.h"
namespace stach { namespace touch {
// For Waveshare 1.43 standard and B, FT3168 or FT6146 at I2C 0x38.
// Shares Wire with QMI8658 + PCF85063. NEVER call Wire.begin() here.
struct Driver {
  bool ready=false;
  bool begin(){
    Wire.beginTransmission(board::ft3168_address);
    ready=Wire.endTransmission()==0;
    return ready;
  }
  // True = a valid point/release sample. False = a BUS/PROTOCOL error.
  bool read(Point& p){
    if(!ready)return false;
    Wire.beginTransmission(board::ft3168_address);
    Wire.write(uint8_t(0x02));
    if(Wire.endTransmission(false)!=0)return false;
    if(Wire.requestFrom(int(board::ft3168_address),5)!=5)return false;
    uint8_t r[5];
    for(int i=0;i<5;++i)r[i]=uint8_t(Wire.read());
    return decodeFT(r,p);
  }
};
}}
#endif
