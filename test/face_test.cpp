#include "stach_face.h"
#include <cassert>
#include <vector>
#include <cmath>
struct FakeSurface {
  int clears=0,lines=0,white=0,black=0;
  void clear(uint16_t c){assert(c==0);++clears;}
  void hline(int x,int y,int w,uint16_t c){assert(x>=0&&y>=0&&x+w<=466&&y<466&&w>0);++lines;if(c==0xffff)++white;else ++black;}
};
int main(){
 using namespace stach;
 for(auto e:{Emotion::Neutral,Emotion::Sleepy,Emotion::Angry,Emotion::Curious,Emotion::Happy}){
  for(uint32_t ms=0;ms<12000;ms+=37){
   auto state=face::evaluate(ms,e);
   assert(std::isfinite(state.gazeX));assert(std::isfinite(state.gazeY));
   assert(state.openness>=0&&state.openness<=1);
   FakeSurface f;face::draw(f,state);
   assert(f.clears==1&&f.white>0&&f.lines>0);
  }
 }
 assert(face::evaluate(6920,Emotion::Neutral).openness<0.2f);
 assert(face::evaluate(150,Emotion::Sleepy).openness<0.5f);
}
