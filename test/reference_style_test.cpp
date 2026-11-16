#include "stach_character.h"
#include <cassert>
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
struct PixelSurface {
 std::vector<uint16_t> pixels;
 int whites=0, blackOps=0;
 PixelSurface(): pixels(466*466,0){}
 void clear(uint16_t c){assert(c==0);std::fill(pixels.begin(),pixels.end(),c);whites=0;blackOps=0;}
 void hline(int x,int y,int w,uint16_t c){
 assert(x>=0&&y>=0&&y<466&&x+w<=466&&w>0);
  // RGB565 greyscale is permitted ONLY at antialiased shape boundaries.
  for(int i=0;i<w;++i)pixels[y*466+x+i]=c;
  if(c==0)++blackOps;else ++whites;
 }
};

int main(){
 using namespace stach;
 assert(parseEmotion("surprised")==Emotion::Invalid);
 for(auto e:{Emotion::Neutral,Emotion::Angry,Emotion::Sleepy,Emotion::Curious,Emotion::Happy}){
  for(uint32_t ms=0;ms<12000;ms+=227){
   face::State st=face::evaluate(ms,e);
   assert(std::isfinite(st.offsetX)&&std::isfinite(st.offsetY));
   PixelSurface s;face::draw(s,st);
   assert(s.whites>0);
   assert(st.browStrength==0 || e!=Emotion::Neutral);
  }
 }
 // Frontal eyes now have identical size. Pupils are perfectly centred.
 face::State neutral;
 PixelSurface surf;face::draw(surf,neutral);
 assert(surf.pixels[229*466+163]==0); // left black oval pupil
 assert(surf.pixels[229*466+303]==0); // right black oval pupil
 assert(surf.pixels[229*466+205]==0xffff); // left white eye ring
 assert(surf.pixels[229*466+345]==0xffff); // right white eye ring
 assert(surf.pixels[127*466+153]==0); // no brow in neutral
 assert(surf.pixels[137*466+307]==0); // no brow in neutral
 assert(surf.pixels[20*466+20]==0); // no glow on background
 // Brow appears only when expressive.
 face::State angry=face::evaluate(900,Emotion::Angry);
 assert(angry.browStrength>0 && angry.browTilt>0);
 face::State curious=face::evaluate(900,Emotion::Curious);
 assert(curious.browAsymmetry>0);
 character::Controller controller;
 controller.set(Emotion::Angry,1000);
 controller.tick(1500);
 controller.set(Emotion::Neutral,2000);
 controller.tick(2400);
 assert(controller.sample(2400).browStrength==0);
 std::cout<<"Reference style tests passed\n";
}
