#pragma once
#include <stdint.h>
#include <string.h>
#include "stach_touch.h"

namespace stach { namespace pairing {
enum class Screen : uint8_t { Face, Wifi, Portal };
inline const char* name(Screen screen) {
  switch (screen) {
    case Screen::Face: return "face";
    case Screen::Wifi: return "wifi";
    case Screen::Portal: return "portal";
  }
  return "face";
}
inline bool parseScreen(const char* s, Screen& out) {
  if (!s) return false;
  if (strcmp(s, "face") == 0) { out = Screen::Face; return true; }
  if (strcmp(s, "wifi") == 0) { out = Screen::Wifi; return true; }
  if (strcmp(s, "portal") == 0) { out = Screen::Portal; return true; }
  return false;
}
// Overflow-safe timestamps, no secrets in this UI state.
struct Controller {
  static constexpr uint32_t timeoutMs = 90000;
  Screen screen = Screen::Face;
  uint32_t lastInteraction = 0;
  bool show(Screen next, uint32_t now) {
    const bool changed = screen != next;
    screen = next;
    lastInteraction = now;
    return changed;
  }
  bool gesture(touch::Gesture kind, uint32_t now) {
    switch (kind) {
      case touch::Gesture::LongPress:
        return show(screen == Screen::Face ? Screen::Wifi : Screen::Face, now);
      case touch::Gesture::DoubleTap:
        if (screen != Screen::Face) return show(Screen::Face, now);
        return false;
      case touch::Gesture::Tap:
      case touch::Gesture::SwipeRight:
      case touch::Gesture::SwipeLeft:
        if (screen == Screen::Wifi) return show(Screen::Portal, now);
        if (screen == Screen::Portal) return show(Screen::Wifi, now);
        return false;
      default: return false;
    }
  }
  bool tick(uint32_t now) {
    if (screen == Screen::Face) return false;
    if (static_cast<uint32_t>(now - lastInteraction) >= timeoutMs)
      return show(Screen::Face, now);
    return false;
  }
};
// Pure layout: version-7 QR + a real four-module quiet zone.
// 45 modules + 4 on each side = 53; 5 px/module => 265 px.
struct Layout {
  static constexpr int panelWidth = 466;
  static constexpr int version = 7;
  static constexpr int modules = 4 * version + 17;
  static constexpr int quiet = 4;
  static constexpr int pitch = 5;
  static constexpr int total = (modules + 2 * quiet) * pitch;
  static constexpr int origin = (panelWidth - total) / 2;
  static constexpr int modulesStart = origin + quiet * pitch;
};
static_assert(Layout::modules == 45, "QR version layout error");
static_assert(Layout::total == 265, "QR should fit the circle");
static_assert(Layout::total * Layout::total * 2 < Layout::panelWidth * Layout::panelWidth,
              "QR corners must stay inside round panel");
}}
