#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <esp_system.h>
#include "stach_core.h"

static WebServer server(80);
static Preferences preferences;
static String ssid;
static String password;
static stach::Emotion emotion = stach::Emotion::Neutral;
static char qrPayload[128] = {};
static bool initialized = false;

static const char page[] PROGMEM = R"HTML(<!doctype html>
<html lang="ru"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Stach</title><style>
body{background:#000;color:#fff;font:16px system-ui;text-align:center;margin:0;padding:24px}
.eyes{display:flex;justify-content:center;gap:28px;margin:55px auto}
.eye{width:90px;height:112px;border-radius:50%;background:#fff;display:grid;place-items:center}
.pupil{width:52px;height:72px;background:#000;border-radius:50%}
button{background:#222;color:#fff;border:1px solid #555;border-radius:12px;padding:14px;margin:5px}
</style><h1>Stach</h1><div class="eyes"><div class="eye"><div class="pupil"></div></div><div class="eye"><div class="pupil"></div></div></div>
<div id="buttons"></div><p id="status" role="status"></p><script>
for(const name of ['neutral','angry','sleepy','curious']){
 const b=document.createElement('button');b.textContent=name;
 b.onclick=async()=>{try{const r=await fetch('/api/expression?value='+encodeURIComponent(name),{method:'POST'});
 document.getElementById('status').textContent=await r.text()}catch(e){document.getElementById('status').textContent='Connection error'}};
 document.getElementById('buttons').appendChild(b)}
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
    String body = String("{\"name\":\"Stach\",\"expression\":\"") + stach::emotionName(emotion) +
      "\",\"ip\":\"" + WiFi.softAPIP().toString() + "\"}";
    server.send(200,"application/json",body);
  });
  server.on("/api/expression", HTTP_POST, [](){
    stach::Emotion next = stach::parseEmotion(server.arg("value").c_str());
    if (next == stach::Emotion::Invalid) {server.send(400,"text/plain","Invalid expression");return;}
    emotion = next;
    server.send(200,"text/plain",String("Expression: ")+stach::emotionName(emotion));
  });
  server.onNotFound([](){server.send(404,"text/plain","Not found");});
  server.begin();
  initialized = true;
  Serial.println("Stach AP: " + ssid);
  Serial.println("Stach Wi-Fi password: " + password);
  Serial.println("Wi-Fi QR payload: " + String(qrPayload));
  Serial.println("Open http://192.168.4.1 after joining Stach Wi-Fi");
}
void loop() { if (initialized) server.handleClient(); delay(2); }
