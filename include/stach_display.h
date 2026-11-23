#pragma once
#ifdef STACH_WAVESHARE_BOARD
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "stach_board.h"
namespace stach { namespace display {
// Both controller types are sold under the 1.43-inch name.
// Select one at build time; wrong selection must be corrected before flashing.
inline Arduino_DataBus* bus() {
  static Arduino_DataBus* b = new Arduino_ESP32QSPI(board::qspi_cs, board::qspi_clk,
    board::qspi_d0, board::qspi_d1, board::qspi_d2, board::qspi_d3);
  return b;
}
inline Arduino_GFX* panel() {
#if defined(STACH_PANEL_CO5300)
  static Arduino_GFX* p = new Arduino_CO5300(bus(), board::amoled_reset, 0, board::width, board::height);
#else
  static Arduino_GFX* p = new Arduino_SH8601(bus(), board::amoled_reset, 0, board::width, board::height);
#endif
  return p;
}
inline bool begin() {
  pinMode(board::amoled_enable, OUTPUT);
  digitalWrite(board::amoled_enable, HIGH);
  delay(50);
  if (!panel()->begin()) return false;
  panel()->fillScreen(0x0000);
  // Note: Arduino_GFX* has no setBrightness() method.
  // Brightness will be implemented via a board-specific validated API.
  return true;
}
inline void diagnosticPattern() {
  Arduino_GFX* p=panel();
  p->fillScreen(0x0000);
  p->drawCircle(board::width/2,board::height/2,170,0xFFFF);
  p->fillCircle(175,220,45,0xFFFF);
  p->fillCircle(290,220,45,0xFFFF);
  p->fillCircle(175,220,20,0x0000);
  p->fillCircle(290,220,20,0x0000);
  // Diagnostic RGB bars near the top: visible only on explicit USB command.
  // Keeping this separate from the everyday black-and-white eye renderer.
  p->fillRect(139,58,58,24,0xF800); // red
  p->fillRect(204,58,58,24,0x07E0); // green
  p->fillRect(269,58,58,24,0x001F); // blue
}
}}
#endif
