
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <esp_system.h>

WebServer server(80);
Preferences prefs;

String apSsid;
String apPassword;
String expression = "neutral";
String qrPayload;

const char PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8">
  <meta name="viewport"
        content="width=device-width,initial-scale=1">
  <title>Stach</title>
  <style>
    body {
      background: #080808;
      color: white;
      font-family: system-ui;
      margin: 0;
      padding: 24px;
      text-align: center;
    }
    .eyes {
      display: flex;
      gap: 32px;
      justify-content: center;
      margin: 55px 0;
    }
    .eye {
      width: 90px;
      height: 112px;
      border-radius: 50%;
      background: white;
      display: grid;
      place-items: center;
    }
    .pupil {
      width: 53px;
      height: 72px;
      border-radius: 50%;
      background: black;
    }
    button {
      background: #242424;
      color: white;
      border: 1px solid #555;
      border-radius: 12px;
      padding: 14px;
      margin: 5px;
    }
  </style>
</head>
<body>
  <h1>Stach</h1>
  <p>Панель управления устройством</p>

  <div class="eyes">
    <div class="eye"><div class="pupil"></div></div>
    <div class="eye"><div class="pupil"></div></div>
  </div>

  <h3>Эмоции</h3>
  <div id="controls"></div>
  <p id="status"></p>

  <script>
    const emotions = [
      "neutral", "angry", "sleepy", "curious"
    ];

    for (const emotion of emotions) {
      const button = document.createElement("button");
      button.textContent = emotion;

      button.onclick = async () => {
        const response = await fetch(
          "/api/expression?value=" +
          encodeURIComponent(emotion),
          { method: "POST" }
        );

        document.getElementById("status")
          .textContent = await response.text();
      };

      document.getElementById("controls")
        .appendChild(button);
    }
  </script>
</body>
</html>
)HTML";

void setup() {
  Serial.begin(115200);

  prefs.begin("stach", false);

  apPassword = prefs.getString("ap_pass", "");

  if (apPassword.length() < 12) {
    char buffer[17];
    uint8_t randomBytes[8];

    esp_fill_random(randomBytes, sizeof(randomBytes));

    for (int i = 0; i < 8; i++) {
      sprintf(
        buffer + i * 2,
        "%02X",
        randomBytes[i]
      );
    }

    apPassword = String(buffer);
    prefs.putString("ap_pass", apPassword);
  }

  uint64_t mac = ESP.getEfuseMac();

  char suffix[7];
  snprintf(
    suffix,
    sizeof(suffix),
    "%06lX",
    (unsigned long)(mac & 0xFFFFFF)
  );

  apSsid = "Stach-" + String(suffix);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(
    apSsid.c_str(),
    apPassword.c_str()
  );

  qrPayload =
    "WIFI:T:WPA;S:" + apSsid +
    ";P:" + apPassword + ";;";

  server.on("/", HTTP_GET, []() {
    server.send_P(
      200,
      "text/html; charset=utf-8",
      PAGE
    );
  });

  server.on("/api/status", HTTP_GET, []() {
    String json =
      "{\"name\":\"Stach\",\"expression\":\"" +
      expression +
      "\",\"ip\":\"" +
      WiFi.softAPIP().toString() +
      "\"}";

    server.send(200, "application/json", json);
  });

  server.on("/api/expression", HTTP_POST, []() {
    String value = server.arg("value");

    if (
      value != "neutral" &&
      value != "angry" &&
      value != "sleepy" &&
      value != "curious"
    ) {
      server.send(
        400,
        "text/plain",
        "Invalid expression"
      );
      return;
    }

    expression = value;

    server.send(
      200,
      "text/plain",
      "Expression: " + expression
    );
  });

  server.begin();

  Serial.println("Stach started");
  Serial.println("WiFi: " + apSsid);
  Serial.println("Password: " + apPassword);
  Serial.println("QR payload: " + qrPayload);
  Serial.println("Web: http://192.168.4.1");
}

void loop() {
  server.handleClient();
  delay(2);
}

