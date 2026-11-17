#include "stach_mouse.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
using namespace stach;
static motion::Sample sample(float z,float x=0,float y=0) {
  motion::Sample s; s.ax=0;s.ay=0;s.az=1; s.gz=z;s.gx=x;s.gy=y; return s;
}
int main() {
  int checks=0;auto check=[&](bool b){if(!b){std::cerr << "Failed check " << checks+1 << "\n";std::abort();}++checks;};
  float v=0;
  for(const char* s:{"0.5","0.75","1","2.25","4.0"})check(mouse::parseSensitivity(s,v));
  for(const char* s:{"","0","-2","4.01","NaN","inf","nan","1x"," 2","12345678901234"})check(!mouse::parseSensitivity(s,v));
  check(!mouse::parseSensitivity(nullptr,v));
  int8_t d=0;
  for(const char* s:{"-40","0","40","+12","-1"})check(mouse::parseDelta(s,d));
  for(const char* s:{"41","-41","5.5","NaN","","--1","9999999"," 4"})check(!mouse::parseDelta(s,d));
  check(!mouse::parseDelta(nullptr,d));
  check(mouse::gestureAction(touch::Gesture::Tap)==mouse::Action::LeftClick);
  check(mouse::gestureAction(touch::Gesture::DoubleTap)==mouse::Action::RightClick);
  check(mouse::gestureAction(touch::Gesture::LongPress)==mouse::Action::Exit);
  check(mouse::gestureAction(touch::Gesture::SwipeUp)==mouse::Action::ScrollUp);
  check(mouse::gestureAction(touch::Gesture::SwipeDown)==mouse::Action::ScrollDown);
  check(mouse::gestureAction(touch::Gesture::SwipeLeft)==mouse::Action::ScrollLeft);
  check(mouse::gestureAction(touch::Gesture::SwipeRight)==mouse::Action::ScrollRight);
  check(mouse::gestureAction(touch::Gesture::None)==mouse::Action::None);
  mouse::Engine e;
  check(!e.active()&&!e.connected()&&!e.calibrated());
  check(!e.update(sample(80),100).ready);
  e.enable(true,100);check(e.active()&&!e.calibrated());
  check(!e.update(sample(80),200).ready);
  e.link(true,200);check(e.connected());
  for(int i=0;i<10;++i)check(!e.update(sample(30),220+20*i).ready);
  check(e.calibrationSamples()==0);
  for(int i=0;i<47;++i)check(!e.update(sample(0.4f,0.6f),500+20*i).ready);
  check(!e.calibrated()&&e.calibrationSamples()==47);
  check(!e.update(sample(0.4f,0.6f),1440).ready);
  check(e.calibrated());
  bool moved=false;
  for(int i=0;i<20;++i){auto r=e.update(sample(100,70),1460+20*i);if(r.ready){moved=true;check(r.x>0 && r.y>=0);check(r.x<=32&&r.y<=32);}}
  check(moved);
  // Rollback: temporary IMU disconnect never emits delayed accumulated input.
  check(!e.update(sample(180),4000).ready);
  check(!e.update(sample(0.4f,0.6f),4020).ready);
  e.link(false,4040);check(!e.connected()&&!e.calibrated());
  check(!e.update(sample(100),4060).ready);
  e.link(true,4080);check(e.connected()&&!e.calibrated());
  for(int i=0;i<48;++i)e.update(sample(0.4f,0.6f),4100+20*i);
  check(e.calibrated());
  mouse::Config conf;conf.invertX=true;conf.invertY=true;
  e.configure(conf,5200);check(!e.calibrated());
  for(int i=0;i<48;++i)e.update(sample(0.4f,0.6f),5220+20*i);
  moved=false;
  for(int i=0;i<20;++i){auto r=e.update(sample(100,70),6200+20*i);if(r.ready){moved=true;check(r.x<0&&r.y<=0);}}
  check(moved);
  conf.axisYX=true;e.configure(conf,6700);check(!e.calibrated());
  for(int i=0;i<48;++i)e.update(sample(0.4f,0.6f,0.3f),6720+20*i);
  moved=false;
  for(int i=0;i<20;++i){auto r=e.update(sample(10,70,100),7700+20*i);if(r.ready){moved=true;check(r.x<0);}}
  check(moved);
  // Invalid sensitivity must not alter configuration.
  conf.sensitivity=std::numeric_limits<float>::quiet_NaN();e.configure(conf,8250);
  check(mouse::validSensitivity(e.config.sensitivity));
  e.enable(false,8300);check(!e.active()&&!e.calibrated());
  check(!e.update(sample(100),8320).ready);
  e.enable(true,8400);check(e.active()&&!e.calibrated());
  // Nonfinite / invalid accelerometer samples reset calibration.
  e.recalibrate(8500);
  motion::Sample bad=sample(0);bad.ax=std::numeric_limits<float>::quiet_NaN();
  check(!e.update(bad,8520).ready&&!e.calibrated());
  // Tick wrap does not cause huge movement; stale samples suppressed.
  e.link(false,8600);e.link(true,8600);
  for(int i=0;i<48;++i)e.update(sample(0,0,0),0xffffff00u+static_cast<uint32_t>(i*20));
  check(e.calibrated());
  check(!e.update(sample(60),0xffffff00u+48*20+200).ready);
  std::cout << "STACH mouse policy: " << checks << " assertions passed\n";
}
