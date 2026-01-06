/*
 * Sample provided by F1ATB - Expanded for Grid Home Assistant Dashboard
 *
 * See also https://github.com/AudunKodehode/JC3248W535EN-Touch-LCD
 */
#include "secrets.h"
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h> //Works with Version 1.6.0 and not 1.6.1 (October 2025)
#include <HTTPClient.h>
#include <WiFi.h>
#include <Wire.h>

// Pin definitions
#define GFX_BL 1
#define TOUCH_ADDR 0x3B
#define TOUCH_SDA 4
#define TOUCH_SCL 8
#define TOUCH_I2C_CLOCK 400000
#define TOUCH_RST_PIN 12
#define TOUCH_INT_PIN 11
#define AXS_MAX_TOUCH_NUMBER 1

// Custom colors for the grid
#define PANEL_RED 0xF800
#define PANEL_GREEN 0x07E0
#define PANEL_BLUE 0x001F
#define PANEL_CYAN 0x07FF
#define PANEL_MAGENTA 0xF81F
#define PANEL_ORANGE 0xFD20
#define PANEL_GRAY 0x8410
#define PANEL_WHITE 0xFFFF
#define PANEL_BLACK 0x0000
#define PANEL_NAVY 0x000F
#define PANEL_YELLOW 0xFFE0

Arduino_ESP32QSPI *bus = new Arduino_ESP32QSPI(45, 47, 21, 48, 40, 39);
Arduino_AXS15231B *g =
    new Arduino_AXS15231B(bus, GFX_NOT_DEFINED, 0, false, 320, 480);
Arduino_Canvas *gfx = new Arduino_Canvas(320, 480, g, 0, 0, 0);

uint16_t touchX, touchY;
bool lastTouched = false;

// UI Grid settings (Rotation 1: 480x320)
const int headerH = 40;
const int colW = 160;
const int rowH = 130;

struct GridButton {
  const char *label;
  const char *domain;
  const char *service;
  const char *entity_id;
  uint16_t color;
};

GridButton panelButtons[6] = {
    {"LUCES", "light", "toggle", "light.salon", PANEL_GREEN},
    {"JARDIN", "script", "turn_on", "script.boton_panel_2", PANEL_CYAN},
    {"CINE", "script", "turn_on", "script.boton_panel_3", PANEL_MAGENTA},
    {"VENTILADOR", "script", "turn_on", "script.boton_panel_5", PANEL_ORANGE},
    {"ALARMA", "input_boolean", "toggle", "input_boolean.alarma", PANEL_RED},
    {"TODO OFF", "script", "turn_on", "script.boton_panel_4", PANEL_GRAY}};

// Prototypes
bool getTouchPoint(uint16_t &x, uint16_t &y);
void connectWiFi();
void callHAService(const char *domain, const char *service,
                   const char *entity_id);
void drawUI();

void setup() {
  Serial.begin(115200);

  // Initialize Display
  gfx->begin();
  gfx->setRotation(1);
  gfx->fillScreen(PANEL_BLACK);

  pinMode(GFX_BL, OUTPUT); // Back Light On
  digitalWrite(GFX_BL, HIGH);

  gfx->setTextSize(2);
  gfx->setTextColor(PANEL_WHITE);
  gfx->setCursor(20, 140);
  gfx->print("CONECTANDO WIFI...");
  gfx->flush();

  // Initialize touch
  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  Wire.setClock(TOUCH_I2C_CLOCK);

  // Configure touch pins
  pinMode(TOUCH_INT_PIN, INPUT_PULLUP);
  pinMode(TOUCH_RST_PIN, OUTPUT);
  digitalWrite(TOUCH_RST_PIN, LOW);
  delay(200);
  digitalWrite(TOUCH_RST_PIN, HIGH);
  delay(200);

  connectWiFi();
  drawUI();
}

