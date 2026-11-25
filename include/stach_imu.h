#pragma once
#if defined(STACH_WAVESHARE_BOARD) || defined(STACH_EYES_175)
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include "stach_board.h"
#include "stach_motion.h"
// QMI8658: ACC_X_L=0x35, GYR_X_L=0x3B, WHO_AM_I=0x00 (0x05).
// ACC +/-8g (4096 LSB/g), GYR +/-512deg/s (64 LSB/dps).
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
    // CTRL1=0x40: enable auto-increment, little endian, preserve I2C mode.
    // CTRL2=0x26: accel +/-8g, 125 Hz (accel-only); CTRL3=0x56:
    // gyro +/-512 dps, approximately 112-125 Hz depending on chip revision.
    // CTRL7=0x03: enable accel+gyro. Never re-init Wire here.
    return write(0x08,0x00) && write(0x02,0x40) &&
           write(0x03,0x26) && write(0x04,0x56) && write(0x08,0x03);
  }
  static int16_t i16(const uint8_t* p){return int16_t((uint16_t(p[1])<<8)|p[0]);}
  bool read(stach::motion::Sample& result){
    if(!address)return false;
    Wire.beginTransmission(address);Wire.write(uint8_t(0x35));
    if(Wire.endTransmission(false)!=0)return false;
    if(Wire.requestFrom(int(address),12)!=12)return false;
    uint8_t d[12];for(int i=0;i<12;++i)d[i]=uint8_t(Wire.read());
    result.ax=float(i16(d+0))/4096.f;
    result.ay=float(i16(d+2))/4096.f;
    result.az=float(i16(d+4))/4096.f;
    result.gx=float(i16(d+6))/64.f;
    result.gy=float(i16(d+8))/64.f;
    result.gz=float(i16(d+10))/64.f;
    return isfinite(result.ax)&&isfinite(result.ay)&&isfinite(result.az)&&
           isfinite(result.gx)&&isfinite(result.gy)&&isfinite(result.gz);
  }
};
}}
#endif
