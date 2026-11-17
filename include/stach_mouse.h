#pragma once
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "stach_motion.h"
#include "stach_touch.h"

// Portable, hardware-free BLE mouse policy. The BLE transport is in
// stach_ble_mouse.h; this file can be tested with an ordinary C++ compiler.
namespace stach { namespace mouse {
inline float limit(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline bool validSensitivity(float v) { return std::isfinite(v) && v >= 0.5f && v <= 4.0f; }
inline bool parseSensitivity(const char* text, float& out) {
  if (!text || !*text || std::strlen(text) > 12) return false;
  for(const char* p=text;*p;++p) if((*p < '0' || *p > '9') && *p != '.') return false;
  char* end = nullptr;
  const float v = std::strtof(text, &end);
  if (end == text || *end || !validSensitivity(v)) return false;
  out = v;
  return true;
}
inline bool parseDelta(const char* text, int8_t& out) {
  if (!text || !*text || std::strlen(text) > 5) return false;
  for(const char* p=text;*p;++p) if((*p < '0' || *p > '9') && !(p==text && (*p=='+' || *p=='-'))) return false;
  char* end = nullptr;
  long v = std::strtol(text, &end, 10);
  if (end == text || *end || v < -40 || v > 40) return false;
  out = static_cast<int8_t>(v);
  return true;
}
struct Config {
  float sensitivity = 1.25f;
  bool invertX = false;
  bool invertY = false;
  // 'zx': horizontal cursor movement from gyro Z; 'yx': gyro Y.
  // Vertical movement is always from gyro X. Change to suit board mounting.
  bool axisYX = false;
};
enum class Button : uint8_t { None=0, Left=1, Right=2, Middle=4 };
enum class Action : uint8_t { None, LeftClick, RightClick, ScrollUp, ScrollDown, ScrollLeft, ScrollRight, Exit };
inline Action gestureAction(touch::Gesture g) {
  switch(g) {
    case touch::Gesture::Tap: return Action::LeftClick;
    case touch::Gesture::DoubleTap: return Action::RightClick;
    case touch::Gesture::SwipeUp: return Action::ScrollUp;
    case touch::Gesture::SwipeDown: return Action::ScrollDown;
    case touch::Gesture::SwipeLeft: return Action::ScrollLeft;
    case touch::Gesture::SwipeRight: return Action::ScrollRight;
    case touch::Gesture::LongPress: return Action::Exit;
    default: return Action::None;
  }
}
struct Delta { int8_t x=0, y=0; bool ready=false; };

class Engine {
  bool mode_ = false;
  bool connected_ = false;
  bool calibrated_ = false;
  bool hasLastTime_ = false;
  int calibrationCount_ = 0;
  float biasX_ = 0, biasY_ = 0, sumX_ = 0, sumY_ = 0;
  float smoothX_ = 0, smoothY_ = 0, remainderX_ = 0, remainderY_ = 0;
  uint32_t lastTime_ = 0;
  uint32_t calibrationAt_ = 0;
  static constexpr int kCalibrationSamples = 48;
  static constexpr float kStillThreshold = 7.f; // deg/s; only calibrate while still
  void restartCalibration(uint32_t now) {
    calibrated_ = false;
    calibrationCount_ = 0;
    sumX_ = sumY_ = 0;
    smoothX_ = smoothY_ = remainderX_ = remainderY_ = 0;
    hasLastTime_ = false;
    calibrationAt_ = now;
  }
  float rawX(const motion::Sample& s) const { return config.axisYX ? s.gy : s.gz; }
  float rawY(const motion::Sample& s) const { return s.gx; }
public:
  Config config{};
  bool active() const { return mode_; }
  bool connected() const { return connected_; }
  bool calibrated() const { return calibrated_; }
  int calibrationSamples() const { return calibrationCount_; }
  void enable(bool on, uint32_t now) {
    if(mode_ == on) return;
    mode_ = on;
    restartCalibration(now);
  }
  void link(bool connected, uint32_t now) {
    if(connected_ == connected) return;
    connected_ = connected;
    restartCalibration(now); // prevent stale cursor jumps after reconnection
  }
  void recalibrate(uint32_t now) { restartCalibration(now); }
  void configure(const Config& newConfig, uint32_t now) {
    if(!validSensitivity(newConfig.sensitivity)) return;
    const bool axesChanged = newConfig.axisYX != config.axisYX ||
      newConfig.invertX != config.invertX || newConfig.invertY != config.invertY;
    config = newConfig;
    if(axesChanged) restartCalibration(now);
  }
  Delta update(const motion::Sample& s, uint32_t now) {
    Delta none;
    if(!mode_ || !connected_) return none;
    const float x = rawX(s), y = rawY(s);
    const float magnitude = std::sqrt(s.ax*s.ax + s.ay*s.ay + s.az*s.az);
    if(!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(magnitude)) {
      restartCalibration(now); return none;
    }
    if(!calibrated_) {
      // Gyro bias must be measured at rest, not while the user is turning Stach.
      if(std::fabs(x) > kStillThreshold || std::fabs(y) > kStillThreshold ||
         magnitude < 0.75f || magnitude > 1.25f) {
        calibrationCount_ = 0; sumX_ = sumY_ = 0; calibrationAt_ = now;
        return none;
      }
      sumX_ += x; sumY_ += y; ++calibrationCount_;
      if(calibrationCount_ >= kCalibrationSamples) {
        biasX_ = sumX_ / calibrationCount_;
        biasY_ = sumY_ / calibrationCount_;
        calibrated_ = true;
        lastTime_ = now;
        hasLastTime_ = true;
      }
      return none;
    }
    if(!hasLastTime_) { lastTime_ = now; hasLastTime_ = true; return none; }
    uint32_t age = static_cast<uint32_t>(now - lastTime_);
    lastTime_ = now;
    if(age < 5 || age > 100) {
      smoothX_=smoothY_=remainderX_=remainderY_=0;
      return none;
    }
    float vx = x - biasX_, vy = y - biasY_;
    // Small gyro noise is ignored. Use no stale accumulator when stationary.
    if(std::fabs(vx) < 2.5f) vx=0;
    if(std::fabs(vy) < 2.5f) vy=0;
    smoothX_ = smoothX_*0.58f + vx*0.42f;
    smoothY_ = smoothY_*0.58f + vy*0.42f;
    float px = (config.invertX ? -smoothX_ : smoothX_) * config.sensitivity * 3.0f * float(age)/1000.f;
    float py = (config.invertY ? -smoothY_ : smoothY_) * config.sensitivity * 3.0f * float(age)/1000.f;
    // Clamp and DISCARD overflow so a fast spin cannot create a delayed cursor march.
    px=limit(px,-32.f,32.f);py=limit(py,-32.f,32.f);
    remainderX_=limit(remainderX_+px,-32.f,32.f);
    remainderY_=limit(remainderY_+py,-32.f,32.f);
    const int dx = static_cast<int>(remainderX_); // truncates toward zero
    const int dy = static_cast<int>(remainderY_);
    remainderX_ -= dx; remainderY_ -= dy;
    if(dx==0 && dy==0) return none;
    return {static_cast<int8_t>(dx),static_cast<int8_t>(dy),true};
  }
};
}} // stach::mouse