void loop() {
  bool touched = getTouchPoint(touchX, touchY);

  if (touched && !lastTouched) {
    if (touchY >= headerH && touchY < (headerH + 2 * rowH)) {
      int col = touchX / colW;
      int row = (touchY - headerH) / rowH;
      int idx = row * 3 + col;

      if (idx >= 0 && idx < 6) {
        Serial.print("Pressed: ");
        Serial.println(panelButtons[idx].label);

        // Visual feedback
        int bx = col * colW + 5;
        int by = headerH + row * rowH + 5;
        gfx->fillRect(bx, by, colW - 10, rowH - 10, PANEL_YELLOW);
        gfx->flush();

        callHAService(panelButtons[idx].domain, panelButtons[idx].service,
                      panelButtons[idx].entity_id);

        delay(300);
        drawUI();
      }
    }
  }

  lastTouched = touched;
  delay(5);
}

void drawUI() {
  gfx->fillScreen(PANEL_BLACK);

  // Header
  gfx->fillRect(0, 0, 480, headerH, PANEL_NAVY);
  gfx->setTextColor(PANEL_WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(10, 10);
  gfx->print("CONTROL HOME ASSISTANT");

  // Grid
  for (int i = 0; i < 6; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = col * colW + 5;
    int y = headerH + row * rowH + 5;
    int w = colW - 10;
    int h = rowH - 10;

    gfx->fillRect(x, y, w, h, panelButtons[i].color);
    gfx->drawRect(x, y, w, h, PANEL_WHITE);

    gfx->setTextColor(PANEL_BLACK);
    gfx->setTextSize(2);
    gfx->setCursor(x + 10, y + h / 2 - 10);
    gfx->print(panelButtons[i].label);
  }

  // Footer
  gfx->setTextSize(1);
  gfx->setTextColor(PANEL_WHITE);
  gfx->setCursor(10, 305);
  if (WiFi.status() == WL_CONNECTED) {
    gfx->print("WiFi OK - IP: ");
    gfx->print(WiFi.localIP().toString());
  } else {
    gfx->print("WiFi ERROR");
  }
  gfx->flush();
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 20) {
    delay(500);
    Serial.print(".");
    counter++;
  }
}

void callHAService(const char *domain, const char *service,
                   const char *entity_id) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(HA_URL) + "/api/services/" + String(domain) + "/" +
                 String(service);
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> doc;
    doc["entity_id"] = entity_id;
    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode > 0) {
      Serial.print("HA Resp: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("HA Error: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}

bool getTouchPoint(uint16_t &x, uint16_t &y) {
  uint8_t data[(AXS_MAX_TOUCH_NUMBER * 6 + 2)] = {0};

  const uint8_t read_cmd[11] = {
      0xb5,
      0xab,
      0xa5,
      0x5a,
      0x00,
      0x00,
      (uint8_t)((AXS_MAX_TOUCH_NUMBER * 6 + 2) >> 8),
      (uint8_t)((AXS_MAX_TOUCH_NUMBER * 6 + 2) & 0xff),
      0x00,
      0x00,
      0x00};

  Wire.beginTransmission(TOUCH_ADDR);
  Wire.write(read_cmd, 11);
  if (Wire.endTransmission() != 0)
    return false;

  if (Wire.requestFrom((uint16_t)TOUCH_ADDR, (uint8_t)sizeof(data)) !=
      (uint8_t)sizeof(data))
    return false;

  for (int i = 0; i < (int)sizeof(data); i++) {
    data[i] = Wire.read();
  }

  if (data[1] > 0 && data[1] <= AXS_MAX_TOUCH_NUMBER) {
    uint16_t rawX = ((data[2] & 0x0F) << 8) | data[3];
    uint16_t rawY = ((data[4] & 0x0F) << 8) | data[5];
    if (rawX > 500 || rawY > 500)
      return false;
    y = map(rawX, 0, 320, 320, 0);
    x = rawY;
    return true;
  }
  return false;
}
