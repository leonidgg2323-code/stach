#pragma once
#include "stach_display.h"
#include "stach_touch.h"
#include <Wire.h>
#include <Preferences.h>
#include <esp_random.h>
#include <math.h>
namespace stach { namespace ui175 {
constexpr int center = 233;
constexpr uint16_t white = 0xffff, mint = 0x77f5, slate = 0x2106;
enum class Screen { Face, Games, Targets, Sums, Dice, Coin };
Screen screen = Screen::Face;
struct Power {
  bool valid = false, battery = false, usb = false, charging = false;
  int percent = -1, millivolts = -1;
} power;
bool gameDirty=true;
constexpr uint32_t splashDuration=400;
bool splash = true;
uint32_t splashAt, lastPower;
bool pmicRead(uint8_t reg, uint8_t &value) {
  Wire.beginTransmission(0x34);
  Wire.write(reg);
  if (Wire.endTransmission(false) || Wire.requestFrom(uint8_t(0x34), uint8_t(1)) != 1) return false;
  value = Wire.read();
  return true;
}

void readPower(uint32_t now) {
  Power before=power;
  uint8_t status, charge, pct, hi, lo;
  power.millivolts=-1;
  if (!pmicRead(0, status) || !pmicRead(1, charge)) {
    power.percent = -1;
    if(before.valid) gameDirty=true;
    power.valid = false;
    return;
  }
  bool usb = status & 0x20;
  if (power.valid && !power.usb && usb) {
    splash = true;
    splashAt = now;
    screen = Screen::Face;
  }
  power.valid = true;
  power.usb = usb;
  power.battery = status & 8;
  power.charging = power.battery && (charge >> 5) == 1;
  if (power.battery && pmicRead(0x34,hi) && pmicRead(0x35,lo)) {
    int mv=((hi & 31)<<8) | lo;
    if(mv>0) power.millivolts=mv;
  }
  power.percent = power.battery && pmicRead(0xa4, pct) && pct <= 100 ? pct : -1;
  if(!before.valid || power.percent!=before.percent || power.battery!=before.battery ||
     power.charging!=before.charging || power.usb!=before.usb) gameDirty=true;
}

void label(const char *s, int x, int y, int size, uint16_t color = white) {
  display::panel()->setTextSize(size);
  display::panel()->setTextColor(color);
  display::panel()->setCursor(x - int(strlen(s)) * 3 * size, y);
  display::panel()->print(s);
}

void batteryBar() {
  display::panel()->drawRoundRect(164, 47, 38, 18, 4, white);
  display::panel()->fillRect(202, 52, 4, 8, white);
  if (power.valid && power.percent > 0)
    display::panel()->fillRect(167, 50, 32 * power.percent / 100, 12, power.charging ? mint : white);
  char value[16];
  if (!power.valid) strcpy(value, "--");
  else if (!power.battery) strcpy(value, power.usb ? "USB" : "NO BAT");
  else if (power.percent < 0) strcpy(value, "--");
  else snprintf(value, sizeof(value), "%s%d%%", power.charging ? "+" : "", power.percent);
  label(value, 251, 49, 2);
}

void drawSplash(uint32_t age) {
  display::panel()->fillScreen(0);
  label("STACH", center, 175, 7);
  label("WAKING UP", center, 260, 2, mint);
  display::panel()->drawRoundRect(145, 306, 176, 10, 4, slate);
  const int width = min(uint32_t(170), age * 170 / splashDuration);
  if (width) display::panel()->fillRect(148, 309, width, 4, mint);
  display::flush();
}

int score, roundNumber, targetX, targetY, a, b, choices[2], correct;
uint32_t gameAt;
bool finished;
uint32_t shownSecond;
enum class Feedback { None, Hit, Miss, Correct, Wrong };
Feedback feedback=Feedback::None;
uint32_t feedbackAt;
int bestTargets=0, bestSums=0;
bool recordChecked=false;

void loadRecords(Preferences& prefs) {
  bestTargets=prefs.getInt("bestTargets",0);
  bestSums=prefs.getInt("bestSums",0);
}

void newRound() {
  if (screen == Screen::Targets) {
    targetX = random(108, 359);
    targetY = random(161, 323);
  } else {
    a = random(2, 16);
    b = random(2, 16);
    correct = random(2);
    choices[correct] = a + b;
    choices[1 - correct] = a + b + (random(2) ? 1 : -1) * random(1, 5);
  }
}

void startGame(Screen game, uint32_t now) {
  screen = game;
  score = 0;
  recordChecked=false;
  feedback=Feedback::None;
  roundNumber = 1;
  finished = false;
  gameAt = now;
  newRound();
}

constexpr uint32_t tossDuration=950;
uint32_t tossAt, lastTossFrame;
bool tossing=false;
int tossResult=0;

void startToss(Screen game,uint32_t now) {
  screen=game;
  if(game==Screen::Coin) tossResult=esp_random() & 1u;
  else {
    // Three uniform bits: discard 6 and 7 so all six faces have equal weight.
    unsigned sample;
    do { sample=esp_random() & 7u; } while(sample>=6);
    tossResult=int(sample)+1;
  }
  tossAt=now;
  lastTossFrame=now;
  tossing=true;
  gameDirty=true;
}

void drawDie(int cx,int cy,float angle,int face) {
  auto* d=display::panel();
  const float co=cosf(angle),si=sinf(angle);
  auto x=[&](int u,int v){return cx+lroundf(co*u-si*v);};
  auto y=[&](int u,int v){return cy+lroundf(si*u+co*v);};
  constexpr int r=53;
  const int ax=x(-r,-r),ay=y(-r,-r),bx=x(r,-r),by=y(r,-r);
  const int dx=x(-r,r),dy=y(-r,r),ex=x(r,r),ey=y(r,r);
  d->fillTriangle(ax,ay,bx,by,ex,ey,white);
  d->fillTriangle(ax,ay,ex,ey,dx,dy,white);
  auto pip=[&](int u,int v){d->fillCircle(x(u,v),y(u,v),8,slate);};
  if(face%2) pip(0,0);
  if(face>=2){pip(-29,-29);pip(29,29);}
  if(face>=4){pip(29,-29);pip(-29,29);}
  if(face==6){pip(-29,0);pip(29,0);}
}

void drawToss(uint32_t now) {
  const bool coin=screen==Screen::Coin;
  float t=tossing?float(now-tossAt)/tossDuration:1.f;
  if(t>1.f)t=1.f;
  const float ease=1.f-(1.f-t)*(1.f-t);
  const int cy=239-int(48*4*t*(1.f-t));
  label(coin?"COIN":"DICE",center,97,4,mint);
  if(coin) {
    const float side=cosf(ease*(6+tossResult)*PI);
    const int radius=max(3,int(54*fabsf(side)));
    auto* d=display::panel();
    d->fillEllipse(center+3,cy+3,radius,54,0xa4a4);
    d->fillEllipse(center,cy,radius,54,0xfe6b);
    if(radius>8)d->drawEllipse(center,cy,radius-5,48,0xa4a4);
    if(radius>28)label(side>=0?"H":"T",center,cy-17,5,slate);
  } else {
    const int step=int(ease*12);
    const int face=tossing && step<12 ? 1+(tossResult-1+step)%6 : tossResult;
    drawDie(center+int(25*sinf(t*2*PI)*(1-t)),cy,2*PI*ease,face);
  }
  if(!tossing) {
    char result[16];
    if(coin) strcpy(result,tossResult?"TAILS":"HEADS");
    else snprintf(result,sizeof(result),"Rolled %d",tossResult);
    label(result,center,310,3,mint);
    label(coin?"Tap to flip again":"Tap to roll again",center,345,2);
  }
}

bool gameNeedsRedraw(uint32_t now,Preferences& prefs) {
  if(screen==Screen::Dice || screen==Screen::Coin) {
    if(tossing && (now-lastTossFrame>=32 || now-tossAt>=tossDuration)) {
      lastTossFrame=now;
      tossing=now-tossAt<tossDuration;
      gameDirty=true;
    }
    return gameDirty;
  }
  if(screen==Screen::Targets && !finished) {
    uint32_t elapsed=now-gameAt;
    finished=elapsed>=20000;
    if(finished || elapsed/1000!=shownSecond) gameDirty=true;
    shownSecond=elapsed/1000;
  }
  if(feedback!=Feedback::None && now-feedbackAt>=(screen==Screen::Sums?700u:240u)) {
    feedback=Feedback::None;
    gameDirty=true;
    if(screen==Screen::Sums && !finished) {
      if(roundNumber==5) finished=true;
      else { ++roundNumber; newRound(); }
    }
  }
  if(finished && !recordChecked && (screen==Screen::Targets || screen==Screen::Sums)) {
    int& best=screen==Screen::Targets?bestTargets:bestSums;
    if(score>best && prefs.putInt(screen==Screen::Targets?"bestTargets":"bestSums",score)==sizeof(int))
      best=score;
    recordChecked=true;
  }
  return gameDirty;
}

void button(int x, int y, int w, const char *title) {
  display::panel()->fillRoundRect(x, y, w, 64, 16, slate);
  label(title, x + w / 2, y + 21, 3);
}

void drawGames(uint32_t now) {
  gameDirty=false;
  display::panel()->fillScreen(0);
  char line[32];
  if (screen == Screen::Games) {
    label("GAMES", center, 100, 5, mint);
    button(70,169,154,"Targets");
    button(242,169,154,"Sums");
    button(70,258,154,"Dice");
    button(242,258,154,"Coin");
  } else if(screen==Screen::Dice || screen==Screen::Coin) {
    drawToss(now);
  } else if (finished) {
    label("ROUND OVER", center, 152, 4, mint);
    snprintf(line, sizeof(line), "Score %d", score);
    label(line, center, 218, 4);
    snprintf(line,sizeof(line),"Best %d",screen==Screen::Targets?bestTargets:bestSums);
    label(line,center,258,2,mint);
    button(126, 284, 214, "Play again");
  } else if (screen == Screen::Targets) {
    snprintf(line, sizeof(line), "%lus   Hits %d", (unsigned long)((20000 - (now - gameAt) + 999) / 1000), score);
    label(line, center, 110, 3);
    display::panel()->fillCircle(targetX, targetY, 32, mint);
    display::panel()->fillCircle(targetX, targetY, 18, 0);
    display::panel()->fillCircle(targetX, targetY, 8, mint);
    if(feedback==Feedback::Hit) label("+1",center,350,3,mint);
    else if(feedback==Feedback::Miss) label("Miss",center,350,2,0xfd69);
  } else {
    snprintf(line, sizeof(line), "%d / 5", roundNumber);
    label(line, center, 110, 3, mint);
    if(feedback==Feedback::None) snprintf(line,sizeof(line),"%d + %d",a,b);
    else snprintf(line,sizeof(line),"%d + %d = %d",a,b,a+b);
    label(line,center,171,feedback==Feedback::None?5:3);
    if(feedback==Feedback::Correct) label("Correct! +1",center,218,2,mint);
    else if(feedback==Feedback::Wrong) label("Correct answer above",center,218,2,0xfd69);
    for (int i = 0; i < 2; ++i) {
      snprintf(line, sizeof(line), "%d", choices[i]);
      button(90 + 150 * i, 261, 135, line);
    }
  }
  label("< Swipe left", center, 382, 2, mint);
}

void tapGame(int x, int y, uint32_t now) {
  if (screen == Screen::Games) {
    bool left=x>=70 && x<224, right=x>=242 && x<396;
    if(!left && !right)return;
    if(y>=169 && y<233) startGame(left?Screen::Targets:Screen::Sums,now);
    if(y>=258 && y<322) startToss(left?Screen::Dice:Screen::Coin,now);
  } else if(screen==Screen::Dice || screen==Screen::Coin) {
    if(!tossing) startToss(screen,now);
  } else if (finished) {
    if (x >= 126 && x < 340 && y >= 284 && y < 348) startGame(screen, now);
  } else if (screen == Screen::Targets) {
    int dx = x - targetX, dy = y - targetY;
    feedbackAt=now;
    if (dx*dx + dy*dy <= 32*32) { feedback=Feedback::Hit; ++score; newRound(); }
    else feedback=Feedback::Miss;
  } else if (screen == Screen::Sums && y >= 261 && y < 325) {
    int pick = x >= 90 && x < 225 ? 0 : x >= 240 && x < 375 ? 1 : -1;
    if (pick < 0 || feedback!=Feedback::None) return;
    feedbackAt=now;
    feedback=pick==correct?Feedback::Correct:Feedback::Wrong;
    if (pick == correct) ++score;

  }
}


bool gameGesture(const touch::Event& event, uint32_t now) {
  using touch::Gesture;
  if(screen==Screen::Face) {
    if(event.kind!=Gesture::SwipeRight) return false;
    screen=Screen::Games;
  } else if(event.kind==Gesture::SwipeLeft) {
    feedback=Feedback::None;
    screen=screen==Screen::Games ? Screen::Face : Screen::Games;
  } else if(event.kind==Gesture::Tap || event.kind==Gesture::DoubleTap) {
    if(screen==Screen::Targets && !finished && now-gameAt>=20000) {
      finished=true;
    } else {
      tapGame(event.x,event.y,now);
    }
  }
  gameDirty=true;
  return true;
}
}}
