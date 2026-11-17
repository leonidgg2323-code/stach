#pragma once
#ifdef STACH_WAVESHARE_BOARD
#include <Arduino.h>
#include <BleMouse.h>
#include "stach_mouse.h"

// ESP32 BLE HID transport. Only built for Waveshare profile.
// NOTE: T-vK BLE Mouse 0.3.1::end() is a no-op. We initialize advertising
// lazily on first Mouse Mode activation and then gate *every* HID report by
// Engine::active(). BLE advertising may remain after leaving Mouse Mode.
namespace stach { namespace ble {
class Adapter {
  BleMouse hid_{"Stach Mouse", "Stach", 100};
  bool started_ = false;
public:
  void beginOnce() {
    if(started_) return;
    started_ = true;
    hid_.begin();
  }
  bool started() const { return started_; }
  bool connected() { return started_ && hid_.isConnected(); }
  bool send(const mouse::Engine& engine, const mouse::Delta& d) {
    if(!engine.active() || !connected() || !d.ready) return false;
    hid_.move(d.x, d.y, 0, 0); return true;
  }
  bool move(const mouse::Engine& engine,int8_t x,int8_t y) {
    if(!engine.active() || !connected()) return false;
    hid_.move(x,y,0,0);return true;
  }
  bool click(const mouse::Engine& engine,mouse::Button b) {
    if(!engine.active() || !connected() || b==mouse::Button::None) return false;
    hid_.click(static_cast<uint8_t>(b));return true;
  }
  bool scroll(const mouse::Engine& engine,int8_t vertical,int8_t horizontal) {
    if(!engine.active() || !connected()) return false;
    hid_.move(0,0,vertical,horizontal);return true;
  }
};
}} // stach::ble
#endif
