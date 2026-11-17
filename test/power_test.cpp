#include "stach_power.h"
#include <cassert>
#include <iostream>
#include <cmath>
using namespace stach::power;
int main(){
  int n=0;auto check=[&](bool v){assert(v);++n;};
  Meter m;
  check(!m.known());check(m.batteryPercent==-1);check(m.adcMilliVolts==-1);
  check(!validRatio(0));check(!validRatio(NAN));check(!validRatio(INFINITY));
  check(!validRatio(.9f));check(!validRatio(10.1f));check(validRatio(2));
  float ratio=0;
  check(!Meter::measuredRatio(2000,2000,ratio));
  check(!Meter::measuredRatio(4200,0,ratio));
  check(!Meter::measuredRatio(4500,2000,ratio));
  check(Meter::measuredRatio(4000,2000,ratio));check(ratio==2.0f);
  m.observe(2000);check(!m.known());check(m.adcMilliVolts==2000);
  m.configure(ratio);check(!m.known());
  m.observe(2000);check(!m.known());m.observe(2000);check(!m.known());
  m.observe(2000);check(m.known());check(m.batteryMilliVolts==4000);
  check(m.batteryPercent==78);check(percentFromCellMv(3300)==0);check(percentFromCellMv(4200)==100);
  int last=-1;for(int v=3100;v<=4300;++v){const int pct=percentFromCellMv(v);check(pct>=last);last=pct;}
  m.observe(0);check(!m.known());check(m.batteryPercent==-1);
  m.configure(-1);m.observe(2000);m.observe(2000);m.observe(2000);check(!m.known());
  m.configure(2.0f);m.observe(3000);m.observe(3000);m.observe(3000);check(!m.known());
  Saver s;s.activity(1000);
  check(s.tick(45000,false)==ScreenMode::Normal);
  check(s.tick(47000,false)==ScreenMode::Eco);
  check(s.tick(303000,false)==ScreenMode::Dark);
  check(s.tick(303001,true)==ScreenMode::Normal);
  s.activity(303002);check(s.tick(303003,false)==ScreenMode::Normal);
  check(s.tick(316000,false,true,8)==ScreenMode::Eco);
  check(s.tick(424000,false,true,8)==ScreenMode::Dark);
  s.enabled=false;check(s.tick(999999,false)==ScreenMode::Normal);
  check(Saver::framePeriod(ScreenMode::Normal)==100);
  check(Saver::framePeriod(ScreenMode::Eco)==400);
  check(Saver::framePeriod(ScreenMode::Dark)==0);
  s.enabled=true;s.activity(UINT32_MAX-200);
  check(s.tick(100,false)==ScreenMode::Normal);
  std::cout << "STACH battery/power policy passed: "<<n<<" assertions\n";
}
