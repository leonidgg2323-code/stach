#pragma once
#ifdef STACH_WAVESHARE_BOARD
#include <Arduino.h>
#include <Wire.h>
#include "stach_board.h"
#include "stach_touch.h"
#ifdef STACH_EYES_175
#include <TouchDrvCSTXXX.hpp>
#endif
namespace stach { namespace touch {
#ifdef STACH_EYES_175
static volatile bool touchPending;
static void IRAM_ATTR touchInterrupt() { touchPending=true; }
struct Driver {
  TouchDrvCST92xx touch;
  Point finger;
  uint32_t lastTouch=0;
  bool begin() {
    touch.setPins(40,11);
    if (!touch.begin(Wire,board::touch_address,board::i2c_sda,board::i2c_scl)) return false;
    touch.setMaxCoordinates(466,466);
    touch.setMirrorXY(true,true);
    pinMode(11,INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(11),touchInterrupt,FALLING);
    return true;
  }
  bool read(Point& point) {
    const uint32_t now=millis();
    noInterrupts();
    bool pending = touchPending;
    touchPending = false;
    interrupts();
    if (pending || digitalRead(11) == LOW) {
      int16_t x[5]{}, y[5]{};
      finger.down = touch.getPoint(x, y, 5) == 1;
      if (finger.down) {
        finger.x = constrain(x[0], 0, 465);
        finger.y = constrain(y[0], 0, 465);
      }
      lastTouch=now;
    } else if (now-lastTouch>1500) finger.down=false;
    point=finger;
    return true;
  }
};
#else
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
#endif
}}
#endif
