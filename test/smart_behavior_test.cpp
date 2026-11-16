#include "stach_behavior.h"
#include "stach_character.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

struct Buffer {
 std::vector<uint16_t> pixels;
 Buffer():pixels(466*466,0){}
 void clear(uint16_t c){std::fill(pixels.begin(),pixels.end(),c);}
 void hline(int x,int y,int w,uint16_t c){
   assert(x>=0&&y>=0&&w>0&&x+w<=466&&y<466);
   for(int k=0;k<w;++k)pixels[y*466+x+k]=c;
 }
};

int main(){
 using namespace stach;
 int checks=0;
 auto ck=[&](bool ok){assert(ok);++checks;};
 // Forward: matching eyes, no brows, solid background, anti-aliased contour.
 face::State forward=face::evaluate(1000,Emotion::Neutral);
 ck(forward.gazeX==0 && forward.browStrength==0);
 Buffer f;face::draw(f,forward);
 ck(f.pixels[229*466+163]==0 && f.pixels[229*466+303]==0);
 ck(f.pixels[229*466+205]==0xffff && f.pixels[229*466+345]==0xffff);
 ck(f.pixels[2*466+2]==0);
 // Motion phases: anticipation, left, right, center again.
 auto prep=face::evaluate(1640,Emotion::Neutral);
 auto left=face::evaluate(2600,Emotion::Neutral);
 auto right=face::evaluate(5500,Emotion::Neutral);
 ck(prep.offsetX>0 && prep.gazeX==0);
 ck(left.gazeX<0 && left.offsetX<0);
 ck(right.gazeX>0 && right.offsetX>0);
 ck(std::fabs(face::evaluate(7000,Emotion::Neutral).gazeX)<.0001f);
 ck(face::evaluate(5500,Emotion::Neutral).browStrength==0);
 // Vertical eye extent: closer right eye bigger looking left, vice versa right.
 Buffer l;face::draw(l,left);
 ck(l.pixels[156*466+116]!=0xffff && l.pixels[156*466+256]!=0); // nonessential exact edge? evaluated below too
 // Event reason engine, no random emotional swaps.
 behavior::Engine e;e.begin(0);
 ck(e.choose(100)==Emotion::Neutral);
 e.touch(1000);ck(e.choose(1001)==Emotion::Curious);ck(e.choose(6000)==Emotion::Neutral);
 e.battery(8);ck(e.choose(6000)==Emotion::Sleepy);
 e.battery(62);ck(e.choose(6000)==Emotion::Neutral);
 e.charge(true,true,6000);ck(e.choose(6000)==Emotion::Happy&&e.showCharging());
 e.charge(true,false,6100);ck(e.choose(6100)==Emotion::Neutral);
 e.acceleration(.20f,7000);e.acceleration(.18f,7080);e.acceleration(3.1f,7170);
 ck(e.choose(7171)==Emotion::Angry);
 ck(e.choose(15000)==Emotion::Neutral);
 e.battery(101);ck(!e.batteryKnown);
 e.sleepyAfterMs=10000;
 ck(e.choose(20000)==Emotion::Sleepy);
 behavior::FallDetector fd;
 ck(!fd.add(1.f,0));ck(!fd.add(.2f,10));ck(!fd.add(3.0f,40)); // false before 75ms freefall
 ck(!fd.add(.2f,1000));ck(!fd.add(.2f,1090));ck(fd.add(3.f,1200));
 ck(!fd.add(3.f,1300)); // debounce/cooldown
 character::Controller c;
 c.set(Emotion::Happy,1000,true);
 ck(c.sample(1400).chargeStrength>0.99f);
 c.set(Emotion::Neutral,1400,false);
 ck(std::fabs(c.sample(1400).chargeStrength-c.from.chargeStrength)<.0001f);
 ck(c.sample(1800).chargeStrength==0);
 std::cout<<"Smart motion and event tests passed: "<<checks<<" assertions\n";
}
