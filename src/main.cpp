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
#include "stach_modes.h"
#include "stach_security.h"
#include "stach_mouse.h"
#include "stach_power.h"
#include "stach_web.h"
#include <esp_timer.h>
#ifdef STACH_WAVESHARE_BOARD
#include "stach_hardware.h"
#include "stach_display.h"
#include "stach_imu.h"
#include "stach_touch_hal.h"
#include "stach_qr_display.h"
#include "stach_ble_mouse.h"
#include "stach_mouse_ui.h"
#include "stach_power_hal.h"
#include "stach_power_ui.h"
#include "stach_hw_report.h"
#ifdef STACH_EYES_175
#include "stach_ui_175.h"
#include "stach_owner.h"
#include "stach_owner_page.h"
namespace owner=stach::owner;
namespace ui175 = stach::ui175;
static uint32_t fingerAt;
static bool hasFinger;
static float fingerX, fingerY;
#endif
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
static stach::modes::Appearance appearance;
static stach::mouse::Engine airMouse;
static stach::control::RemoteGaze remoteGaze;
static bool initialized = false;
static char sessionToken[33] = {}; // random, RAM-only; never printed/stored
static stach::security::RateGate requestRate;
static bool apHealthy=false;
static uint32_t lastApRecovery=0;

static void securityHeaders(){
  server.sendHeader("Cache-Control", "no-store, max-age=0");
  server.sendHeader("X-Content-Type-Options", "nosniff");
  server.sendHeader("Referrer-Policy", "no-referrer");
  server.sendHeader("X-Frame-Options", "DENY");
  server.sendHeader("Content-Security-Policy", "default-src 'none'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; img-src 'self' data:; connect-src 'self'; base-uri 'none'; form-action 'none'; frame-ancestors 'none'");
}
static uint32_t remoteAddress(){
  IPAddress ip=server.client().remoteIP();
  return uint32_t(ip[0]) | uint32_t(ip[1])<<8 | uint32_t(ip[2])<<16 | uint32_t(ip[3])<<24;
}
static bool guardEnvelope() {
  securityHeaders();
  if (!stach::security::allowedHost(server.hostHeader().c_str()) ||
      !stach::security::allowedOrigin(server.header("Origin").c_str()) ||
      !stach::security::allowedFetchSite(server.header("Sec-Fetch-Site").c_str())) {
    server.send(403,"application/json","{\"error\":\"Invalid request origin\"}");return false;
  }
  if(server.clientContentLength()>512 || server.args()>8 || server.uri().length()>120){
    server.send(413,"application/json","{\"error\":\"Request too large\"}");return false;
  }
  if(!requestRate.allow(remoteAddress(),millis())){
    server.sendHeader("Retry-After","1");
    server.send(429,"application/json","{\"error\":\"Too many requests\"}");return false;
  }
  return true;
}
static bool guardRequest(bool modifying) {
  if(!guardEnvelope())return false;
#ifdef STACH_EYES_175
  if(!owner::authorized(server)) {
    if(server.method()==HTTP_GET && server.uri()=="/") {
      String page=FPSTR(owner::loginPage);
      page.replace("__SIGNUP__",owner::claimed?"false":"true");
      server.send(200,"text/html; charset=utf-8",page);
    } else server.send(401,"application/json","{\"error\":\"Owner login required\"}");
    return false;
  }
#endif
  if(modifying && !stach::security::equalToken(sessionToken,server.header("X-Stach-Token").c_str())){
    server.send(403,"application/json","{\"error\":\"Session token missing or expired; reload the page\"}");return false;
  }
  return true;
}
static bool startAccessPoint(){
  apHealthy=false;
  if(!WiFi.mode(WIFI_AP))return false;
  const IPAddress local(192,168,4,1);
  if(!WiFi.softAPConfig(local,local,IPAddress(255,255,255,0)))return false;
  apHealthy=WiFi.softAP(ssid.c_str(),password.c_str());
  return apHealthy;
}

static stach::power::Meter batteryMeter;
static stach::power::Saver saver;
static stach::motion::Tilt tilt;
static stach::motion::Gaze gestureGaze;
static stach::motion::Sample lastMotion;
static stach::touch::Gesture lastGesture=stach::touch::Gesture::None;
static uint32_t lastGestureAt=0;
#ifdef STACH_WAVESHARE_BOARD
static bool faceDisplayReady = false;
static stach::hwreport::Report bootHardwareReport;
static uint32_t hardwareTestUntilMs=0;
static bool hardwareTestVisible=false;
static bool wasScreenDark=false;
static stach::ble::Adapter hidMouse;
static bool mouseScreenShown=false;
static uint32_t lastMouseScreen=0;
static bool pairingNeedsRedraw=true;
static stach::pairing::Screen drawnScreen = stach::pairing::Screen::Portal;
static stach::imu::Driver imu;
static bool imuReady=false;
static stach::touch::Driver touchDriver;
static stach::touch::Recognizer touchRecognizer;
static bool touchReady=false;
static uint32_t lastTouchValidAt=0;
struct AmoledSurface {
  uint8_t brightness=100;
  uint16_t shaded(uint16_t c) const {
    if(brightness==100)return c;
    const uint32_t r=(c>>11)&31u, g=(c>>5)&63u, b=c&31u;
    return uint16_t((((r*brightness+50u)/100u)<<11) |
      (((g*brightness+50u)/100u)<<5) | ((b*brightness+50u)/100u));
  }
  void clear(uint16_t color) { stach::display::panel()->fillScreen(shaded(color)); }
  void hline(int x,int y,int w,uint16_t color) {
    if(y<0 || y>=stach::face::HEIGHT || w<=0)return;
    if(x<0){w+=x;x=0;}
    if(x+w>stach::face::WIDTH)w=stach::face::WIDTH-x;
    if(w>0)stach::display::panel()->drawFastHLine(x,y,w,shaded(color));
  }
};
#endif

