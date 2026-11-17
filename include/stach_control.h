#pragma once
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "stach_face.h"

namespace stach { namespace control {
// Validated input for HTTP commands. Do not silently accept NaN or garbage.
inline bool parseAxis(const char* s, float& out) {
  if (!s || !*s || std::strlen(s) > 12) return false;
  char* end = nullptr;
  const float value = std::strtof(s, &end);
  if (end == s || *end != '\0' || !std::isfinite(value) || value < -1.f || value > 1.f) return false;
  out = value;
  return true;
}
inline bool parseSwitch(const char* s, bool& out) {
  if (s && std::strcmp(s, "1") == 0) { out = true; return true; }
  if (s && std::strcmp(s, "0") == 0) { out = false; return true; }
  return false;
}

struct Settings {
  bool idleMotion = true;
  bool tiltEnabled = true;
};

struct RemoteGaze {
  float x = 0.f, y = 0.f;
  uint32_t since = 0;
  static constexpr uint32_t durationMs = 2400;
  bool enabled = false;

  bool set(float xNorm, float yNorm, uint32_t now) {
    if (!std::isfinite(xNorm) || !std::isfinite(yNorm) ||
        xNorm < -1.f || xNorm > 1.f || yNorm < -1.f || yNorm > 1.f) return false;
    x = xNorm; y = yNorm; since = now; enabled = true;
    return true;
  }
  void clear() { x = 0.f; y = 0.f; enabled = false; }
  bool active(uint32_t now) const {
    return enabled && static_cast<uint32_t>(now - since) < durationMs;
  }
  void apply(stach::face::State& s, uint32_t now) const {
    if (!active(now)) return;
    const uint32_t age = static_cast<uint32_t>(now - since);
    const float in = stach::face::ease(stach::face::clamp(float(age) / 180.f, 0.f, 1.f));
    const float out = stach::face::ease(stach::face::clamp(float(durationMs - age) / 400.f, 0.f, 1.f));
    const float w = in * out;
    s.gazeX = s.gazeX * (1.f - w) + x * w;
    s.gazeY = s.gazeY * (1.f - w) + y * w;
    // Never send the whole face to the screen edge.
    s.offsetX = stach::face::clamp(s.offsetX * (1.f - w) + x * 18.f * w,
                   -stach::face::MAX_EYE_SHIFT_X, stach::face::MAX_EYE_SHIFT_X);
    s.offsetY = stach::face::clamp(s.offsetY * (1.f - w) + y * 10.f * w,
                   -stach::face::MAX_EYE_SHIFT_Y, stach::face::MAX_EYE_SHIFT_Y);
  }
};
}} // namespace stach::control
