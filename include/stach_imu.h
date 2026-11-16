#pragma once
#ifdef STACH_WAVESHARE_BOARD
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include "stach_board.h"
// Minimal QMI8658 accelerometer-only driver for fall event classification.
// Register reference: QMI8658 CTRL2=0x03 CTRL7=0x08 AX_L=0x35.
namespace stach { namespace imu {
struct Driver {
  uint8_t address=0;
  bool write(uint8_t reg,uint8_t data){
    Wire.beginTransmission(address);Wire.write(reg);Wire.write(data);
    return Wire.endTransmission()==0;
  }
  bool begin(){
    const uint8_t choices[]={board::qmi8658_address_a,board::qmi8658_address_b};
    for(uint8_t addr:choices){
      Wire.beginTransmission(addr);Wire.write(uint8_t(0));
      if(Wire.endTransmission(false)!=0)continue;
      if(Wire.requestFrom(int(addr),1)!=1)continue;
      if(Wire.read()==5){address=addr;break;}
    }
    if(!address)return false;
    // Accel +/-8g, 125 Hz ODR (CTRL2 aFS=2, aODR=3); accel only.
    return write(0x08,0x00)&&write(0x03,0x23)&&write(0x08,0x01);
  }
  bool accelerationG(float& magnitude){
    if(!address)return false;
    Wire.beginTransmission(address);Wire.write(uint8_t(0x35));
    if(Wire.endTransmission(false)!=0)return false;
    if(Wire.requestFrom(int(address),6)!=6)return false;
    uint8_t bytes[6];for(int i=0;i<6;++i)bytes[i]=uint8_t(Wire.read());
    int16_t x=int16_t((uint16_t(bytes[1])<<8)|bytes[0]);
    int16_t y=int16_t((uint16_t(bytes[3])<<8)|bytes[2]);
    int16_t z=int16_t((uint16_t(bytes[5])<<8)|bytes[4]);
    float gx=float(x)/4096.f,gy=float(y)/4096.f,gz=float(z)/4096.f;
    magnitude=sqrtf(gx*gx+gy*gy+gz*gz);
    return isfinite(magnitude);
  }
};
}} // namespace stach::imu
#endif
