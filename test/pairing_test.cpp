#include "stach_pairing.h"
#include "stach_core.h"
#include <cassert>
#include <cstring>
#include <iostream>

int main() {
  using namespace stach;
  using pairing::Screen;
  int checks = 0;
  auto check = [&](bool condition) { assert(condition); ++checks; };
  pairing::Controller ui;
  check(ui.screen == Screen::Face);
  check(!ui.gesture(touch::Gesture::Tap, 3));
  check(ui.screen == Screen::Face);
  check(ui.gesture(touch::Gesture::LongPress, 5));
  check(ui.screen == Screen::Wifi);
  check(!ui.tick(100));
  check(ui.gesture(touch::Gesture::Tap, 120));
  check(ui.screen == Screen::Portal);
  check(ui.gesture(touch::Gesture::SwipeLeft, 150));
  check(ui.screen == Screen::Wifi);
  check(ui.gesture(touch::Gesture::SwipeRight, 160));
  check(ui.screen == Screen::Portal);
  check(ui.gesture(touch::Gesture::DoubleTap, 165));
  check(ui.screen == Screen::Face);
  check(ui.gesture(touch::Gesture::LongPress, 200));
  check(!ui.tick(200 + pairing::Controller::timeoutMs - 1));
  check(ui.tick(200 + pairing::Controller::timeoutMs));
  check(ui.screen == Screen::Face);
  check(!ui.tick(500000));
  check(ui.show(Screen::Wifi, 0xfffffff0U));
  check(!ui.tick(0x00000010U)); // uint32 rollover safe
  check(ui.tick(0xfffffff0U + pairing::Controller::timeoutMs));
  Screen parsed = Screen::Face;
  check(pairing::parseScreen("wifi", parsed) && parsed == Screen::Wifi);
  check(pairing::parseScreen("portal", parsed) && parsed == Screen::Portal);
  check(pairing::parseScreen("face", parsed) && parsed == Screen::Face);
  check(!pairing::parseScreen("surprised", parsed));
  check(!pairing::parseScreen(nullptr, parsed));
  check(pairing::Layout::origin > 0 && pairing::Layout::modulesStart > pairing::Layout::origin);
  check(pairing::Layout::total == 265);
  check(pairing::Layout::modules == 45);
  char data[128]{};
  check(wifiQr(data, sizeof(data), "Stach-A1B2C3", "0123456789ABCDEF"));
  check(std::strcmp(data, "WIFI:T:WPA;S:Stach-A1B2C3;P:0123456789ABCDEF;;") == 0);
  check(!wifiQr(data, 10, "Stach-A1B2C3", "0123456789ABCDEF"));
  std::cout << "Pairing UI and Wi-Fi QR payload: " << checks << " checks passed\n";
}
