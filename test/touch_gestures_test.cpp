#include "stach_touch.h"
#include "stach_motion.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace stach;

static touch::Point p(bool down,int x=100,int y=100){touch::Point r;r.down=down;r.x=int16_t(x);r.y=int16_t(y);return r;}
static void check(bool condition,int& n){if(!condition)std::cerr<<"Failed test #"<<(n+1)<<"\n";assert(condition);++n;}
int main(){
  int n=0;
  {
    touch::Point q;
    const uint8_t down[]={1,0x01,0x21,0x00,0xFE}; // x=289, y=254
    check(touch::decodeFT(down,q)&&q.down&&q.x==289&&q.y==254,n);
    const uint8_t contact[]={1,0x80,0x04,0x01,0x22};
    check(touch::decodeFT(contact,q)&&q.down&&q.x==4&&q.y==290,n);
    const uint8_t lift[]={1,0x40,0x04,0,0};
    check(touch::decodeFT(lift,q)&&!q.down,n);
    const uint8_t zero[]={0,0,0,0,0};
    check(touch::decodeFT(zero,q)&&!q.down,n);
    const uint8_t multi[]={2,0,0,0,0};
    check(!touch::decodeFT(multi,q),n);
    const uint8_t invalidX[]={1,1,0xF4,0,3}; // x=500
    check(!touch::decodeFT(invalidX,q),n);
    const uint8_t invalidEvent[]={1,0xC0,3,0,3};
    check(!touch::decodeFT(invalidEvent,q),n);
    check(!touch::decodeFT(nullptr,q),n);
  }
  {
    touch::Recognizer g;
    check(g.feed(p(true),100).kind==touch::Gesture::None&&g.pressed(),n);
    check(g.feed(p(false),180).kind==touch::Gesture::None&&!g.pressed(),n);
    check(g.feed(p(false),400).kind==touch::Gesture::None,n);
    check(g.feed(p(false),531).kind==touch::Gesture::Tap,n);
    check(g.feed(p(false),700).kind==touch::Gesture::None,n);
  }
  {
    touch::Recognizer g;
    g.feed(p(true),100);g.feed(p(false),150);
    g.feed(p(true,120,105),250);
    check(g.feed(p(false,120,105),310).kind==touch::Gesture::DoubleTap,n);
    check(g.feed(p(false),1000).kind==touch::Gesture::None,n);
  }
  {
    touch::Recognizer g;
    g.feed(p(true),100);
    check(g.feed(p(true,102,105),750).kind==touch::Gesture::LongPress,n);
    check(g.feed(p(true,102,105),850).kind==touch::Gesture::None,n);
    check(g.feed(p(false),900).kind==touch::Gesture::None,n);
  }
  {
    touch::Recognizer g;
    g.feed(p(true),0);g.feed(p(true,100,20),500);
    check(g.feed(p(false),810).kind==touch::Gesture::SwipeUp,n);
    g.feed(p(true,200,200),1000);g.feed(p(true,120,210),1120);
    check(g.feed(p(false),1200).kind==touch::Gesture::SwipeLeft,n);
    g.feed(p(true,200,200),1300);g.feed(p(true,280,200),1450);
    check(g.feed(p(false),1480).kind==touch::Gesture::SwipeRight,n);
    g.feed(p(true,200,200),1600);g.feed(p(true,200,310),1700);
    check(g.feed(p(false),1750).kind==touch::Gesture::SwipeDown,n);
  }
  {
    touch::Recognizer g;
    g.feed(p(true,110,110),0);g.feed(p(true,120,120),75);
    check(g.feed(p(false),150).kind==touch::Gesture::None,n);
    check(g.feed(p(false),505).kind==touch::Gesture::Tap,n);
    g.feed(p(true,100,100),600);g.feed(p(true,150,100),700);
    check(g.feed(p(true,152,100),1400).kind==touch::Gesture::None,n);
    g.cancel();check(!g.pressed(),n);
  }
  {
    touch::Recognizer g;
    g.feed(p(true,100,100),100);g.feed(p(false),150);
    check(g.feed(p(true,300,300),600).kind==touch::Gesture::Tap,n);
    check(g.pressed(),n);
    check(g.feed(p(false,300,300),660).kind==touch::Gesture::None,n);
    check(g.feed(p(false),1011).kind==touch::Gesture::Tap,n);
  }
  {
    // rollover of uint32_t timestamps must not cause duplicate long presses
    touch::Recognizer g;
    g.feed(p(true),0xFFFFFF00u);
    check(g.feed(p(true),0x000001A0u).kind==touch::Gesture::LongPress,n);
    check(g.feed(p(false),0x000001B0u).kind==touch::Gesture::None,n);
  }
  {
    motion::Tilt tilt;
    motion::Sample still;still.ax=0;still.ay=0;still.az=1;
    tilt.update(still);check(tilt.ready&&tilt.x==0&&tilt.y==0,n);
    motion::Sample side=still;side.ax=.6f;side.az=.8f;
    tilt.update(side);check(tilt.x>0&&tilt.x<.2f,n);
    float prev=tilt.x;
    motion::Sample impact=still;impact.ax=4;impact.az=4;
    tilt.update(impact);check(tilt.x==prev,n);
    motion::Sample freefall{};tilt.update(freefall);check(tilt.x==prev,n);
    motion::Gaze gaze;
    auto neutral=face::evaluate(1000,Emotion::Neutral);
    auto before=neutral;gaze.event(touch::Gesture::SwipeLeft,1000);
    gaze.apply(before,1000,tilt);
    check(std::fabs(before.offsetX-neutral.offsetX)<1.f,n);
    auto after=neutral;gaze.apply(after,1280,tilt);
    check(after.offsetX<-12&&after.gazeX<-.8f,n);
    auto released=neutral;gaze.apply(released,2800,tilt);
    check(released.offsetX>-5.f,n);
    gaze.event(touch::Gesture::SwipeRight,3000);
    auto right=neutral;gaze.apply(right,3400,tilt);
    check(right.offsetX>12&&right.gazeX>.8f,n);
    gaze.event(touch::Gesture::DoubleTap,3410);
    auto centered=neutral;gaze.apply(centered,3411,motion::Tilt{});
    check(centered.offsetX==neutral.offsetX,n);
  }
  {
    // Anti-edge regression: center of both eyes stays in the central 40x24px zone.
    motion::Tilt tilt;
    motion::Sample still;still.az=1.f;tilt.update(still);
    for(auto swipe:{touch::Gesture::SwipeLeft,touch::Gesture::SwipeRight,
                    touch::Gesture::SwipeUp,touch::Gesture::SwipeDown}){
      motion::Gaze gaze;
      gaze.event(swipe,100);
      for(uint32_t t=100;t<1900;t+=29){
        auto s=face::evaluate(t,Emotion::Neutral);
        gaze.apply(s,t,tilt);
        check(std::fabs(s.offsetX)<=face::MAX_EYE_SHIFT_X+.001f,n);
        check(std::fabs(s.offsetY)<=face::MAX_EYE_SHIFT_Y+.001f,n);
      }
    }
    // Automatic look + tilted IMU also remains central.
    motion::Tilt maxTilt;maxTilt.ready=true;maxTilt.x=1;maxTilt.y=-1;
    motion::Gaze noSwipe;
    for(uint32_t t=0;t<8000;t+=57){
      auto s=face::evaluate(t,Emotion::Neutral);
      noSwipe.apply(s,t,maxTilt);
      check(std::fabs(s.offsetX)<=face::MAX_EYE_SHIFT_X+.001f,n);
      check(std::fabs(s.offsetY)<=face::MAX_EYE_SHIFT_Y+.001f,n);
    }
    // Renderer itself provides a second clamp, regardless of caller inputs.
    struct Bounds {
      bool clean=true;int count=0;
      void clear(uint16_t){}
      void hline(int x,int y,int w,uint16_t color){
        if(color==0 || w<1)return;
        if(x<70 || x+w-1>396 || y<125 || y>340)clean=false;
        count+=w;
      }
    };
    for(float x:{-999.f,-50.f,-20.f,0.f,20.f,50.f,999.f}){
      for(float y:{-999.f,-50.f,-12.f,0.f,12.f,50.f,999.f}){
        face::State s;s.offsetX=x;s.offsetY=y;s.gazeX=x>0?1.f:-1.f;
        Bounds buf;face::draw(buf,s);
        check(buf.clean && buf.count>1000,n);
      }
    }
  }
  std::cout<<"Touch, gestures, motion checks passed: "<<n<<" assertions\n";
}
