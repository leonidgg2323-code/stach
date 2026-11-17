#include "stach_control.h"
#include <cassert>
#include <cstdio>
#include <cmath>
#include <initializer_list>

int main() {
  using namespace stach::control;
  float out = 99;
  int assertions = 0;
  auto ck = [&](bool v) { assert(v); ++assertions; };
  for (const char* s : {"-1", "1", "0", "-0.75", "0.25", "0.0001"}) ck(parseAxis(s, out));
  for (const char* s : {"", "2", "-1.1", "1.1", "NaN", "nan", "inf", "-inf", "hello", "  ", "0oops", "1.0  ", "111111111111111"}) ck(!parseAxis(s, out));
  ck(!parseAxis(nullptr, out));
  bool value = false;
  ck(parseSwitch("1", value) && value);
  ck(parseSwitch("0", value) && !value);
  for (const char* s : {"yes", "", "false", "2", " 1", "1 "}) ck(!parseSwitch(s, value));
  ck(!parseSwitch(nullptr, value));
  RemoteGaze g;
  ck(!g.active(0));
  ck(!g.set(NAN, 0, 10));
  ck(!g.set(0, INFINITY, 10));
  ck(!g.set(1.01f, 0, 10));
  ck(!g.set(0, -1.01f, 10));
  ck(g.set(-1.f, .8f, 1000));
  ck(g.active(1100));
  stach::face::State s = stach::face::evaluate(0, stach::Emotion::Neutral);
  g.apply(s, 1300);
  ck(s.gazeX < -.95f && s.gazeY > .75f);
  ck(s.offsetX >= -stach::face::MAX_EYE_SHIFT_X && s.offsetX <= stach::face::MAX_EYE_SHIFT_X);
  ck(s.offsetY >= -stach::face::MAX_EYE_SHIFT_Y && s.offsetY <= stach::face::MAX_EYE_SHIFT_Y);
  ck(!g.active(3400));
  stach::face::State defaultFace;
  g.apply(defaultFace, 3400);
  ck(defaultFace.gazeX == 0);
  ck(g.set(1.f, -1.f, UINT32_MAX - 100));
  ck(g.active(80)); // rollover must not expire early
  ck(!g.active(2400));
  g.clear();
  ck(!g.active(100));
  // Fuzz bounded sensor/HTTP gaze inputs; never violate safe travel limits.
  for (int ix = -10; ix <= 10; ++ix)
    for (int iy = -10; iy <= 10; ++iy) {
      ck(g.set(ix / 10.f, iy / 10.f, 1234));
      auto f = stach::face::evaluate(0, stach::Emotion::Neutral);
      g.apply(f, 1500);
      ck(std::fabs(f.gazeX) <= 1.f && std::fabs(f.gazeY) <= 1.f);
      ck(std::fabs(f.offsetX) <= stach::face::MAX_EYE_SHIFT_X);
      ck(std::fabs(f.offsetY) <= stach::face::MAX_EYE_SHIFT_Y);
    }
  std::printf("Stage-7 remote control: %d assertions passed\n", assertions);
}
