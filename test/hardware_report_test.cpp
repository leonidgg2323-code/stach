#include "stach_hw_report.h"
#include <cassert>
#include <iostream>
using namespace stach::hwreport;
int main(){
  int assertions=0;
  auto ck=[&](bool ok){assert(ok);++assertions;};
  Evidence good;
  good.flashBytes=16u*1024u*1024u;
  good.psramBytes=8u*1024u*1024u;
  good.i2cBus=true;
  good.touchAck=true;
  good.imuAck=true;
  good.rtcAck=true;
  good.displayBegin=true;
  good.touchBegin=true;
  good.imuBegin=true;
  Report r=evaluate(good);
  ck(r.warnings==0 && r.basicDriversReady);
  auto e=good;e.flashBytes=8u*1024u*1024u;r=evaluate(e);
  ck((r.warnings&FlashMismatch)!=0);ck(!r.basicDriversReady);
  e=good;e.psramBytes=0;r=evaluate(e);
  ck((r.warnings&PsramUnavailable)!=0);ck(!r.basicDriversReady);
  e=good;e.psramBytes=6u*1024u*1024u;r=evaluate(e);
  ck((r.warnings&PsramUnavailable)!=0);
  e=good;e.i2cBus=false;r=evaluate(e);
  ck((r.warnings&I2CBusUnavailable)!=0);ck(!r.basicDriversReady);
  e=good;e.touchAck=false;r=evaluate(e);
  ck((r.warnings&TouchNotResponding)!=0);ck(!r.basicDriversReady);
  e=good;e.imuAck=false;r=evaluate(e);
  ck((r.warnings&ImuNotResponding)!=0);ck(!r.basicDriversReady);
  e=good;e.rtcAck=false;r=evaluate(e);
  ck((r.warnings&RtcNotResponding)!=0);ck(r.basicDriversReady);
  e=good;e.displayBegin=false;r=evaluate(e);
  ck((r.warnings&DisplayInitFailed)!=0);ck(!r.basicDriversReady);
  e=good;e.touchBegin=false;r=evaluate(e);
  ck((r.warnings&TouchDriverFailed)!=0);ck(!r.basicDriversReady);
  e=good;e.imuBegin=false;r=evaluate(e);
  ck((r.warnings&ImuDriverFailed)!=0);ck(!r.basicDriversReady);
  e=good;e.flashBytes=0;e.touchBegin=false;e.rtcAck=false;r=evaluate(e);
  ck((r.warnings&(FlashMismatch|RtcNotResponding|TouchDriverFailed))==(FlashMismatch|RtcNotResponding|TouchDriverFailed));
  ck(!r.basicDriversReady);
  ck((CRITICAL&RtcNotResponding)==0);
  ck((CRITICAL&TouchDriverFailed)!=0);
  std::cout << "Hardware report tests passed: "<<assertions<<" checks\n";
}