// Mouse mode is opt-in and never survives reboot. No HID reports are sent
// outside this mode, even if BLE remains bonded/connected after exiting.
static bool setMouseMode(bool on, uint32_t now) {
#ifdef STACH_WAVESHARE_BOARD
  if(on) {
    pairing.show(stach::pairing::Screen::Face,now);
    hidMouse.beginOnce(); // start BLE advertising only after explicit activation
  }
  airMouse.enable(on,now);
  return true;
#else
  (void)on; (void)now;
  return false; // network prototype has no BLE transport
#endif
}

// One central transition point prevents Face/Mouse/QR overlays from overlapping.
static stach::modes::Mode currentMode() {
  return stach::modes::fromState(airMouse.active(),pairing.screen);
}
static bool switchMode(stach::modes::Mode next,uint32_t now) {
#ifdef STACH_WAVESHARE_BOARD
  const bool supportsMouse=true;
#else
  const bool supportsMouse=false;
#endif
  const stach::modes::Plan plan=stach::modes::transition(currentMode(),next,supportsMouse);
  if(!plan.accepted)return false;
  if(plan.disableMouse && !setMouseMode(false,now))return false;
  if(plan.enableMouse && !setMouseMode(true,now))return false;
#ifdef STACH_EYES_175
  ui175::screen=ui175::Screen::Face;
#endif
  pairing.show(plan.screen,now);
  if(next!=stach::modes::Mode::Mouse)remoteGaze.clear();
  saver.activity(now);
  return true;
}

#ifdef STACH_WAVESHARE_BOARD
static void mouseGesture(stach::touch::Gesture gesture,uint32_t now) {
  using stach::mouse::Action;
  switch(stach::mouse::gestureAction(gesture)) {
    case Action::Exit: switchMode(stach::modes::Mode::Face,now); break;
    case Action::LeftClick: hidMouse.click(airMouse,stach::mouse::Button::Left);break;
    case Action::RightClick: hidMouse.click(airMouse,stach::mouse::Button::Right);break;
    case Action::ScrollUp: hidMouse.scroll(airMouse,3,0);break;
    case Action::ScrollDown: hidMouse.scroll(airMouse,-3,0);break;
    case Action::ScrollLeft: hidMouse.scroll(airMouse,0,-3);break;
    case Action::ScrollRight: hidMouse.scroll(airMouse,0,3);break;
    default:break;
  }
}
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
  saver.activity(millis());
#ifdef STACH_WAVESHARE_BOARD
  // Waveshare factory initializes the AMOLED power/driver before the touch
  // and I2C peripheral setup. The previous STACH firmware probed I2C first,
  // which could report false missing peripherals while their power was gated.
  // This correction does NOT prove that the panel accepted graphics commands.
  const bool displayReady = stach::display::begin();
  faceDisplayReady = displayReady;
#ifdef STACH_EYES_175
  ui175::splashAt=millis();
  if(displayReady) ui175::drawSplash(0);
#endif
  Serial.printf("[HW] AMOLED init: %s (software only)\n", displayReady ? "OK" : "FAILED");
#ifndef STACH_EYES_175
  delay(120); // 1.43 power-enable rail settling time
#endif
  const stach::hardware::Probe bootProbe=stach::hardware::probe();
  stach::hardware::logProbe(bootProbe);
#ifndef STACH_EYES_175
  stach::hardware::scanI2C();
#endif
  imuReady=imu.begin();
  Serial.printf("[IMU] Accelerometer+gyro: %s\n",imuReady?"OK":"not available");
  stach::power::beginHardware();
  touchReady=touchDriver.begin();
  Serial.printf("[TOUCH] driver: %s\n",touchReady?"OK":"not available");
  stach::hwreport::Evidence evidence;
  evidence.flashBytes=ESP.getFlashChipSize();
  evidence.psramBytes=ESP.getPsramSize();
  evidence.i2cBus=bootProbe.i2c;
  evidence.touchAck=bootProbe.touch;
  evidence.imuAck=bootProbe.imu;
  evidence.rtcAck=bootProbe.rtc;
  evidence.displayBegin=faceDisplayReady;
  evidence.touchBegin=touchReady;
  evidence.imuBegin=imuReady;
  bootHardwareReport=stach::hwreport::evaluate(evidence);
  Serial.printf("[HWSELFTEST] flash=%lu psram=%lu i2c=%u display=%u touch=%u imu=%u rtc=%u warnings=0x%03X basicDrivers=%s\n",
    (unsigned long)evidence.flashBytes,(unsigned long)evidence.psramBytes,
    (unsigned)evidence.i2cBus,(unsigned)evidence.displayBegin,
    (unsigned)evidence.touchBegin,(unsigned)evidence.imuBegin,
    (unsigned)evidence.rtcAck,(unsigned)bootHardwareReport.warnings,
    bootHardwareReport.basicDriversReady?"OK":"CHECK");
  Serial.println("[HWSELFTEST] Send D via USB serial for a 5-second display pattern; I repeats the summary.");
