#pragma once
#include <stdint.h>
#include <math.h>

// Portable, intentionally conservative battery reporting and screen-idle policy.
// ADC voltage on GPIO4 is NOT automatically treated as cell voltage or %.
namespace stach { namespace power {
inline bool validRatio(float ratio) { return isfinite(ratio) && ratio >= 1.0f && ratio <= 10.0f; }
inline int percentFromCellMv(int mv) {
  // Approximate single-cell Li-ion OCV lookup, NOT a coulomb counter.
  // Calibration is required; load/charge voltage can bias this estimate.
  static constexpr int volts[] = {3300,3450,3600,3700,3800,3900,4000,4100,4200};
  static constexpr int pc[]    = {0,   5,   15,  30,  48,  63,  78,  91,  100};
  if (mv<=volts[0])return 0;
  if (mv>=volts[8])return 100;
  for(int i=1;i<9;++i)if(mv<=volts[i]){
    return pc[i-1]+(pc[i]-pc[i-1])*(mv-volts[i-1])/(volts[i]-volts[i-1]);
  }
  return -1;
}
struct Meter {
  float ratio = 0.f;              // Unknown until battery source checked and calibrated.
  int adcMilliVolts = -1;         // Actual ADC pin voltage; never a battery voltage.
  int batteryMilliVolts = -1;
  int batteryPercent = -1;
  float filteredAdc = 0.f;
  uint8_t goodSamples = 0;
  bool known() const { return validRatio(ratio) && goodSamples >= 3 && batteryPercent >= 0; }
  void configure(float newRatio) {
    ratio=validRatio(newRatio)?newRatio:0.f;
    batteryMilliVolts=-1;batteryPercent=-1;goodSamples=0;filteredAdc=0.f;
  }
  // Only call after a measured cell voltage and the corresponding ADC reading,
  // AND after confirming that GPIO4 tracks the CELL instead of USB/system rail.
  static bool measuredRatio(int measuredCellMv,int adcPinMv,float& out) {
    if(measuredCellMv<3200 || measuredCellMv>4250 || adcPinMv<300 || adcPinMv>3100)return false;
    const float candidate=float(measuredCellMv)/float(adcPinMv);
    if(!validRatio(candidate))return false;
    out=candidate;return true;
  }
  void observe(int adcMv) {
    // Reject invalid/noisy samples. Keep unknown until three sound readings.
    if(adcMv<200 || adcMv>3100) {
      adcMilliVolts=-1; batteryMilliVolts=-1;batteryPercent=-1;goodSamples=0;return;
    }
    adcMilliVolts=adcMv;
    if(!goodSamples) filteredAdc=float(adcMv);
    else {
      // Drop a huge transient but do not mask lasting voltage changes.
      const float alpha=fabsf(float(adcMv)-filteredAdc)>300.f?0.07f:0.18f;
      filteredAdc+=(float(adcMv)-filteredAdc)*alpha;
    }
    if(goodSamples<255)++goodSamples;
    if(!validRatio(ratio) || goodSamples<3) {
      batteryMilliVolts=-1;batteryPercent=-1;return;
    }
    const float cellMv=filteredAdc*ratio;
    if(cellMv<2800.f || cellMv>4300.f) {
      batteryMilliVolts=-1;batteryPercent=-1;return;
    }
    batteryMilliVolts=int(cellMv+0.5f);
    batteryPercent=percentFromCellMv(batteryMilliVolts);
  }
};

enum class ScreenMode : uint8_t { Normal, Eco, Dark };
inline const char* modeName(ScreenMode m) {
  return m==ScreenMode::Normal?"normal":m==ScreenMode::Eco?"eco":"dark";
}
struct Saver {
  bool enabled = true;
  uint32_t lastActivityMs = 0;
  ScreenMode current = ScreenMode::Normal;
  void activity(uint32_t now) {lastActivityMs=now;current=ScreenMode::Normal;}
  ScreenMode tick(uint32_t now, bool keepAwake, bool batteryKnown=false, int pct=-1) {
    if(!enabled || keepAwake) { current=ScreenMode::Normal;return current; }
    const bool low=batteryKnown && pct>=0 && pct<=15;
    const uint32_t idle=uint32_t(now-lastActivityMs);  // millis rollover-safe
    if(idle >= (low?120000u:300000u))current=ScreenMode::Dark;
    else if(idle >= (low?10000u:45000u))current=ScreenMode::Eco;
    else current=ScreenMode::Normal;
    return current;
  }
  static uint32_t framePeriod(ScreenMode m) {return m==ScreenMode::Normal?100u:m==ScreenMode::Eco?400u:0u;}
};
}} // namespace stach::power
