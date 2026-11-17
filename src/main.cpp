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

static const char page[] PROGMEM = R"HTML(<!doctype html>
<html lang="ru"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Stach</title><style>
*{box-sizing:border-box}body{background:#101010;color:white;font:16px system-ui;margin:0;text-align:center;padding:20px}
h1{font-size:24px}.preview{width:min(78vw,466px);aspect-ratio:1;border-radius:50%;
background:#000;border:3px solid #555;position:relative;overflow:hidden;margin:18px auto}
.face{position:absolute;inset:0;transform:translate(var(--x,0px),var(--y,0px));transition:transform .35s ease-in-out}
.eye{position:absolute;background:#fff;border-radius:50%;transform:scale(var(--perspective,1),calc(var(--blink,1)*var(--perspective,1)));transition:transform .18s ease}
.eye.left{left:21.9%;top:33.5%;width:23.6%;height:31.3%}
.eye.right{left:52%;top:33.5%;width:23.6%;height:31.3%}
.pupil{position:absolute;background:#000;border-radius:50%;left:29%;top:24%;width:42%;height:52%;
transform:translate(var(--gx,0px),var(--gy,0px));transition:transform .4s ease}
.eye.right .pupil{left:29%;top:24%;width:42%;height:52%}
.brow{position:absolute;background:#fff;border-radius:15px;height:1.2%;width:15%;opacity:0;transition:opacity .2s,transform .3s}
.brow.left{left:25%;top:26%;transform:rotate(0deg)}
.brow.right{left:58%;top:26%;transform:rotate(0deg)}
.face.angry .brow{opacity:1}.face.angry .brow.left{transform:rotate(17deg)}
.face.angry .brow.right{transform:rotate(-17deg)}
.face.sleepy .brow{opacity:.75}.face.sleepy .brow.left{transform:translateY(9px)}
.face.sleepy .brow.right{transform:translateY(9px)}
.face.curious .brow{opacity:1}.face.curious .brow.left{transform:translateY(7px)}
.face.curious .brow.right{transform:translateY(-10px)}
.face.happy .brow{opacity:.85}.face.happy .brow.left{transform:rotate(-9deg)}
.face.happy .brow.right{transform:rotate(9deg)}
.face.angry{--blink:.76}.face.sleepy{--blink:.47}.face.happy{--blink:.80}
button{background:#222;color:white;border:1px solid #666;border-radius:10px;padding:12px;margin:4px;font:inherit;cursor:pointer}
button.active{border-color:#fff;background:#444}p{color:#bbb}
</style><h1>Stach</h1>
<div class="preview" aria-label="Матовые белые глаза на чёрном фоне">
<div class="face neutral" id="face"><div class="brow left"></div><div class="brow right"></div>
<div class="eye left"><div class="pupil"></div></div><div class="eye right"><div class="pupil"></div></div>
</div></div><div id="buttons"></div>
<div><button onclick="setPairing('wifi')">QR Wi-Fi на Stach</button>
<button onclick="setPairing('portal')">QR панели на Stach</button>
<button onclick="setPairing('face')">Вернуть глаза</button></div>
<p id="pairingStatus">Подключение: ожидание</p>
<p id="status" role="status"></p><p id="sensorStatus" aria-live="polite">Сенсор: ожидание данных</p>
<script>
const names=['neutral','angry','sleepy','curious','happy'];
const face=document.getElementById('face'),status=document.getElementById('status');
let expression='neutral';
function update(name){expression=name;face.className='face '+name;document.querySelectorAll('button')
.forEach(b=>b.classList.toggle('active',b.dataset.emotion===name));}
for(const name of names){const b=document.createElement('button');b.textContent=name;
b.dataset.emotion=name;b.onclick=async()=>{try{const r=await fetch('/api/expression?value='+encodeURIComponent(name),{method:'POST'});
if(r.ok)update(name);status.textContent=await r.text()}catch(e){status.textContent='Connection error'}};
document.getElementById('buttons').appendChild(b)}update(expression);
const leftEye=document.querySelector('.eye.left'),rightEye=document.querySelector('.eye.right');
function ease(t){t=Math.max(0,Math.min(1,t));return t*t*(3-2*t)}
function motion(t){
 t=t%8;let look=0, prep=0;
 if(t>=1.5&&t<1.74)prep=ease((t-1.5)/.24);
 else if(t>=1.74&&t<2.2)look=-ease((t-1.74)/.46);
 else if(t>=2.2&&t<3.2)look=-1;
 else if(t>=3.2&&t<3.7)look=-(1-ease((t-3.2)/.5));
 else if(t>=4.5&&t<4.74)prep=-ease((t-4.5)/.24);
 else if(t>=4.74&&t<5.2)look=ease((t-4.74)/.46);
 else if(t>=5.2&&t<6.2)look=1;
 else if(t>=6.2&&t<6.7)look=1-ease((t-6.2)/.5);
 const w=face.parentElement.clientWidth/466;
 face.style.setProperty('--x',(18*look+2*prep)*w+'px');
 face.style.setProperty('--y',3*Math.abs(look)*w+'px');
 face.style.setProperty('--gx',(15*look)*w+'px');
 face.style.setProperty('--gy','0px');
 leftEye.style.setProperty('--perspective',1+.17*look);
 rightEye.style.setProperty('--perspective',1-.17*look);
}
let started=performance.now();setInterval(()=>motion((performance.now()-started)/1000),32);
async function refreshStatus(){try{
 const r=await fetch('/api/status');if(!r.ok)return;
 const s=await r.json();if(names.includes(s.expression))update(s.expression);
 document.getElementById('sensorStatus').textContent=
 'Касание: '+(s.touchReady?'подключено':'недоступно')+
 ' · Жест: '+s.lastGesture+
 ' · Наклон: '+(s.imuReady?(Number(s.tiltX).toFixed(2)+', '+Number(s.tiltY).toFixed(2)):'недоступен');
 document.getElementById('pairingStatus').textContent=
 'Экран: '+(s.pairingScreen||'face')+' · Сеть: '+(s.ssid||'Stach')+' · '+(s.ip||'192.168.4.1');
 }catch(e){document.getElementById('sensorStatus').textContent='Нет связи с устройством';}}
async function setPairing(mode){
 try {const r=await fetch('/api/pairing?mode='+encodeURIComponent(mode),{method:'POST'});
 document.getElementById('pairingStatus').textContent=await r.text();}
 catch(e){document.getElementById('pairingStatus').textContent='Нет связи с устройством';}}
refreshStatus();setInterval(refreshStatus,1500);
</script></html>)HTML";

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
  server.on("/", HTTP_GET, [](){server.send_P(200,"text/html; charset=utf-8",page);});
  server.on("/api/status", HTTP_GET, [](){
    String body = String("{\"name\":\"Stach\",\"expression\":\"") + stach::emotionName(character.current) +
      "\",\"ip\":\"" + WiFi.softAPIP().toString() + "\",\"ssid\":\"" + ssid +
      "\",\"pairingScreen\":\"" + stach::pairing::name(pairing.screen) +
      "\",\"chargingKnown\":" +
      (behavior.chargingKnown?"true":"false") +
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
      ",\"gyroZ\":" + String(lastMotion.gz,2) + "}";
    server.send(200,"application/json",body);
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
      gestureGaze.apply(frame,now,tilt);
      stach::face::draw(surface,frame);
    }
  }
#endif
  delay(2);
}
