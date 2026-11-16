#include "stach_character.h"
#include <cassert>
#include <cmath>
#include <cstring>
#include <initializer_list>
int main(){
  using namespace stach;
  character::Controller c;
  assert(!c.set(Emotion::Invalid,100));
  assert(c.current==Emotion::Neutral);
  for(const char* n:{"neutral","angry","sleepy","curious","happy"}){
    Emotion e=parseEmotion(n);assert(e!=Emotion::Invalid);
    assert(std::strcmp(emotionName(e),n)==0);
    assert(c.set(e,1000));
    for(uint32_t t=1000;t<=1500;t+=10){
      auto s=c.sample(t);
      assert(std::isfinite(s.gazeX)&&std::isfinite(s.gazeY));
      assert(s.openness>=0 && s.openness<=1);
    }
    c.tick(1500);assert(!c.transitioning);
  }
  c.set(Emotion::Angry,2000);
  auto before=c.sample(2100);
  c.set(Emotion::Sleepy,2100);
  auto after=c.sample(2100);
  assert(std::abs(before.openness-after.openness)<0.0001f);
  assert(std::abs(before.gazeX-after.gazeX)<0.0001f);
  c.tick(2600);assert(!c.transitioning);
  for(uint32_t t=0;t<100000;t+=137){
    auto s=c.sample(t);assert(std::isfinite(s.openness));
  }
}
