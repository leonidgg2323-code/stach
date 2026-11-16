#pragma once
#include <stdint.h>
#include <cmath>
#include "stach_core.h"

// Event-driven emotion selector; portable and hardware-independent.
// A sensor provider must report REAL charging status and accelerometer g.
namespace stach { namespace behavior {
struct FallDetector {
  uint64_t lowSince=0,armedAt=0,cooldownUntil=0;
  bool lowActive=false,armed=false;
  bool add(float g,uint64_t now){
    if(!std::isfinite(g)||g<0)return false;
    if(now<cooldownUntil)return false;
    if(g<0.38f){
      if(!lowActive){lowSince=now;lowActive=true;}
      if(now-lowSince>=75){armed=true;armedAt=now;}
      return false;
    }
    lowActive=false;
    if(armed){
      if(now-armedAt>1600){armed=false;return false;}
      if(g>2.2f){armed=false;cooldownUntil=now+5000;return true;}
    }
    return false;
  }
};
struct Engine {
  uint64_t started=0,lastChargeMs=0,angryUntil=0,curiousUntil=0;
  bool chargingKnown=false,charging=false,batteryKnown=false;
  int batteryPercent=100;
  uint64_t sleepyAfterMs=48ULL*60*60*1000; // continuous-uptime default, configurable
  FallDetector fall;
  void begin(uint64_t now){started=now;lastChargeMs=now;}
  void charge(bool known,bool plugged,uint64_t now){
    chargingKnown=known;charging=known&&plugged;
    if(charging)lastChargeMs=now;
  }
  void battery(int pct){batteryKnown=(pct>=0&&pct<=100);if(batteryKnown)batteryPercent=pct;}
  void acceleration(float magnitudeG,uint64_t now){if(fall.add(magnitudeG,now))angryUntil=now+6500;}
  void touch(uint64_t now){curiousUntil=now+4200;}
  Emotion choose(uint64_t now) const {
    if(now<angryUntil)return Emotion::Angry;
    if(chargingKnown&&charging)return Emotion::Happy;
    if((batteryKnown&&batteryPercent<=12) || (now>=lastChargeMs&&now-lastChargeMs>=sleepyAfterMs))return Emotion::Sleepy;
    if(now<curiousUntil)return Emotion::Curious;
    return Emotion::Neutral;
  }
  bool showCharging() const{return chargingKnown&&charging;}
};
}}
