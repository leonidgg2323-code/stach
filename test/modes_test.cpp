#include "stach_modes.h"
#include "stach_character.h"
#include <cassert>
#include <iostream>
#include <string>
int main(){
  using namespace stach;
  int count=0;
  auto ck=[&](bool yes){assert(yes);++count;};
  modes::Mode v=modes::Mode::Face;
  for(auto pair: {"face","mouse","wifi","portal"}){
    ck(modes::parse(pair,v));ck(std::string(modes::name(v))==pair);
  }
  for(auto bad:{"", "Mouse", "wifi ", "neutral", "qr", "false", "portal/../../"})ck(!modes::parse(bad,v));
  ck(!modes::parse(nullptr,v));
  ck(modes::fromState(false,pairing::Screen::Face)==modes::Mode::Face);
  ck(modes::fromState(false,pairing::Screen::Wifi)==modes::Mode::Wifi);
  ck(modes::fromState(false,pairing::Screen::Portal)==modes::Mode::Portal);
  ck(modes::fromState(true,pairing::Screen::Portal)==modes::Mode::Mouse);
  const modes::Mode all[]={modes::Mode::Face,modes::Mode::Mouse,modes::Mode::Wifi,modes::Mode::Portal};
  for(auto old:all)for(auto next:all){
    auto p=modes::transition(old,next,true);
    ck(p.accepted);
    ck(p.enableMouse==(next==modes::Mode::Mouse && old!=modes::Mode::Mouse));
    ck(p.disableMouse==(next!=modes::Mode::Mouse && old==modes::Mode::Mouse));
    ck(!(p.enableMouse && p.disableMouse));
    ck(p.screen==(next==modes::Mode::Wifi?pairing::Screen::Wifi:
      next==modes::Mode::Portal?pairing::Screen::Portal:pairing::Screen::Face));
    auto unavailable=modes::transition(old,next,false);
    ck(unavailable.accepted==(next!=modes::Mode::Mouse));
  }
  for(unsigned n=0;n<256;++n){
    ck(modes::validBrightness(n)==(n>=20 && n<=100));
    ck(modes::validGazeActivity(n)==(n>=20 && n<=100));
  }
  modes::Appearance prefs;prefs.brightness=0;prefs.gazeActivity=201;prefs.blink=false;
  prefs.sanitize();ck(prefs.brightness==100 && prefs.gazeActivity==100 && !prefs.blink);
  for(unsigned n=0;n<=150;++n){
    std::string text=std::to_string(n);unsigned v=777;
    ck(modes::parsePercent(text.c_str(),v)&&v==n);
  }
  for(auto bad:{"", "-1", "+20", " 20", "20.0", "1000", "1a", "25%", "45 ", "99999"}){
    unsigned v=77;ck(!modes::parsePercent(bad,v));
  }
  character::Controller character;
  character.blinkEnabled=false;
  auto with=face::evaluate(6920,Emotion::Neutral,true);
  auto without=face::evaluate(6920,Emotion::Neutral,false);
  ck(with.openness<1 && without.openness==1);
  ck(character.sample(6920).openness==1);
  std::cout<<"Stage 10 mode and personalization assertions: "<<count<<" passed\n";
}
