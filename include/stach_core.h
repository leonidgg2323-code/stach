#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace stach {
enum class Emotion : uint8_t { Neutral, Angry, Sleepy, Curious, Happy, Invalid };
inline Emotion parseEmotion(const char* value) {
  if (!value) return Emotion::Invalid;
  if (strcmp(value, "neutral") == 0) return Emotion::Neutral;
  if (strcmp(value, "angry") == 0) return Emotion::Angry;
  if (strcmp(value, "sleepy") == 0) return Emotion::Sleepy;
  if (strcmp(value, "curious") == 0) return Emotion::Curious;
  if (strcmp(value, "happy") == 0) return Emotion::Happy;
  return Emotion::Invalid;
}
inline const char* emotionName(Emotion e) {
  switch (e) {
    case Emotion::Neutral: return "neutral";
    case Emotion::Angry: return "angry";
    case Emotion::Sleepy: return "sleepy";
    case Emotion::Curious: return "curious";
    case Emotion::Happy: return "happy";
    default: return "invalid";
  }
}
inline bool validPassword(const char* password) {
  if (!password) return false;
  size_t n = strlen(password);
  if (n < 12 || n > 63) return false;
  for (size_t i = 0; i < n; ++i) if (password[i] < 33 || password[i] > 126) return false;
  return true;
}
inline bool wifiQr(char* out, size_t capacity, const char* ssid, const char* password) {
  if (!out || capacity == 0 || !ssid || !password || !validPassword(password)) return false;
  // SSID is device-generated and restricted to Stach-[A-F0-9]; password is hex.
  if (strncmp(ssid, "Stach-", 6) != 0 || strlen(ssid) != 12) return false;
  for (size_t i = 6; i < 12; ++i)
    if (!((ssid[i] >= '0' && ssid[i] <= '9') || (ssid[i] >= 'A' && ssid[i] <= 'F'))) return false;
  int n = snprintf(out, capacity, "WIFI:T:WPA;S:%s;P:%s;;", ssid, password);
  return n > 0 && static_cast<size_t>(n) < capacity;
}
}
