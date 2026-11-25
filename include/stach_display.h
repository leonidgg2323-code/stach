#pragma once
#ifdef STACH_WAVESHARE_BOARD
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "stach_board.h"
#ifdef STACH_EYES_175
#include <esp_heap_caps.h>
#endif

namespace stach { namespace display {
// Tested reference hardware setup from thelastoutpostworkshop's Waveshare
// ESP32-S3-Touch-AMOLED-1.43 game (CO5300, 466x466, col gap 6).
// LCD ID / physical panel has NOT been established for this specific unit.
inline Arduino_DataBus* bus() {
  static Arduino_DataBus* b = new Arduino_ESP32QSPI(
    board::qspi_cs, board::qspi_clk,
    board::qspi_d0, board::qspi_d1, board::qspi_d2, board::qspi_d3, false);
  return b;
}

inline Arduino_GFX* output() {
#if defined(STACH_EYES_175)
  static Arduino_GFX* p = new Arduino_CO5300(bus(),board::amoled_reset,0,
    board::width,board::height,6,0,0,0);
#elif defined(STACH_CANVAS_CO5300)
  static Arduino_GFX* p = new Arduino_CO5300(bus(),board::amoled_reset,0,
    board::width,board::height,6,0,6,0);
#elif defined(STACH_PANEL_CO5300)
  static Arduino_GFX* p = new Arduino_CO5300(bus(),board::amoled_reset,0,
    board::width,board::height);
#else
  static Arduino_GFX* p = new Arduino_SH8601(bus(),board::amoled_reset,0,
    board::width,board::height);
#endif
  return p;
}

inline Arduino_GFX* panel() {
#if defined(STACH_CANVAS_CO5300)
  static Arduino_GFX* p = new Arduino_Canvas(board::width,board::height,output());
  return p;
#else
  return output();
#endif
}

inline bool begin() {
#ifndef STACH_EYES_175
  pinMode(board::amoled_enable, OUTPUT);
  digitalWrite(board::amoled_enable, HIGH);
  delay(150);
#endif
#if defined(STACH_EYES_175)
  const bool started = panel()->begin();
  if(started) static_cast<Arduino_CO5300*>(output())->setBrightness(128);
#elif defined(STACH_CANVAS_CO5300)
  // The reference uses 80 MHz and Arduino_Canvas->begin() to initialise the output.
  const bool started = panel()->begin(80000000);
#else
  const bool started = panel()->begin();
#endif
  if(!started) return false;
  panel()->setRotation(0);
  panel()->fillScreen(0x0000);
  panel()->flush();  // Required for buffered canvas output.
  Serial.println("[STACH-CANVAS] CO5300 Canvas init commands sent; display not verified");
  return true;
}

inline void flush() {
#if defined(STACH_EYES_175)
  // The canvas and panel use the same RGB565 row order. Send just the
  // contiguous bands that changed since the previous presentation.
  constexpr int w=board::width, h=board::height, band=8;
  static auto* previous=static_cast<uint16_t*>(
    heap_caps_calloc(w*h,sizeof(uint16_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT));
  auto* pixels=static_cast<Arduino_Canvas*>(panel())->getFramebuffer();
  if (!previous) { panel()->flush(); return; }
  int first=-1;
  for(int y=0; y<h; y+=band) {
    const int end=min(y+band,h);
    const size_t bytes=(end-y)*w*sizeof(uint16_t);
    const bool changed=memcmp(pixels+y*w,previous+y*w,bytes)!=0;
    if(changed && first<0) first=y;
    if(first>=0 && (!changed || end==h)) {
      const int lines=(changed ? end : y)-first;
      output()->draw16bitRGBBitmap(0,first,pixels+first*w,w,lines);
      memcpy(previous+first*w,pixels+first*w,lines*w*sizeof(uint16_t));
      first=-1;
    }
  }
#elif defined(STACH_CANVAS_CO5300)
  panel()->flush();
#endif
}

inline void diagnosticPattern() {
  Arduino_GFX* p=panel();
  p->fillScreen(0x0000);
  p->drawCircle(board::width/2,board::height/2,170,0xFFFF);
  p->fillCircle(175,220,45,0xFFFF);
  p->fillCircle(290,220,45,0xFFFF);
  p->fillCircle(175,220,20,0x0000);
  p->fillCircle(290,220,20,0x0000);
  p->fillRect(139,58,58,24,0xF800);
  p->fillRect(204,58,58,24,0x07E0);
  p->fillRect(269,58,58,24,0x001F);
  flush();
}
}} // stach::display
#endif
