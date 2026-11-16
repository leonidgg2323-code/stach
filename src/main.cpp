#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <esp_system.h>
#include "stach_core.h"
#include "stach_face.h"
#include "stach_character.h"
#include "stach_behavior.h"
#include <esp_timer.h>
#ifdef STACH_WAVESHARE_BOARD
#include "stach_hardware.h"
#include "stach_display.h"
#include "stach_imu.h"
#endif

static WebServer server(80);
static Preferences preferences;
static String ssid;
static String password;
static stach::character::Controller character;
static stach::behavior::Engine behavior;
static uint64_t manualUntil=0;
static char qrPayload[128] = {};
static bool initialized = false;
#ifdef STACH_WAVESHARE_BOARD
static bool faceDisplayReady = false;
static stach::imu::Driver imu;
static bool imuReady=false;
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
</div></div><div id="buttons"></div><p id="status" role="status"></p>
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
 face.style.setProperty('--x',(47*look+4*prep)*w+'px');
 face.style.setProperty('--y',3*Math.abs(look)*w+'px');
 face.style.setProperty('--gx',(15*look)*w+'px');
 face.style.setProperty('--gy','0px');
 leftEye.style.setProperty('--perspective',1+.17*look);
 rightEye.style.setProperty('--perspective',1-.17*look);
}
let started=performance.now();setInterval(()=>motion((performance.now()-started)/1000),32);
fetch('/api/status').then(r=>r.json()).then(s=>{if(names.includes(s.expression))update(s.expression)}).catch(()=>{});
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
  Serial.printf("[IMU] Accelerometer: %s\n",imuReady?"OK":"not available");
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
  if (!WiFi.softAP(ssid.c_str(), password.c_str())) {Serial.println("Wi-Fi AP failed"); return;}
  server.on("/", HTTP_GET, [](){server.send_P(200,"text/html; charset=utf-8",page);});
  server.on("/api/status", HTTP_GET, [](){
    String body = String("{\"name\":\"Stach\",\"expression\":\"") + stach::emotionName(character.current) +
      "\",\"ip\":\"" + WiFi.softAPIP().toString() + "\",\"chargingKnown\":" +
      (behavior.chargingKnown?"true":"false") + "}";
    server.send(200,"application/json",body);
  });
  server.on("/api/expression", HTTP_POST, [](){
    stach::Emotion next = stach::parseEmotion(server.arg("value").c_str());
    if (next == stach::Emotion::Invalid) {server.send(400,"text/plain","Invalid expression");return;}
    character.set(next,millis());
    manualUntil=uint64_t(esp_timer_get_time())/1000ULL+8000; // 8-second manual preview override
    server.send(200,"text/plain",String("Expression: ")+stach::emotionName(character.current));
  });
  server.onNotFound([](){server.send(404,"text/plain","Not found");});
  server.begin();
  initialized = true;
  Serial.println("Stach AP: " + ssid);
  Serial.println("Stach Wi-Fi password: " + password);
  Serial.println("Wi-Fi QR payload: " + String(qrPayload));
  Serial.println("Open http://192.168.4.1 after joining Stach Wi-Fi");
}
void loop() {
  if (initialized) server.handleClient();
  const uint32_t tickMs=millis();
  const uint64_t uptimeMs=uint64_t(esp_timer_get_time())/1000ULL;
#ifdef STACH_WAVESHARE_BOARD
  static uint64_t lastImuMs=0;
  if(imuReady && uptimeMs-lastImuMs>=20){
    lastImuMs=uptimeMs;
    float g=0;
    if(imu.accelerationG(g))behavior.acceleration(g,uptimeMs);
  }
#endif
  if(uptimeMs>=manualUntil){
    character.set(behavior.choose(uptimeMs),tickMs,behavior.showCharging());
  }
  character.tick(tickMs);
#ifdef STACH_WAVESHARE_BOARD
  static uint32_t lastFrame=0;
  const uint32_t now=millis();
  if(faceDisplayReady && (uint32_t)(now-lastFrame)>=100) {
    lastFrame=now;
    AmoledSurface surface;
    stach::face::draw(surface,character.sample(now));
  }
#endif
  delay(2);
}