#endif
  if (!preferences.begin("stach", false)) {Serial.println("NVS unavailable"); return;}
  batteryMeter.configure(preferences.getFloat("batRatio",0.0f));
  saver.enabled = preferences.getBool("psaver",true);
  settings.idleMotion = preferences.getBool("idle",
#ifdef STACH_EYES_175
    false
#else
    true
#endif
  );
  settings.tiltEnabled = preferences.getBool("tilt", true);
  appearance.brightness=preferences.getUChar("eyeBright",100);
  appearance.gazeActivity=preferences.getUChar("eyeMotion",100);
  appearance.blink=preferences.getBool("blink",true);
  appearance.sanitize();
  character.blinkEnabled=appearance.blink;
  stach::mouse::Config mouseConf;
  mouseConf.sensitivity=preferences.getFloat("msens",1.25f);
  if(!stach::mouse::validSensitivity(mouseConf.sensitivity)) mouseConf.sensitivity=1.25f;
  mouseConf.invertX=preferences.getBool("minvx",false);
  mouseConf.invertY=preferences.getBool("minvy",false);
  mouseConf.axisYX=preferences.getBool("maxis",false);
  airMouse.configure(mouseConf,millis());
  uint8_t sessionBytes[16];
  esp_fill_random(sessionBytes,sizeof(sessionBytes));
  stach::security::encodeToken(sessionBytes,sessionToken);
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
  if(!startAccessPoint()) Serial.println("[WIFI] AP unavailable, will retry without reboot");
  const String address = String("http://") + WiFi.softAPIP().toString() + "/";
  if (address.length() >= sizeof(portalUrl)) {Serial.println("Portal URL too long"); return;}
  address.toCharArray(portalUrl,sizeof(portalUrl));
  const char* collectedHeaders[]={"Origin","Sec-Fetch-Site","X-Stach-Token","Cookie","X-Stach-Account","Accept-Encoding"};
  server.collectHeaders(collectedHeaders,sizeof(collectedHeaders)/sizeof(collectedHeaders[0]));
#ifdef STACH_EYES_175
  owner::begin(preferences);
  ui175::loadRecords(preferences);
  if(!owner::claimed) pairing.show(stach::pairing::Screen::Wifi,millis());
  auto accountPost=[](bool signup) {
    if(!guardEnvelope())return;
    if(server.header("X-Stach-Account")!="1"){
      server.send(403,"text/plain","Missing account request header");return;
    }
    const bool wasClaimed=owner::claimed;
    owner::authenticate(server,preferences,signup);
    if(signup && !wasClaimed && owner::claimed) switchMode(stach::modes::Mode::Face,millis());
  };
  server.on("/account/register",HTTP_POST,[accountPost](){accountPost(true);});
  server.on("/account/login",HTTP_POST,[accountPost](){accountPost(false);});
  server.on("/api/logout",HTTP_POST,[](){
    if(!guardRequest(true))return;
    owner::logout(server);
    server.send(200,"application/json","{}");
  });
  server.on("/api/name",HTTP_POST,[](){
    if(!guardRequest(true))return;
    String name=server.arg("name"); name.trim();
    if(!owner::validName(name)){server.send(400,"text/plain","Invalid name");return;}
    if(preferences.putString("displayName",name)!=name.length()){server.send(500,"text/plain","Save failed");return;}
    owner::deviceName=name;
    server.send(200,"application/json","{}");
  });
