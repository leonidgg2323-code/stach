#pragma once
#ifdef STACH_WAVESHARE_BOARD
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "stach_power.h"
#include "stach_display.h"
namespace stach { namespace power_ui {
inline void draw(const power::Meter& m,bool chargeKnown,bool charging) {
  Arduino_GFX* g=display::panel();
  const int x=173,y=15,w=33,h=15;
  const uint16_t white=0xFFFF,green=0x07E0;
  // Battery symbol is discreet at top, NEVER covering eyes.
  g->drawRect(x,y,w,h,white);
  g->fillRect(x+w,y+5,3,5,white);
  if(m.known()){
    int fill=(w-4)*m.batteryPercent/100;
    if(fill>0)g->fillRect(x+2,y+2,fill,h-4,chargeKnown&&charging?green:white);
  }
  g->setTextSize(1);
  g->setTextColor(chargeKnown&&charging?green:white);
  g->setCursor(x+47,y+4);
  if(m.known()){
    g->print(m.batteryPercent);g->print("%");
  } else g->print("?");
  // Do not display a charging bolt unless charging is positively detected.
  if(chargeKnown&&charging){g->setCursor(x-14,y+4);g->print("+");}
  g->setTextColor(white);
}
}} // namespace stach::power_ui
#endif
