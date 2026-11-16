#include <cstring>
#include <iostream>
#include "stach_core.h"
static int checks=0;
#define CHECK(x) do { ++checks; if (!(x)) { std::cerr << "FAILED " << __FILE__ << ':' << __LINE__ << ": " #x << '\n'; return 1; } } while(0)
int main() {
 using namespace stach;
 CHECK(parseEmotion("neutral") == Emotion::Neutral);
 CHECK(parseEmotion("angry") == Emotion::Angry);
 CHECK(parseEmotion("sleepy") == Emotion::Sleepy);
 CHECK(parseEmotion("curious") == Emotion::Curious);
 CHECK(parseEmotion("ANGRY") == Emotion::Invalid);
 CHECK(parseEmotion("surprised") == Emotion::Invalid);
 CHECK(parseEmotion(nullptr) == Emotion::Invalid);
 CHECK(parseEmotion("") == Emotion::Invalid);
 CHECK(std::strcmp(emotionName(Emotion::Curious),"curious") == 0);
 CHECK(std::strcmp(emotionName(Emotion::Invalid),"invalid") == 0);
 CHECK(!validPassword("short"));
 CHECK(!validPassword(nullptr));
 CHECK(validPassword("123456789012"));
 CHECK(validPassword("123456789012345678901234567890123456789012345678901234567890123"));
 CHECK(!validPassword("bad password here"));
 CHECK(!validPassword("12345678901\n"));
 char qr[128] = {};
 CHECK(wifiQr(qr,sizeof(qr),"Stach-A1B2C3","123456789012"));
 CHECK(std::strcmp(qr,"WIFI:T:WPA;S:Stach-A1B2C3;P:123456789012;;") == 0);
 char tiny[4];
 CHECK(!wifiQr(tiny,sizeof(tiny),"Stach-A1B2C3","123456789012"));
 CHECK(!wifiQr(qr,sizeof(qr),"Bad-ABCDEF","123456789012"));
 CHECK(!wifiQr(qr,sizeof(qr),"Stach-a1b2c3","123456789012"));
 CHECK(!wifiQr(qr,sizeof(qr),"Stach-A1B2C3","short"));
 CHECK(!wifiQr(nullptr,128,"Stach-A1B2C3","123456789012"));
 CHECK(!wifiQr(qr,sizeof(qr),nullptr,"123456789012"));
 CHECK(!wifiQr(qr,sizeof(qr),"Stach-A1B2C3",nullptr));
 CHECK(!wifiQr(qr,0,"Stach-A1B2C3","123456789012"));
 std::cout << checks << " core checks passed\n";
}
