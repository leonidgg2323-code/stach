#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <esp_system.h>
#include "stach_core.h"
#include "stach_face.h"
#include "stach_character.h"
#include "stach_behavior.h"
#include "stach_touch.h"
#include "stach_motion.h"
#include "stach_pairing.h"
#include "stach_control.h"
#include "stach_web.h"
#include <esp_timer.h>
#ifdef STACH_WAVESHARE_BOARD
#include "stach_hardware.h"
#include "stach_display.h"
#include "stach_imu.h"
#include "stach_touch_hal.h"
#include "stach_qr_display.h"
#endif

static WebServer server(80);
static Preferences preferences;
static String ssid;
static String password;
static stach::character::Controller character;
static stach::behavior::Engine behavior;
static uint64_t manualUntil=0;
static char qrPayload[128] = {};
static char portalUrl[64] = {};
static stach::pairing::Controller pairing;
static stach::control::Settings settings;
static stach::control::RemoteGaze remoteGaze;
static bool initialized = false;
static stach::motion::Tilt tilt;
static stach::motion::Gaze gestureGaze;
static stach::motion::Sample lastMotion;
static stach::touch::Gesture lastGesture=stach::touch::Gesture::None;
static uint32_t lastGestureAt=0;
#ifdef STACH_WAVESHARE_BOARD
static bool faceDisplayReady = false;
static stach::pairing::Screen drawnScreen = stach::pairing::Screen::Portal;
static stach::imu::Driver imu;
static bool imuReady=false;
static stach::touch::Driver touchDriver;
static stach::touch::Recognizer touchRecognizer;
static bool touchReady=false;
static uint32_t lastTouchValidAt=0;
struct AmoledSurface {
  void clear(uint16_t color) { stach::display::panel()->fillScreen(color); }
  void hline(int x,int y,int w,uint16_t color) {
    if(y<0 || y>=stach::face::HEIGHT || w<=0)return;
    if(x<0){w+=x;x=0;}
    if(x+w>stach::face::WIDTH)w=stach::face::WIDTH-x;
    if(w>0)stach::display::panel()->drawFastHLine(x,y,w,color);
  }
};
#endif

// The mobile-first page is embedded from web/stach-panel.html (stach_web.h).

static String newPassword() {
  const char* hex = "0123456789ABCDEF";
  char buffer[17];
  uint8_t bytes[8];
  esp_fill_random(bytes, sizeof(bytes));
  for (int i=0;i<8;i++) {buffer[i*2]=hex[bytes[i]>>4];buffer[i*2+1]=hex[bytes[i]&15];}
  buffer[16]='\0';
  return String(buffer);
}

