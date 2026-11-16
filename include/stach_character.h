#pragma once
#include <stdint.h>
#include "stach_face.h"
namespace stach { namespace character {
struct Controller {
  Emotion current=Emotion::Neutral;
  face::State from{};
  uint32_t since=0,duration=360;
  bool transitioning=false, charging=false;
  static float mix(float a,float b,float t){return a+(b-a)*t;}
  face::State sample(uint32_t now) const {
    face::State to=face::evaluate(now,current);
    to.chargeStrength=charging?1.f:0.f;
    if(!transitioning)return to;
    float t=face::ease(face::clamp(float(uint32_t(now-since))/float(duration),0,1));
    face::State r;
#define STACH_BLEND(f) r.f=mix(from.f,to.f,t)
    STACH_BLEND(gazeX); STACH_BLEND(gazeY); STACH_BLEND(openness);
    STACH_BLEND(offsetX); STACH_BLEND(offsetY); STACH_BLEND(scale);
    STACH_BLEND(browLift); STACH_BLEND(browTilt); STACH_BLEND(browStrength);
    STACH_BLEND(browArc); STACH_BLEND(browAsymmetry); STACH_BLEND(chargeStrength);
#undef STACH_BLEND
    r.chargePhase=to.chargePhase;
    return r;
  }
  bool set(Emotion next,uint32_t now,bool newCharging=false){
    if(next==Emotion::Invalid)return false;
    if(next==current&&newCharging==charging)return true;
    from=sample(now);current=next;charging=newCharging;
    since=now;transitioning=true;return true;
  }
  void tick(uint32_t now){if(transitioning&&uint32_t(now-since)>=duration)transitioning=false;}
};
}}