#endif
  server.on("/api/session", HTTP_GET, [](){
    if(!guardRequest(false))return;
    // Same-origin read only. A random token expires on every reboot.
    server.send(200,"application/json",String("{\"token\":\"")+sessionToken+"\"}");
  });
  // On-device boot self-test snapshot. This is not a claim of physical validation.
  // No Wi-Fi password, BLE pairing key or session token is exposed here.
  server.on("/api/hardware", HTTP_GET, [](){
    if(!guardRequest(false))return;
#ifdef STACH_WAVESHARE_BOARD
#if defined(STACH_PANEL_CO5300) || defined(STACH_CANVAS_CO5300)
    const char* panelProfile="CO5300";
#else
    const char* panelProfile="SH8601";
#endif
    const auto& e=bootHardwareReport.evidence;
    String body=String("{\"supported\":true,\"hardwareValidated\":false,\"panelProfile\":\"") + panelProfile +
      "\",\"flashBytes\":"+String(e.flashBytes)+
      ",\"psramBytes\":"+String(e.psramBytes)+
      ",\"i2cBus\":"+(e.i2cBus?"true":"false")+
      ",\"touchI2CAck\":"+(e.touchAck?"true":"false")+
      ",\"imuI2CAck\":"+(e.imuAck?"true":"false")+
      ",\"rtcI2CAck\":"+(e.rtcAck?"true":"false")+
      ",\"displayBegin\":"+(e.displayBegin?"true":"false")+
      ",\"touchBegin\":"+(e.touchBegin?"true":"false")+
      ",\"imuBegin\":"+(e.imuBegin?"true":"false")+
      ",\"basicDriversReady\":"+(bootHardwareReport.basicDriversReady?"true":"false")+
      ",\"warningMask\":"+String(bootHardwareReport.warnings)+"}";
    server.send(200,"application/json",body);
#else
    server.send(200,"application/json","{\"supported\":false,\"hardwareValidated\":false}");
#endif
  });
  server.on("/api/health", HTTP_GET, [](){
    if(!guardRequest(false))return;
    String body=String("{\"uptimeMs\":")+String(millis())+
      ",\"freeHeap\":"+String(ESP.getFreeHeap())+
      ",\"minFreeHeap\":"+String(ESP.getMinFreeHeap())+
      ",\"apHealthy\":"+(apHealthy?"true":"false")+
      ",\"apClients\":"+String(WiFi.softAPgetStationNum())+"}";
    server.send(200,"application/json",body);
  });
  server.on("/", HTTP_GET, [](){
    if(!guardRequest(false))return;
    String encodings=server.header("Accept-Encoding");
    encodings.toLowerCase();
    int start=0;
    while(start<(int)encodings.length()) {
      int end=encodings.indexOf(',',start);
      if(end<0) end=encodings.length();
      String coding=encodings.substring(start,end);
      coding.trim();
      if(coding=="gzip") {
        server.sendHeader("Content-Encoding","gzip");
        server.send_P(200,"text/html; charset=utf-8",reinterpret_cast<const char*>(stach::web::compressedPage),sizeof(stach::web::compressedPage));
        return;
      }
      start=end+1;
    }
    server.send_P(200,"text/html; charset=utf-8",stach::web::page);
  });
  server.on("/api/status", HTTP_GET, [](){
    if(!guardRequest(false))return;
    const uint32_t now=millis();
    const uint64_t uptime=uint64_t(esp_timer_get_time())/1000ULL;
    const uint32_t remain=manualUntil>uptime?uint32_t(manualUntil-uptime):0;
    String body = String("{\"name\":\"") +
#ifdef STACH_EYES_175
      owner::deviceName + "\",\"ownerAccount\":true,\"ownerLogin\":\"" + owner::stored.login +
#else
      "Stach" +
#endif
      "\",\"expression\":\"" + stach::emotionName(character.current) +
      "\",\"ip\":\"" + WiFi.softAPIP().toString() + "\",\"ssid\":\"" + ssid +
      "\",\"pairingScreen\":\"" + stach::pairing::name(pairing.screen) +
      "\",\"deviceMode\":\"" + stach::modes::name(currentMode()) +
      "\",\"chargingKnown\":" + (behavior.chargingKnown?"true":"false") +
      ",\"charging\":" + (behavior.charging?"true":"false") +
      ",\"batteryKnown\":" + (behavior.batteryKnown?"true":"false") +
      ",\"batteryPercent\":" + (behavior.batteryKnown?String(behavior.batteryPercent):String("null")) +
#ifdef STACH_EYES_175
      ",\"batterySource\":\"AXP2101\",\"batteryPresent\":" + (ui175::power.valid?(ui175::power.battery?"true":"false"):"null") +
#endif
      ",\"batteryAdcMv\":" + (batteryMeter.adcMilliVolts>0?String(batteryMeter.adcMilliVolts):String("null")) +
      ",\"batteryMv\":" + (batteryMeter.batteryMilliVolts>0?String(batteryMeter.batteryMilliVolts):String("null")) +
      ",\"batteryCalibrated\":" + (stach::power::validRatio(batteryMeter.ratio)?"true":"false") +
      ",\"powerSaver\":" + (saver.enabled?"true":"false") +
      ",\"eyeBrightness\":" + String(appearance.brightness) +
      ",\"gazeActivity\":" + String(appearance.gazeActivity) +
      ",\"blinkEnabled\":" + (appearance.blink?"true":"false") +
      ",\"screenMode\":\"" + stach::power::modeName(saver.current) + "\"" +
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
      ",\"mouseMode\":" + (airMouse.active()?"true":"false") +
      ",\"mouseBleStarted\":" +
#ifdef STACH_WAVESHARE_BOARD
      (hidMouse.started()?"true":"false") +
#else
      String("false") +
#endif
      ",\"mouseConnected\":" +
#ifdef STACH_WAVESHARE_BOARD
      (hidMouse.connected()?"true":"false") +
#else
      String("false") +
#endif
      ",\"mouseCalibrated\":" + (airMouse.calibrated()?"true":"false") +
      ",\"mouseCalibratingSamples\":" + String(airMouse.calibrationSamples()) +
      ",\"mouseSensitivity\":" + String(airMouse.config.sensitivity,2) +
      ",\"mouseInvertX\":" + (airMouse.config.invertX?"true":"false") +
      ",\"mouseInvertY\":" + (airMouse.config.invertY?"true":"false") +
      ",\"mouseAxisYX\":" + (airMouse.config.axisYX?"true":"false") +
      ",\"clients\":" + String(WiFi.softAPgetStationNum()) +
      ",\"uptimeMs\":" + String(now) + "}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(200,"application/json; charset=utf-8",body);
  });
  server.on("/api/expression", HTTP_POST, [](){
    if(!guardRequest(true))return;
    saver.activity(millis());
    stach::Emotion next = stach::parseEmotion(server.arg("value").c_str());
    if (next == stach::Emotion::Invalid) {server.send(400,"text/plain","Invalid expression");return;}
    character.set(next,millis());
    manualUntil=uint64_t(esp_timer_get_time())/1000ULL+8000; // 8-second manual preview override
    server.send(200,"text/plain",String("Expression: ")+stach::emotionName(character.current));
  });
  // Centralized mode endpoint: explicit, reversible, and nonpersistent.
  server.on("/api/mode", HTTP_POST, [](){
    if(!guardRequest(true))return;
    stach::modes::Mode next;
    if(!stach::modes::parse(server.arg("mode").c_str(),next)) {
      server.send(400,"application/json","{\"error\":\"Invalid mode\"}");return;
    }
    if(!switchMode(next,millis())) {
      server.send(503,"application/json","{\"error\":\"Mouse mode requires ESP32 BLE hardware\"}");return;
    }
    server.sendHeader("Cache-Control","no-store");
    server.send(200,"application/json",String("{\"ok\":true,\"mode\":\"")+stach::modes::name(currentMode())+"\"}");
  });
  // Only validated appearance preferences are stored in NVS.
  server.on("/api/appearance", HTTP_GET, [](){
    if(!guardRequest(false))return;
    server.sendHeader("Cache-Control","no-store");
    server.send(200,"application/json",String("{\"brightness\":")+String(appearance.brightness)+
      ",\"gazeActivity\":"+String(appearance.gazeActivity)+
      ",\"blink\":"+(appearance.blink?"true":"false")+"}");
  });
  server.on("/api/appearance", HTTP_POST, [](){
    if(!guardRequest(true))return;
    const String key=server.arg("key"),value=server.arg("value");
    if(key=="brightness" || key=="gazeActivity") {
      unsigned n=0;
      if(!stach::modes::parsePercent(value.c_str(),n) ||
         (key=="brightness"?!stach::modes::validBrightness(n):!stach::modes::validGazeActivity(n))) {
        server.send(400,"application/json","{\"error\":\"Value outside allowed range\"}");return;
      }
      const char* store=key=="brightness"?"eyeBright":"eyeMotion";
      if(preferences.putUChar(store,uint8_t(n))!=sizeof(uint8_t)) {
        server.send(500,"application/json","{\"error\":\"NVS save failed\"}");return;
      }
      if(key=="brightness")appearance.brightness=uint8_t(n);
      else appearance.gazeActivity=uint8_t(n);
    } else if(key=="blink") {
      bool b=false;
      if(!stach::control::parseSwitch(value.c_str(),b)) {
        server.send(400,"application/json","{\"error\":\"Blink must be 0 or 1\"}");return;
      }
      if(preferences.putBool("blink",b)!=1) {
        server.send(500,"application/json","{\"error\":\"NVS save failed\"}");return;
      }
      appearance.blink=b;
      character.blinkEnabled=b;
    } else {
      server.send(400,"application/json","{\"error\":\"Unknown appearance setting\"}");return;
    }
    saver.activity(millis());
    server.sendHeader("Cache-Control","no-store");
    server.send(200,"application/json","{\"ok\":true}");
  });
  // AP clients can ask the physical Stach to display a QR code.
  // Never return the persistent Wi-Fi password over /api/status.
  server.on("/api/pairing", HTTP_POST, [](){
    if(!guardRequest(true))return;
    saver.activity(millis());
    stach::pairing::Screen next;
    if(!stach::pairing::parseScreen(server.arg("mode").c_str(),next)){
      server.send(400,"text/plain","Invalid pairing screen");return;
    }
    const stach::modes::Mode target = next==stach::pairing::Screen::Wifi?
      stach::modes::Mode::Wifi:next==stach::pairing::Screen::Portal?
      stach::modes::Mode::Portal:stach::modes::Mode::Face;
    if(!switchMode(target,millis())){server.send(503,"text/plain","Mode unavailable");return;}
    server.send(200,"text/plain",String("Stach screen: ")+stach::pairing::name(pairing.screen));
  });
  // A short-lived, bounded remote gaze. Values are normalized [-1, 1].
  server.on("/api/gaze", HTTP_POST, [](){
    if(!guardRequest(true))return;
    saver.activity(millis());
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
    if(!guardRequest(false))return;
    const String body=String("{\"idleMotion\":") + (settings.idleMotion?"true":"false") +
      ",\"tiltEnabled\":" + (settings.tiltEnabled?"true":"false") + "}";
    server.sendHeader("Cache-Control","no-store");
    server.send(200,"application/json",body);
  });
  server.on("/api/settings", HTTP_POST, [](){
    if(!guardRequest(true))return;
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
  // Power saving can be toggled independently of BLE, gaze and touch.
  server.on("/api/power/saver", HTTP_POST, [](){
    if(!guardRequest(true))return;
    bool enabled=false;
    if(!stach::control::parseSwitch(server.arg("enabled").c_str(),enabled)){
      server.send(400,"application/json","{\"error\":\"Invalid enabled value\"}");return;
    }
    if(preferences.putBool("psaver",enabled)!=1){
      server.send(500,"application/json","{\"error\":\"NVS write failed\"}");return;
    }
    saver.enabled=enabled;saver.activity(millis());
    server.send(200,"application/json","{\"ok\":true}");
  });
  server.on("/api/power/wake", HTTP_POST, [](){
    if(!guardRequest(true))return;
    saver.activity(millis());
    server.send(200,"application/json","{\"ok\":true}");
  });
  // Not an automatic calibration: user must FIRST verify with a multimeter
  // that BAT_ADC tracks the real lithium CELL on the exact board revision.
  server.on("/api/power/calibrate", HTTP_POST, [](){
    if(!guardRequest(true))return;
#ifdef STACH_EYES_175
    server.send(409,"text/plain","AXP2101 provides battery telemetry directly");
#elif defined(STACH_WAVESHARE_BOARD)
    const String value=server.arg("cellMv");
    if(value.length()<4 || value.length()>4){server.send(400,"application/json","{\"error\":\"Invalid measured voltage\"}");return;}
    for(unsigned i=0;i<value.length();++i)if(value[i]<'0'||value[i]>'9'){
      server.send(400,"application/json","{\"error\":\"Digits only\"}");return;
    }
    float ratio=0;
    if(!stach::power::Meter::measuredRatio(value.toInt(),batteryMeter.adcMilliVolts,ratio)){
      server.send(400,"application/json","{\"error\":\"ADC or measured cell voltage invalid\"}");return;
    }
    if(preferences.putFloat("batRatio",ratio)!=sizeof(float)){
      server.send(500,"application/json","{\"error\":\"NVS write failed\"}");return;
    }
    batteryMeter.configure(ratio);saver.activity(millis());
    server.send(200,"application/json","{\"ok\":true,\"note\":\"Voltage-derived percentage is approximate\"}");
#else
    server.send(503,"application/json","{\"error\":\"No battery ADC\"}");
#endif
  });
  // BLE control commands are only accepted from local Wi-Fi AP clients.
  // No automatic HID action on boot; no HID action without mouse mode+BLE link.
  server.on("/api/mouse/mode", HTTP_POST, [](){
    if(!guardRequest(true))return;
    saver.activity(millis());
    bool next=false;
    if(!stach::control::parseSwitch(server.arg("enabled").c_str(),next)) {
      server.send(400,"application/json","{\"error\":\"Invalid mode\"}");return;
    }
    if(!switchMode(next?stach::modes::Mode::Mouse:stach::modes::Mode::Face,millis())) {
      server.send(503,"application/json","{\"error\":\"Hardware BLE unavailable\"}");return;
    }
    server.send(200,"application/json","{\"ok\":true}");
  });
  server.on("/api/mouse/calibrate", HTTP_POST, [](){
    if(!guardRequest(true))return;
    if(!airMouse.active()) {server.send(409,"application/json","{\"error\":\"Enable Mouse Mode first\"}");return;}
    airMouse.recalibrate(millis());
    server.send(200,"application/json","{\"ok\":true}");
  });
  server.on("/api/mouse/config", HTTP_POST, [](){
    if(!guardRequest(true))return;
    const String key=server.arg("key");const String value=server.arg("value");
    stach::mouse::Config next=airMouse.config;
    if(key=="sensitivity"){
      float n=0;
      if(!stach::mouse::parseSensitivity(value.c_str(),n)){
        server.send(400,"application/json","{\"error\":\"Sensitivity out of range\"}");return;}
      next.sensitivity=n;
      if(preferences.putFloat("msens",n)!=sizeof(float)){
        server.send(500,"application/json","{\"error\":\"Save failed\"}");return;}
    }else{
      bool b=false;
      if(!stach::control::parseSwitch(value.c_str(),b)){
        server.send(400,"application/json","{\"error\":\"Invalid option\"}");return;}
      const char* store=nullptr;
      if(key=="invertX"){next.invertX=b;store="minvx";}
      else if(key=="invertY"){next.invertY=b;store="minvy";}
      else if(key=="axisYX"){next.axisYX=b;store="maxis";}
      else {server.send(400,"application/json","{\"error\":\"Unknown option\"}");return;}
      if(preferences.putBool(store,b)!=1){
        server.send(500,"application/json","{\"error\":\"Save failed\"}");return;}
    }
    airMouse.configure(next,millis());
    server.send(200,"application/json","{\"ok\":true}");
  });
  server.on("/api/mouse/click", HTTP_POST, [](){
    if(!guardRequest(true))return;
#ifdef STACH_WAVESHARE_BOARD
    const String b=server.arg("button");
    stach::mouse::Button button=stach::mouse::Button::None;
    if(b=="left")button=stach::mouse::Button::Left;
    else if(b=="right")button=stach::mouse::Button::Right;
    else if(b=="middle")button=stach::mouse::Button::Middle;
    if(button==stach::mouse::Button::None){server.send(400,"application/json","{\"error\":\"Unknown mouse button\"}");return;}
    if(!hidMouse.click(airMouse,button)) {server.send(409,"application/json","{\"error\":\"Mouse inactive or BLE disconnected\"}");return;}
    server.send(200,"application/json","{\"ok\":true}");
#else
    server.send(503,"application/json","{\"error\":\"BLE unavailable\"}");
#endif
  });
  server.on("/api/mouse/move", HTTP_POST, [](){
    if(!guardRequest(true))return;
#ifdef STACH_WAVESHARE_BOARD
    int8_t x=0,y=0;
    if(!stach::mouse::parseDelta(server.arg("dx").c_str(),x)||
       !stach::mouse::parseDelta(server.arg("dy").c_str(),y)){
      server.send(400,"application/json","{\"error\":\"Invalid movement\"}");return;
    }
    if(!hidMouse.move(airMouse,x,y)){server.send(409,"application/json","{\"error\":\"BLE not connected\"}");return;}
    server.send(200,"application/json","{\"ok\":true}");
#else
    server.send(503,"application/json","{\"error\":\"BLE unavailable\"}");
#endif
  });
  server.on("/api/mouse/scroll", HTTP_POST, [](){
    if(!guardRequest(true))return;
#ifdef STACH_WAVESHARE_BOARD
    const String dir=server.arg("direction");int8_t v=0,h=0;
    if(dir=="up")v=3;else if(dir=="down")v=-3;
    else if(dir=="left")h=-3;else if(dir=="right")h=3;
    else{server.send(400,"application/json","{\"error\":\"Invalid scroll direction\"}");return;}
    if(!hidMouse.scroll(airMouse,v,h)){server.send(409,"application/json","{\"error\":\"BLE not connected\"}");return;}
    server.send(200,"application/json","{\"ok\":true}");
#else
    server.send(503,"application/json","{\"error\":\"BLE unavailable\"}");
#endif
  });
  server.onNotFound([](){if(!guardRequest(false))return;server.send(404,"text/plain","Not found");});
  server.enableDelay(false); // loop() already yields between requests
  server.begin();
  initialized = true;
  Serial.println("Stach AP: " + ssid);
  // Intentionally avoid leaking the persistent password in serial logs.
  Serial.println("Open " + String(portalUrl) + " after joining Stach Wi-Fi");
  Serial.println("Long-press Stach's display to show the Wi-Fi QR, tap for portal QR.");
}
void loop() {
  if (initialized) server.handleClient();
  // Recover from a reset of the AP Wi-Fi mode without losing current settings.
  const uint32_t wifiTick=millis();
  if(initialized && uint32_t(wifiTick-lastApRecovery)>=20000){
    lastApRecovery=wifiTick;
    if(!apHealthy || WiFi.getMode()!=WIFI_AP || WiFi.softAPIP()!=IPAddress(192,168,4,1)){
      if(startAccessPoint()) Serial.println("[WIFI] Stach AP restored");
      else Serial.println("[WIFI] Recovery attempt failed; retry scheduled");
    }
  }
  const uint32_t tickMs=millis();
#ifdef STACH_WAVESHARE_BOARD
  // USB serial-only diagnostic commands. No unauthenticated HTTP mutation.
  // The temporary pattern is ignored while Mouse Mode is active.
  if(Serial.available()>0){
    const int command=Serial.read();
    if((command=='D' || command=='d') && faceDisplayReady && !airMouse.active()){
      hardwareTestUntilMs=tickMs+5000;
      hardwareTestVisible=false;
      wasScreenDark=false; // ensure a previously sleeping screen can be cleaned up
      saver.activity(tickMs);
      Serial.println("[HWSELFTEST] Temporary AMOLED pattern requested (5 seconds)");
    }else if(command=='S' || command=='s'){
      stach::hardware::scanI2C();
    }else if(command=='I' || command=='i'){
#ifdef STACH_EYES_175
      Serial.printf("[STACH 1.75] app=%u ap=%u ssid=%s ip=%s mode=%s power=%u usb=%u battery=%u percent=%d\n",
        initialized,apHealthy,ssid.c_str(),WiFi.softAPIP().toString().c_str(),stach::modes::name(currentMode()),
        ui175::power.valid,ui175::power.usb,ui175::power.battery,ui175::power.percent);
#endif
#ifdef STACH_EYES_175
      Serial.printf("[OWNER] state=%s storage=%u screen=%s\n",owner::claimed?"registered":"setup",owner::storageOK,stach::pairing::name(pairing.screen));
#endif
      const auto& e=bootHardwareReport.evidence;
      Serial.printf("[HWSELFTEST] flash=%lu psram=%lu i2c=%u display=%u touch=%u imu=%u rtc=%u warnings=0x%03X basicDrivers=%s\n",
        (unsigned long)e.flashBytes,(unsigned long)e.psramBytes,
        (unsigned)e.i2cBus,(unsigned)e.displayBegin,
        (unsigned)e.touchBegin,(unsigned)e.imuBegin,(unsigned)e.rtcAck,
        (unsigned)bootHardwareReport.warnings,
        bootHardwareReport.basicDriversReady?"OK":"CHECK");
    }
  }
#endif
#ifdef STACH_EYES_175
  if(owner::claimed)
#endif
  pairing.tick(tickMs);
  const uint64_t uptimeMs=uint64_t(esp_timer_get_time())/1000ULL;
#ifdef STACH_WAVESHARE_BOARD
#ifdef STACH_EYES_175
  if(tickMs-ui175::lastPower>=500) {
    ui175::lastPower=tickMs;
    ui175::readPower(tickMs);
    if(!owner::claimed) {
      static bool hadClient=false;
      const bool connected=WiFi.softAPgetStationNum()>0;
      if(connected && !hadClient && pairing.screen==stach::pairing::Screen::Wifi)
        pairing.show(stach::pairing::Screen::Portal,tickMs);
      hadClient=connected;
    }
    behavior.battery(ui175::power.percent);
    batteryMeter.batteryMilliVolts=ui175::power.millivolts;
    behavior.charge(ui175::power.valid,ui175::power.charging,uptimeMs);
    if(ui175::splash) saver.activity(tickMs);
  }
  if(ui175::splash && tickMs-ui175::splashAt>=ui175::splashDuration) {
    ui175::splash=false;
    pairingNeedsRedraw=true;
    drawnScreen=stach::pairing::Screen::Face;
    mouseScreenShown=false;
  }
#else
  static uint32_t lastBatteryPoll=0;
  if(uint32_t(tickMs-lastBatteryPoll)>=4000){
    lastBatteryPoll=tickMs;
    batteryMeter.observe(stach::power::adcSampleMilliVolts());
    behavior.battery(batteryMeter.known()?batteryMeter.batteryPercent:-1);
    bool charging=false;
    const bool chargingKnown=stach::power::readCharging(charging);
    behavior.charge(chargingKnown,charging,uptimeMs);
  }
#endif
  airMouse.link(hidMouse.connected(),tickMs);
  static uint64_t lastImuMs=0;
  if(imuReady && uptimeMs-lastImuMs>=20){
    lastImuMs=uptimeMs;
    stach::motion::Sample current;
    if(imu.read(current)){
      lastMotion=current;
      const float g=sqrtf(current.ax*current.ax+current.ay*current.ay+current.az*current.az);
      behavior.acceleration(g,uptimeMs);
      tilt.update(current);
      hidMouse.send(airMouse,airMouse.update(current,tickMs));
    }
  }
  static uint32_t lastTouchPoll=0;
  if(touchReady && uint32_t(tickMs-lastTouchPoll)>=5){
    lastTouchPoll=tickMs;
    stach::touch::Point point;
    if(touchDriver.read(point)){
      if(point.down && !touchRecognizer.pressed())saver.activity(tickMs);
#ifdef STACH_EYES_175
      if(point.down && !airMouse.active() && pairing.screen==stach::pairing::Screen::Face && ui175::screen==ui175::Screen::Face) {
        // Present the first contact immediately; keep drag updates frame-paced.
        if(!touchRecognizer.pressed()) pairingNeedsRedraw=true;
        fingerAt=tickMs;
        hasFinger=true;
        fingerX=constrain((point.x-233)/195.f,-1.f,1.f);
        fingerY=constrain((point.y-233)/195.f,-1.f,1.f);
      }
#endif
      lastTouchValidAt=tickMs;
      const stach::touch::Event event=touchRecognizer.feed(point,tickMs
#ifdef STACH_EYES_175
        ,ui175::screen==ui175::Screen::Face
#endif
      );
      if(event.kind!=stach::touch::Gesture::None){
        lastGesture=event.kind;lastGestureAt=tickMs;
        if(airMouse.active()) {
          mouseGesture(event.kind,tickMs);
#ifdef STACH_EYES_175
        } else if(pairing.screen==stach::pairing::Screen::Face && ui175::gameGesture(event,tickMs)) {
          saver.activity(tickMs);
          hasFinger=false;
#endif
        } else if(pairing.screen==stach::pairing::Screen::Face &&
                  event.kind==stach::touch::Gesture::SwipeUp) {
          switchMode(stach::modes::Mode::Mouse,tickMs); // deliberate gesture to enter Mouse Mode
        } else {
          const bool changedPairing = pairing.gesture(event.kind,tickMs);
          if(!changedPairing && pairing.screen==stach::pairing::Screen::Face) {
            gestureGaze.event(event.kind,tickMs);
            behavior.touch(uptimeMs);
          }
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
  saver.tick(tickMs,
#ifdef STACH_EYES_175
             ui175::screen!=ui175::Screen::Face ||
#endif
             airMouse.active() || behavior.showCharging() || pairing.screen!=stach::pairing::Screen::Face,
             behavior.batteryKnown,behavior.batteryPercent);
#ifdef STACH_WAVESHARE_BOARD
  static uint32_t lastFrame=0;
  const uint32_t now=millis();
  if(faceDisplayReady) {
    // An explicit diagnostic pattern has priority over normal face/QR screens.
    // It does NOT imply that the physical panel passed visual inspection.
    const bool showDiagnostic=hardwareTestUntilMs && int32_t(hardwareTestUntilMs-now)>0;
    if(!showDiagnostic) hardwareTestUntilMs=0;
    if(hardwareTestVisible && (!showDiagnostic || airMouse.active())){
      hardwareTestVisible=false;
      hardwareTestUntilMs=0;
      pairingNeedsRedraw=true;
      drawnScreen=stach::pairing::Screen::Portal;
      mouseScreenShown=false;
      lastFrame=0;
#ifdef STACH_EYES_175
      ui175::gameDirty=true;
#endif
    }
    if(showDiagnostic && !airMouse.active()) {
      if(!hardwareTestVisible){
        stach::display::diagnosticPattern();
        hardwareTestVisible=true;
      }
    } else
#ifdef STACH_EYES_175
    if(ui175::splash) {
      if(now-lastFrame>=50) {
        lastFrame=now;
        ui175::drawSplash(now-ui175::splashAt);
      }
    } else if(ui175::screen!=ui175::Screen::Face && !airMouse.active() && pairing.screen==stach::pairing::Screen::Face) {
      if(ui175::gameNeedsRedraw(now,preferences)) {
        lastFrame=now;
        ui175::drawGames(now);
        ui175::batteryBar();
        stach::display::flush();
        pairingNeedsRedraw=true;
      }
    } else
#endif
    {
    if(saver.current==stach::power::ScreenMode::Dark) {
      if(!wasScreenDark){stach::display::panel()->fillScreen(0);stach::display::flush();wasScreenDark=true;}
    } else {
    if(wasScreenDark){wasScreenDark=false;pairingNeedsRedraw=true;drawnScreen=stach::pairing::Screen::Portal;lastFrame=0;}
    if(airMouse.active()) {
      if(!mouseScreenShown || uint32_t(now-lastMouseScreen)>=500) {
        lastMouseScreen=now;
        mouseScreenShown=true;
        stach::mouse_ui::draw(airMouse,hidMouse.started(),imuReady);
        stach::display::flush();
      }
    } else {
      if(mouseScreenShown) { pairingNeedsRedraw=true;mouseScreenShown=false;drawnScreen=stach::pairing::Screen::Portal; }
      if(pairing.screen != stach::pairing::Screen::Face) {
      if(pairingNeedsRedraw || pairing.screen != drawnScreen) {
        pairingNeedsRedraw=false;
        const char* contents = pairing.screen==stach::pairing::Screen::Wifi ? qrPayload : portalUrl;
#ifdef STACH_EYES_175
        char setupUrl[sizeof(portalUrl)+sizeof(owner::invitation)+7];
        bool registering=pairing.screen==stach::pairing::Screen::Portal && !owner::claimed;
        if(registering) {
          snprintf(setupUrl,sizeof(setupUrl),"%s#claim=%s",portalUrl,owner::invitation);
          contents=setupUrl;
        }
#endif
        if(!stach::pairing::renderCode(contents,pairing.screen,
#ifdef STACH_EYES_175
                                       !owner::claimed
#else
                                       false
#endif
                                       )) stach::pairing::renderFailure();
        drawnScreen=pairing.screen;
        stach::display::flush();
      }
    } else if(pairingNeedsRedraw || drawnScreen != stach::pairing::Screen::Face || (uint32_t)(now-lastFrame)>=
#ifdef STACH_EYES_175
        (saver.current==stach::power::ScreenMode::Normal ? 32u : stach::power::Saver::framePeriod(saver.current))
#else
        stach::power::Saver::framePeriod(saver.current)
#endif
      ) {
      pairingNeedsRedraw=false;
      lastFrame=now;
      drawnScreen=stach::pairing::Screen::Face;
      AmoledSurface surface;
      surface.brightness=appearance.brightness; // flat RGB565 eye intensity, no glow
      stach::face::State frame=character.sample(now);
      const float motionScale=float(appearance.gazeActivity)/100.f;
      frame.gazeX*=motionScale;frame.gazeY*=motionScale;
      frame.offsetX*=motionScale;frame.offsetY*=motionScale;
      if (!settings.idleMotion) { frame.gazeX=0; frame.gazeY=0; frame.offsetX=0; frame.offsetY=0; }
      const stach::motion::Tilt neutralTilt;
      gestureGaze.apply(frame,now,settings.tiltEnabled?tilt:neutralTilt);
#ifdef STACH_EYES_175
      if(hasFinger) {
        const uint32_t age=now-fingerAt;
        if(age>=500) hasFinger=false;
        else {
          const float weight=1.f-stach::face::ease(stach::face::clamp((float(age)-250.f)/250.f,0.f,1.f));
          frame.gazeX+=(fingerX-frame.gazeX)*weight;
          frame.gazeY+=(fingerY-frame.gazeY)*weight;
          frame.offsetX+=(17*fingerX-frame.offsetX)*weight;
          frame.offsetY+=(10*fingerY-frame.offsetY)*weight;
        }
      }
#endif
      remoteGaze.apply(frame,now);
#ifdef STACH_EYES_175
      frame.chargeStrength=0; // The 1.75 panel draws its own charging overlay.
#endif
      stach::face::draw(surface,frame);
#ifdef STACH_EYES_175
      if(ui175::power.valid && ui175::power.charging) ui175::drawCharging(now);
      ui175::batteryBar();
      if(now<8500) ui175::label("Hold: Wi-Fi   Swipe > Games",233,370,2,ui175::mint);
#else
      stach::power_ui::draw(batteryMeter,behavior.chargingKnown,behavior.charging);
#endif
      stach::display::flush();
      }
    }
    }
    } // end normal display rendering vs diagnostic pattern
  }
#endif
  delay(1);
}