void setup() {
  Serial.begin(115200);
  behavior.begin(uint64_t(esp_timer_get_time())/1000ULL);
#ifdef STACH_WAVESHARE_BOARD
  // Non-destructive hardware diagnostics: does not toggle AMOLED power/reset.
  stach::hardware::logProbe(stach::hardware::probe());
  const bool displayReady = stach::display::begin();
  Serial.printf("[HW] AMOLED init: %s\n", displayReady ? "OK" : "FAILED");
  faceDisplayReady = displayReady;
  imuReady=imu.begin();
  Serial.printf("[IMU] Accelerometer+gyro: %s\n",imuReady?"OK":"not available");
  touchReady=touchDriver.begin();
  Serial.printf("[TOUCH] FT3168/FT6146 0x38: %s\n",touchReady?"OK":"not available");
#endif
  if (!preferences.begin("stach", false)) {Serial.println("NVS unavailable"); return;}
  settings.idleMotion = preferences.getBool("idle", true);
  settings.tiltEnabled = preferences.getBool("tilt", true);
  password = preferences.getString("ap_pass", "");
  if (!stach::validPassword(password.c_str())) {
    password = newPassword();
    if (preferences.putString("ap_pass", password) != password.length()) {
      Serial.println("Failed to persist AP password"); return;
    }
  }
  uint64_t mac = ESP.getEfuseMac();
  char suffix[7];
  snprintf(suffix, sizeof(suffix), "%06lX", static_cast<unsigned long>(mac & 0xFFFFFF));
  ssid = "Stach-" + String(suffix);
  if (!stach::wifiQr(qrPayload, sizeof(qrPayload), ssid.c_str(), password.c_str())) {
    Serial.println("Invalid QR payload"); return;
  }
  WiFi.mode(WIFI_AP);
  // Fixed link-local AP address makes the portal QR reliable.
  if (!WiFi.softAPConfig(IPAddress(192,168,4,1),IPAddress(192,168,4,1),IPAddress(255,255,255,0))) {
    Serial.println("Wi-Fi AP address configuration failed"); return;
  }
  if (!WiFi.softAP(ssid.c_str(), password.c_str())) {Serial.println("Wi-Fi AP failed"); return;}
  const String address = String("http://") + WiFi.softAPIP().toString() + "/";
  if (address.length() >= sizeof(portalUrl)) {Serial.println("Portal URL too long"); return;}
  address.toCharArray(portalUrl,sizeof(portalUrl));
  server.on("/", HTTP_GET, [](){server.sendHeader("Cache-Control","no-store");server.send_P(200,"text/html; charset=utf-8",stach::web::page);});
  server.on("/api/status", HTTP_GET, [](){
    const uint32_t now=millis();
    const uint64_t uptime=uint64_t(esp_timer_get_time())/1000ULL;
    const uint32_t remain=manualUntil>uptime?uint32_t(manualUntil-uptime):0;
    String body = String("{\"name\":\"Stach\",\"expression\":\"") + stach::emotionName(character.current) +
      "\",\"ip\":\"" + WiFi.softAPIP().toString() + "\",\"ssid\":\"" + ssid +
      "\",\"pairingScreen\":\"" + stach::pairing::name(pairing.screen) +
      "\",\"chargingKnown\":" + (behavior.chargingKnown?"true":"false") +
      ",\"charging\":" + (behavior.charging?"true":"false") +
      ",\"batteryKnown\":" + (behavior.batteryKnown?"true":"false") +
      ",\"batteryPercent\":" + (behavior.batteryKnown?String(behavior.batteryPercent):String("null")) +
      ",\"touchReady\":" +
#ifdef STACH_WAVESHARE_BOARD
      (touchReady?"true":"false") +
#else
      String("false") +
#endif
      ",\"imuReady\":" +
#ifdef STACH_WAVESHARE_BOARD
      (imuReady?"true":"false") +
#else
      String("false") +
#endif
      ",\"lastGesture\":\"" + stach::touch::name(lastGesture) +
      "\",\"lastGestureMs\":" + String(lastGestureAt) +
      ",\"tiltX\":" + String(tilt.x,3) +
      ",\"tiltY\":" + String(tilt.y,3) +
      ",\"gyroX\":" + String(lastMotion.gx,2) +
      ",\"gyroY\":" + String(lastMotion.gy,2) +
      ",\"gyroZ\":" + String(lastMotion.gz,2) +
      ",\"idleMotion\":" + (settings.idleMotion?"true":"false") +
      ",\"tiltEnabled\":" + (settings.tiltEnabled?"true":"false") +
      ",\"manualRemainingMs\":" + String(remain) +
      ",\"gazeActive\":" + (remoteGaze.active(now)?"true":"false") +
      ",\"clients\":" + String(WiFi.softAPgetStationNum()) +
      ",\"uptimeMs\":" + String(now) + "}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(200,"application/json; charset=utf-8",body);
  });
  server.on("/api/expression", HTTP_POST, [](){
    stach::Emotion next = stach::parseEmotion(server.arg("value").c_str());
    if (next == stach::Emotion::Invalid) {server.send(400,"text/plain","Invalid expression");return;}
    character.set(next,millis());
    manualUntil=uint64_t(esp_timer_get_time())/1000ULL+8000; // 8-second manual preview override
    server.send(200,"text/plain",String("Expression: ")+stach::emotionName(character.current));
  });
  // AP clients can ask the physical Stach to display a QR code.
  // Never return the persistent Wi-Fi password over /api/status.
  server.on("/api/pairing", HTTP_POST, [](){
    stach::pairing::Screen next;
    if(!stach::pairing::parseScreen(server.arg("mode").c_str(),next)){
      server.send(400,"text/plain","Invalid pairing screen");return;
    }
    pairing.show(next,millis());
    server.send(200,"text/plain",String("Stach screen: ")+stach::pairing::name(pairing.screen));
  });
  // A short-lived, bounded remote gaze. Values are normalized [-1, 1].
  server.on("/api/gaze", HTTP_POST, [](){
    float x=0, y=0;
    if(!stach::control::parseAxis(server.arg("x").c_str(), x) ||
       !stach::control::parseAxis(server.arg("y").c_str(), y) ||
       !remoteGaze.set(x,y,millis())) {
      server.send(400,"application/json","{\"message\":\"Invalid gaze coordinates\"}");return;
    }
    server.sendHeader("Cache-Control","no-store");
    server.send(200,"application/json","{\"ok\":true}");
  });
  server.on("/api/settings", HTTP_GET, [](){
    const String body=String("{\"idleMotion\":") + (settings.idleMotion?"true":"false") +
      ",\"tiltEnabled\":" + (settings.tiltEnabled?"true":"false") + "}";
    server.sendHeader("Cache-Control","no-store");
    server.send(200,"application/json",body);
  });
  server.on("/api/settings", HTTP_POST, [](){
    const String key=server.arg("key");
    bool next=false;
    if((key!="idleMotion" && key!="tiltEnabled") ||
       !stach::control::parseSwitch(server.arg("value").c_str(),next)){
      server.send(400,"application/json","{\"message\":\"Unknown setting or invalid value\"}");return;
    }
    bool* target=(key=="idleMotion")? &settings.idleMotion : &settings.tiltEnabled;
    const char* storeKey=(key=="idleMotion")?"idle":"tilt";
    if(*target!=next && preferences.putBool(storeKey,next)!=1){
      server.send(500,"application/json","{\"message\":\"Settings could not be saved\"}");return;
    }
    *target=next;
    server.sendHeader("Cache-Control","no-store");
    server.send(200,"application/json","{\"ok\":true}");
  });
  server.onNotFound([](){server.send(404,"text/plain","Not found");});
  server.begin();
  initialized = true;
  Serial.println("Stach AP: " + ssid);
  // Intentionally avoid leaking the persistent password in serial logs.
  Serial.println("Open " + String(portalUrl) + " after joining Stach Wi-Fi");
  Serial.println("Long-press Stach's display to show the Wi-Fi QR, tap for portal QR.");
}
void loop() {
  if (initialized) server.handleClient();
  const uint32_t tickMs=millis();
  pairing.tick(tickMs);
  const uint64_t uptimeMs=uint64_t(esp_timer_get_time())/1000ULL;
#ifdef STACH_WAVESHARE_BOARD
  static uint64_t lastImuMs=0;
  if(imuReady && uptimeMs-lastImuMs>=20){
    lastImuMs=uptimeMs;
    stach::motion::Sample current;
    if(imu.read(current)){
      lastMotion=current;
      const float g=sqrtf(current.ax*current.ax+current.ay*current.ay+current.az*current.az);
      behavior.acceleration(g,uptimeMs);
      tilt.update(current);
    }
  }
  static uint32_t lastTouchPoll=0;
  if(touchReady && uint32_t(tickMs-lastTouchPoll)>=20){
    lastTouchPoll=tickMs;
    stach::touch::Point point;
    if(touchDriver.read(point)){
      lastTouchValidAt=tickMs;
      const stach::touch::Event event=touchRecognizer.feed(point,tickMs);
      if(event.kind!=stach::touch::Gesture::None){
        lastGesture=event.kind;lastGestureAt=tickMs;
        const bool changedPairing = pairing.gesture(event.kind,tickMs);
        if(!changedPairing && pairing.screen==stach::pairing::Screen::Face) {
          gestureGaze.event(event.kind,tickMs);
          behavior.touch(uptimeMs); // only on face, not on QR screen
        }
        Serial.printf("[TOUCH] %s (%d,%d)\n",stach::touch::name(event.kind),event.x,event.y);
      }
    }else if(uint32_t(tickMs-lastTouchValidAt)>250){
      touchRecognizer.cancel(); // Do not synthesize fake taps on I2C failures.
    }
  }
#endif
  if(uptimeMs>=manualUntil){
    character.set(behavior.choose(uptimeMs),tickMs,behavior.showCharging());
  }
  character.tick(tickMs);
#ifdef STACH_WAVESHARE_BOARD
  static uint32_t lastFrame=0;
  const uint32_t now=millis();
  if(faceDisplayReady) {
    if(pairing.screen != stach::pairing::Screen::Face) {
      if(pairing.screen != drawnScreen) {
        const char* contents = pairing.screen==stach::pairing::Screen::Wifi ? qrPayload : portalUrl;
        if(!stach::pairing::renderCode(contents,pairing.screen)) stach::pairing::renderFailure();
        drawnScreen=pairing.screen;
      }
    } else if(drawnScreen != stach::pairing::Screen::Face || (uint32_t)(now-lastFrame)>=100) {
      lastFrame=now;
      drawnScreen=stach::pairing::Screen::Face;
      AmoledSurface surface;
      stach::face::State frame=character.sample(now);
      if (!settings.idleMotion) { frame.gazeX=0; frame.gazeY=0; frame.offsetX=0; frame.offsetY=0; }
      const stach::motion::Tilt neutralTilt;
      gestureGaze.apply(frame,now,settings.tiltEnabled?tilt:neutralTilt);
      remoteGaze.apply(frame,now);
      stach::face::draw(surface,frame);
    }
  }
#endif
  delay(2);
}
