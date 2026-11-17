#pragma once
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <initializer_list>
#include "stach_pairing.h"

namespace stach { namespace modes {
// Ephemeral display/HID mode. Only user preferences are persisted.
// A restart must NEVER resume sending Bluetooth mouse reports.
enum class Mode : uint8_t { Face, Mouse, Wifi, Portal };
inline const char* name(Mode m) {
  switch(m) {
    case Mode::Face: return "face";
    case Mode::Mouse: return "mouse";
    case Mode::Wifi: return "wifi";
    case Mode::Portal: return "portal";
  }
  return "face";
}
inline bool parse(const char* s, Mode& out) {
  if(!s)return false;
  for(Mode m : {Mode::Face, Mode::Mouse, Mode::Wifi, Mode::Portal}) {
    if(strcmp(s,name(m))==0){out=m;return true;}
  }
  return false;
}
inline Mode fromState(bool mouseActive,pairing::Screen screen) {
  if(mouseActive)return Mode::Mouse;
  return screen==pairing::Screen::Wifi?Mode::Wifi:screen==pairing::Screen::Portal?Mode::Portal:Mode::Face;
}
struct Plan {
  bool accepted=false;
  bool enableMouse=false;
  bool disableMouse=false;
  pairing::Screen screen=pairing::Screen::Face;
};
inline Plan transition(Mode oldMode,Mode next,bool supportsMouse) {
  Plan p;
  if(next==Mode::Mouse && !supportsMouse)return p;
  p.accepted=true;
  p.enableMouse=(next==Mode::Mouse && oldMode!=Mode::Mouse);
  p.disableMouse=(oldMode==Mode::Mouse && next!=Mode::Mouse);
  p.screen=next==Mode::Wifi?pairing::Screen::Wifi:next==Mode::Portal?pairing::Screen::Portal:pairing::Screen::Face;
  return p;
}
inline bool validBrightness(unsigned n){return n>=20 && n<=100;}
inline bool validGazeActivity(unsigned n){return n>=20 && n<=100;}
inline bool parsePercent(const char* s,unsigned& out) {
  if(!s || !*s)return false;
  unsigned value=0;
  for(const char* p=s;*p;++p){
    if(p-s>=3 || *p<'0' || *p>'9')return false;
    value=value*10u+unsigned(*p-'0');
  }
  out=value;return true;
}
struct Appearance {
  uint8_t brightness=100; // Software eye pixel intensity; not controller backlight setting
  uint8_t gazeActivity=100; // Autonomous motion intensity only, not remote/touch gestures
  bool blink=true;
  void sanitize() {
    if(!validBrightness(brightness))brightness=100;
    if(!validGazeActivity(gazeActivity))gazeActivity=100;
  }
};
}} // namespace stach::modes
