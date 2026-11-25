#pragma once
#ifdef STACH_WAVESHARE_BOARD
#include <Arduino.h>
#include <qrcode.h>
#include <string.h>
#include "stach_pairing.h"
#include "stach_display.h"

namespace stach { namespace pairing {
// Renders only when pairing screen changes; never scans or regenerates QR each frame.
// ricmoo/QRCode uses an exact version and ECC; no heap frame buffer required.
inline bool renderCode(const char* text, Screen screen, bool onboarding=false) {
  if (!text || !*text) return false;
  constexpr uint8_t kVersion = Layout::version;
  // V7/M byte capacity is 122. The longest Stach credentials payload is much shorter.
  if (strlen(text) > 122) return false;
  static uint8_t storage[512];
  if (qrcode_getBufferSize(kVersion) > sizeof(storage)) return false;
  QRCode code{};
  if (qrcode_initText(&code, storage, kVersion, ECC_MEDIUM, text) != 0 ||
      code.size != Layout::modules) return false;

  Arduino_GFX* d = display::panel();
  d->fillScreen(0x0000);
  // Positive contrast: white QR background including the 4-module quiet zone.
  d->fillRect(Layout::origin, Layout::origin, Layout::total, Layout::total, 0xffff);
  for (int y = 0; y < code.size; ++y) {
    int x = 0;
    while (x < code.size) {
      if (!qrcode_getModule(&code, x, y)) { ++x; continue; }
      const int start = x;
      do { ++x; } while (x < code.size && qrcode_getModule(&code, x, y));
      // Draw contiguous dark modules as one rect to reduce QSPI transactions.
      d->fillRect(Layout::modulesStart + start * Layout::pitch,
                  Layout::modulesStart + y * Layout::pitch,
                  (x - start) * Layout::pitch, Layout::pitch, 0x0000);
    }
  }
  d->setTextColor(0xffff, 0x0000);
  const bool wifi=screen==Screen::Wifi;
  const char* title=onboarding ? (wifi?"1/2  JOIN WI-FI":"2/2  REGISTER") : (wifi?"JOIN WI-FI":"CONTROL PANEL");
  d->setTextSize(2);
  d->setCursor((Layout::panelWidth-int(strlen(title))*12)/2,65);
  d->print(title);
  const char* hint="SCAN WITH PHONE";
  d->setCursor((Layout::panelWidth-int(strlen(hint))*12)/2,378);
  d->print(hint);
  d->setTextSize(1);
  const char* nav=wifi?"TAP FOR NEXT CODE":"TAP FOR WI-FI CODE";
  d->setCursor((Layout::panelWidth-int(strlen(nav))*6)/2,405);
  d->print(nav);
  return true;
}
inline void renderFailure() {
  Arduino_GFX* d = display::panel();
  d->fillScreen(0x0000);
  d->setTextColor(0xffff, 0x0000);
  d->setTextSize(2);
  d->setCursor(175, 225);
  d->print("QR ERROR");
}
}}
#endif
