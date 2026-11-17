#pragma once
#include <stdint.h>
#include <math.h>
#include "stach_face.h"
#include "stach_touch.h"
namespace stach { namespace motion {
struct Sample {float ax=0,ay=0,az=0,gx=0,gy=0,gz=0;}; // accel g, gyro deg/s
inline float clamp(float a,float lo,float hi){return a<lo?lo:(a>hi?hi:a);}
struct Tilt {
  float x=0,y=0;
  bool ready=false;
  // Gravitational tilt, reject freefall/impact. Mounted orientation calibration
  // (swap/invert axes) is deferred to on-board confirmation.
  void update(const Sample& s){
    const float mag=sqrtf(s.ax*s.ax+s.ay*s.ay+s.az*s.az);
    if(!isfinite(mag)||mag<.75f||mag>1.3f)return;
    float tx=clamp(s.ax*1.25f,-1.f,1.f);
    float ty=clamp(s.ay*1.25f,-1.f,1.f);
    if(!ready){x=tx;y=ty;ready=true;return;}
    x=x*.88f+tx*.12f; y=y*.88f+ty*.12f;
  }
};
struct Gaze {
  float targetX=0,targetY=0;
  uint32_t started=0;
  bool active=false;
  void event(touch::Gesture g,uint32_t now){
    if(g==touch::Gesture::SwipeLeft){targetX=-1;targetY=0;}
    else if(g==touch::Gesture::SwipeRight){targetX=1;targetY=0;}
    else if(g==touch::Gesture::SwipeUp){targetX=0;targetY=-1;}
    else if(g==touch::Gesture::SwipeDown){targetX=0;targetY=1;}
    else if(g==touch::Gesture::DoubleTap){active=false;return;}
    else return;
    started=now;active=true;
  }
  // Smooth ramp, hold, release. The face remains crisp and unmodified at source.
  void apply(face::State& s,uint32_t now,const Tilt& tilt) const {
    float weight=0;
    if(active){
      const uint32_t age=uint32_t(now-started);
      if(age<170)weight=face::ease(float(age)/170.f);
      else if(age<1100)weight=1;
      else if(age<1600)weight=1-face::ease(float(age-1100)/500.f);
    }
    if(weight>0){
      s.gazeX=targetX*weight+s.gazeX*(1-weight);
      s.gazeY=targetY*weight+s.gazeY*(1-weight);
      s.offsetX=face::clamp(s.offsetX*(1-weight)+targetX*18.f*weight,
                            -face::MAX_EYE_SHIFT_X,face::MAX_EYE_SHIFT_X);
      s.offsetY=face::clamp(s.offsetY*(1-weight)+targetY*10.f*weight,
                            -face::MAX_EYE_SHIFT_Y,face::MAX_EYE_SHIFT_Y);
      return;
    }
    // Gentle tilt follows gravity a little; never override emotion or hard gaze.
    if(tilt.ready){
      const float x=fabsf(tilt.x)<.16f?0:tilt.x;
      const float y=fabsf(tilt.y)<.16f?0:tilt.y;
      s.offsetX=face::clamp(s.offsetX+x*4.f,-face::MAX_EYE_SHIFT_X,face::MAX_EYE_SHIFT_X);
      s.offsetY=face::clamp(s.offsetY+y*3.f,-face::MAX_EYE_SHIFT_Y,face::MAX_EYE_SHIFT_Y);
      s.gazeX=face::clamp(s.gazeX+x*.20f,-1,1);
      s.gazeY=face::clamp(s.gazeY+y*.18f,-1,1);
    }
  }
};
}}
