#pragma once
#include <stdint.h>
#include <stdlib.h>
// Portable FT3168/FT6146 touch decoding and single-finger gesture engine.
// Hardware bus access lives in stach_touch_hal.h, so this file is testable on PC.
namespace stach { namespace touch {
constexpr int kWidth=466, kHeight=466;
struct Point { bool down=false; int16_t x=0, y=0; };
enum class Gesture:uint8_t { None, Tap, DoubleTap, LongPress, SwipeLeft, SwipeRight, SwipeUp, SwipeDown };
inline const char* name(Gesture g){
  switch(g){
    case Gesture::Tap:return "tap";
    case Gesture::DoubleTap:return "double_tap";
    case Gesture::LongPress:return "long_press";
    case Gesture::SwipeLeft:return "swipe_left";
    case Gesture::SwipeRight:return "swipe_right";
    case Gesture::SwipeUp:return "swipe_up";
    case Gesture::SwipeDown:return "swipe_down";
    default:return "none";
  }
}
struct Event {Gesture kind=Gesture::None; int16_t x=0,y=0;};

// FT registers: 0x02 touch count, 0x03..0x06 finger-1 XH,XL,YH,YL.
// The two high XH bits carry event: 0 down, 1 lift, 2 contact.
inline bool decodeFT(const uint8_t raw[5],Point& point){
  if(!raw)return false;
  const uint8_t count=raw[0]&0x0F;
  if(count==0){point.down=false;return true;}
  if(count!=1)return false; // Multi-touch is intentionally unsupported.
  const uint8_t event=(raw[1]>>6)&3;
  if(event==1){point.down=false;return true;}
  if(event==3)return false;
  const int x=((raw[1]&0x0F)<<8)|raw[2];
  const int y=((raw[3]&0x0F)<<8)|raw[4];
  if(x<0||x>=kWidth||y<0||y>=kHeight)return false;
  point.down=true;point.x=int16_t(x);point.y=int16_t(y);
  return true;
}

class Recognizer {
  bool pressed_=false, longSent_=false, pendingTap_=false;
  uint32_t began_=0, pendingAt_=0;
  int16_t startX_=0,startY_=0,lastX_=0,lastY_=0,tapX_=0,tapY_=0;
  static int32_t squared(int32_t n){return n*n;}
  static int32_t dist2(int x,int y,int u,int v){return squared(x-u)+squared(y-v);}
public:
  bool pressed()const{return pressed_;}
  void cancel(){pressed_=false;longSent_=false;pendingTap_=false;}
  Event feed(const Point& p,uint32_t now,bool deferSingleTap=true){
    if(p.down){
      // A second press after the double-tap window must not eat the first tap.
      const bool expired=pendingTap_ && uint32_t(now-pendingAt_)>350;
      const int16_t oldTapX=tapX_,oldTapY=tapY_;
      if(expired)pendingTap_=false;
      if(!pressed_){
        pressed_=true;longSent_=false;began_=now;
        startX_=p.x;startY_=p.y;lastX_=p.x;lastY_=p.y;
      } else {lastX_=p.x;lastY_=p.y;}
      if(expired)return {Gesture::Tap,oldTapX,oldTapY};
      if(!longSent_ && uint32_t(now-began_)>=650 &&
         dist2(lastX_,lastY_,startX_,startY_)<=22*22){
        longSent_=true;pendingTap_=false;
        return {Gesture::LongPress,lastX_,lastY_};
      }
      return {};
    }
    if(pressed_){
      pressed_=false;
      const uint32_t elapsed=uint32_t(now-began_);
      const int32_t dx=int32_t(lastX_)-startX_,dy=int32_t(lastY_)-startY_;
      if(longSent_)return {};
      if(elapsed<=1000 && (abs(dx)>=54||abs(dy)>=54) &&
          dist2(lastX_,lastY_,startX_,startY_)>=54*54){
        pendingTap_=false;
        const Gesture g=(abs(dx)>=abs(dy))?(dx<0?Gesture::SwipeLeft:Gesture::SwipeRight)
                                                :(dy<0?Gesture::SwipeUp:Gesture::SwipeDown);
        return {g,lastX_,lastY_};
      }
      if(elapsed<=350 && dist2(lastX_,lastY_,startX_,startY_)<=22*22){
        if(!deferSingleTap){pendingTap_=false;return {Gesture::Tap,lastX_,lastY_};}
        if(pendingTap_ && uint32_t(now-pendingAt_)<=350 &&
           dist2(startX_,startY_,tapX_,tapY_)<=65*65){
          pendingTap_=false;
          return {Gesture::DoubleTap,lastX_,lastY_};
        }
        pendingTap_=true;pendingAt_=now;tapX_=lastX_;tapY_=lastY_;
        return {};
      }
      return {};
    }
    if(pendingTap_ && uint32_t(now-pendingAt_)>350){
      pendingTap_=false;
      return {Gesture::Tap,tapX_,tapY_};
    }
    return {};
  }
};
}} // namespace stach::touch
