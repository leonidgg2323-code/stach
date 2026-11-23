#pragma once
#include <stdint.h>

// A boot-time software snapshot. "present" means ACK/driver init only;
// it is NOT proof that a peripheral works under real-world use.
// No claim of physical validation is made by this code.
namespace stach { namespace hwreport {
constexpr uint32_t FLASH_EXPECTED_BYTES = 16u * 1024u * 1024u;
constexpr uint32_t PSRAM_MIN_BYTES = 7u * 1024u * 1024u;

enum Warning : uint16_t {
  FlashMismatch = 1u << 0,
  PsramUnavailable = 1u << 1,
  I2CBusUnavailable = 1u << 2,
  TouchNotResponding = 1u << 3,
  ImuNotResponding = 1u << 4,
  RtcNotResponding = 1u << 5,
  DisplayInitFailed = 1u << 6,
  TouchDriverFailed = 1u << 7,
  ImuDriverFailed = 1u << 8,
};
constexpr uint16_t CRITICAL = FlashMismatch | PsramUnavailable |
  I2CBusUnavailable | TouchNotResponding | ImuNotResponding |
  DisplayInitFailed | TouchDriverFailed | ImuDriverFailed;

struct Evidence {
  uint32_t flashBytes = 0;
  uint32_t psramBytes = 0;
  bool i2cBus = false;
  bool touchAck = false;
  bool imuAck = false;
  bool rtcAck = false;
  bool displayBegin = false;
  bool touchBegin = false;
  bool imuBegin = false;
};
struct Report {
  Evidence evidence{};
  uint16_t warnings = 0;
  bool basicDriversReady = false;
};
inline Report evaluate(const Evidence& e) {
  Report r;
  r.evidence = e;
  if (e.flashBytes != FLASH_EXPECTED_BYTES) r.warnings |= FlashMismatch;
  if (e.psramBytes < PSRAM_MIN_BYTES) r.warnings |= PsramUnavailable;
  if (!e.i2cBus) r.warnings |= I2CBusUnavailable;
  if (!e.touchAck) r.warnings |= TouchNotResponding;
  if (!e.imuAck) r.warnings |= ImuNotResponding;
  if (!e.rtcAck) r.warnings |= RtcNotResponding; // diagnostic, not a boot blocker
  if (!e.displayBegin) r.warnings |= DisplayInitFailed;
  if (!e.touchBegin) r.warnings |= TouchDriverFailed;
  if (!e.imuBegin) r.warnings |= ImuDriverFailed;
  r.basicDriversReady = (r.warnings & CRITICAL) == 0;
  return r;
}
}}
