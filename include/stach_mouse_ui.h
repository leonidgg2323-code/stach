#pragma once
#ifdef STACH_WAVESHARE_BOARD
#include <Arduino.h>
#include "stach_display.h"
#include "stach_mouse.h"
namespace stach { namespace mouse_ui {
inline void draw(const mouse::Engine& engine,bool advertising,bool imuReady) {
  Arduino_GFX* p=display::panel();
  p->fillScreen(0x0000);
  p->setTextColor(0xFFFF);p->setTextSize(3);
  p->setCursor(106,84);p->print("MOUSE");
  p->drawRoundRect(46,143,374,184,24,0x630C);
  p->setTextSize(2);
  p->setCursor(78,169);
  if(!engine.connected()) {p->print(advertising?"PAIR VIA PC BT":"STARTING BLE...");}
  else if(!imuReady) p->print("GYRO NOT FOUND");
  else if(!engine.calibrated()) p->print("HOLD DEVICE STILL");
  else p->print("CONNECTED   READY");
  p->setTextColor(0x9CF3);
  p->setCursor(78,207);p->print("Tap: left click");
  p->setCursor(78,238);p->print("2x tap: right");
  p->setCursor(78,269);p->print("Swipe: scroll");
  p->setTextColor(0xFFFF);
  p->setCursor(125,358);p->print("HOLD TO EXIT");
  p->setTextSize(1);
}
}} // namespace stach::mouse_ui
#endif
