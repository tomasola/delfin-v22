#include "secrets.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <Wire.h>
#include <lvgl.h>

// Pin definitions
#define GFX_BL 1
#define TOUCH_ADDR 0x3B
#define TOUCH_SDA 4
#define TOUCH_SCL 8
#define TOUCH_I2C_CLOCK 400000
#define TOUCH_RST_PIN 12
#define TOUCH_INT_PIN 11
#define AXS_MAX_TOUCH_NUMBER 1

// GFX Setup
Arduino_ESP32QSPI *bus = new Arduino_ESP32QSPI(45, 47, 21, 48, 40, 39);
Arduino_AXS15231B *g =
    new Arduino_AXS15231B(bus, GFX_NOT_DEFINED, 0, false, 320, 480);
Arduino_Canvas *gfx = new Arduino_Canvas(320, 480, g, 0, 0, 0);

// LVGL buffer
static const uint32_t screenWidth = 480;
static const uint32_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 20];

// UI Objects
lv_obj_t *temp_label;
lv_obj_t *hum_label;
lv_obj_t *status_label;
lv_obj_t *light_switch;

// Periodic HA update
unsigned long lastHAUpdate = 0;
const unsigned long HA_UPDATE_INTERVAL = 5000;

// Display flush callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w,
                            h);
  lv_disp_flush_ready(disp);
}

// Touch sensing logic (original manual I2C)
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
  for (int i = 0; i < (int)sizeof(data); i++)
    data[i] = Wire.read();

  if (data[1] > 0 && data[1] <= AXS_MAX_TOUCH_NUMBER) {
    uint16_t rawX = ((data[2] & 0x0F) << 8) | data[3];
    uint16_t rawY = ((data[4] & 0x0F) << 8) | data[5];
    if (rawX > 500 || rawY > 500)
      return false;
    // Map to screen orientation
    y = map(rawX, 0, 320, 320, 0);
    x = rawY;
    return true;
  }
  return false;
}

// Touch read callback for LVGL
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  if (getTouchPoint(touchX, touchY)) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchX;
    data->point.y = touchY;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

// HA Helpers
void connectWiFi() {
  Serial.print("WiFi: ");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED && counter < 20) {
    delay(500);
    Serial.print(".");
    counter++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("OK");
    // Test base API
    WiFiClient client;
    HTTPClient http;
    String url = String(HA_URL);
    url.trim();
    url += "/api/";
    Serial.print("Testing HA API: [");
    Serial.print(url);
    Serial.println("]");
    if (http.begin(client, url)) {
      http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));
      int code = http.GET();
      Serial.print("HA API Resp: ");
      Serial.println(code);
      if (code == 200)
        Serial.println("API Logic: OK");
      else
        Serial.println(
            "API Logic: FAIL (Check if 'api:' is in configuration.yaml)");
      http.end();
    }
  } else {
    Serial.println("FAIL");
  }
}

void callHAService(const char *domain, const char *service,
                   const char *entity_id) {
  if (WiFi.status() != WL_CONNECTED)
    return;

  WiFiClient client;
  HTTPClient http;
  String url = String(HA_URL);
  url.trim();
  url += "/api/services/" + String(domain) + "/" + String(service);

  Serial.print("HA Service URL: [");
  Serial.print(url);
  Serial.println("]");
  if (http.begin(client, url)) {
    http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> doc;
    doc["entity_id"] = entity_id;
    String body;
    serializeJson(doc, body);

    int code = http.POST(body);
    Serial.print("HA Service Resp: ");
    Serial.println(code);
    if (code < 0) {
      Serial.print("HTTP Error: ");
      Serial.println(http.errorToString(code).c_str());
    }
    http.end();
  } else {
    Serial.println("HA Service: Unable to connect");
  }
}

String getEntityState(const char *entity_id) {
  if (WiFi.status() != WL_CONNECTED)
    return "err";

  WiFiClient client;
  HTTPClient http;
  String url = String(HA_URL);
  url.trim();
  url += "/api/states/" + String(entity_id);

  Serial.print("HA GetState URL: [");
  Serial.print(url);
  Serial.println("]");
  String state = "err";
  if (http.begin(client, url)) {
    http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));
    int code = http.GET();
    Serial.print("HA GetState Resp: ");
    Serial.println(code);
    if (code == 200) {
      String payload = http.getString();
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, payload);
      state = doc["state"].as<String>();
      Serial.print("Entity: ");
      Serial.print(entity_id);
      Serial.print(" State: ");
      Serial.println(state);
    } else if (code < 0) {
      Serial.print("HTTP Error: ");
      Serial.println(http.errorToString(code).c_str());
    }
    http.end();
  } else {
    Serial.println("HA GetState: Unable to connect");
  }
  return state;
}

// LVGL Event
void alarm_switch_event(lv_event_t *e) {
  lv_obj_t *sw = lv_event_get_target(e);
  if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
    callHAService("input_boolean", "turn_on", "input_boolean.alarma");
  } else {
    callHAService("input_boolean", "turn_off", "input_boolean.alarma");
  }
}

// LVGL UI Creation
void createUI() {
  lv_obj_t *screen = lv_scr_act();
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);

  lv_obj_t *title = lv_label_create(screen);
  lv_label_set_text(title, "HOME ASSISTANT");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

  status_label = lv_label_create(screen);
  lv_label_set_text(status_label, "Iniciando...");
  lv_obj_set_style_text_color(status_label, lv_color_hex(0xAAAAAA), 0);
  lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 40);

  // Temperature Panel
  lv_obj_t *temp_panel = lv_obj_create(screen);
  lv_obj_set_size(temp_panel, 200, 100);
  lv_obj_align(temp_panel, LV_ALIGN_CENTER, -110, -30);
  lv_obj_t *t_t = lv_label_create(temp_panel);
  lv_label_set_text(t_t, "Temperatura");
  lv_obj_align(t_t, LV_ALIGN_TOP_MID, 0, 5);
  temp_label = lv_label_create(temp_panel);
  lv_label_set_text(temp_label, "--. -");
  lv_obj_align(temp_label, LV_ALIGN_CENTER, 0, 10);

  // Humidity Panel
  lv_obj_t *hum_panel = lv_obj_create(screen);
  lv_obj_set_size(hum_panel, 200, 100);
  lv_obj_align(hum_panel, LV_ALIGN_CENTER, 110, -30);
  lv_obj_t *h_t = lv_label_create(hum_panel);
  lv_label_set_text(h_t, "Humedad");
  lv_obj_align(h_t, LV_ALIGN_TOP_MID, 0, 5);
  hum_label = lv_label_create(hum_panel);
  lv_label_set_text(hum_label, "--%");
  lv_obj_align(hum_label, LV_ALIGN_CENTER, 0, 10);

  // Alarm Switch
  lv_obj_t *alarm_panel = lv_obj_create(screen);
  lv_obj_set_size(alarm_panel, 420, 80);
  lv_obj_align(alarm_panel, LV_ALIGN_CENTER, 0, 80);
  lv_obj_t *l_t = lv_label_create(alarm_panel);
  lv_label_set_text(l_t, "Control Alarma");
  lv_obj_align(l_t, LV_ALIGN_LEFT_MID, 20, 0);
  light_switch = lv_switch_create(alarm_panel);
  lv_obj_align(light_switch, LV_ALIGN_RIGHT_MID, -20, 0);
  lv_obj_add_event_cb(light_switch, alarm_switch_event, LV_EVENT_VALUE_CHANGED,
                      NULL);
}

void updateHA() {
  if (millis() - lastHAUpdate < HA_UPDATE_INTERVAL)
    return;
  lastHAUpdate = millis();

  if (WiFi.status() == WL_CONNECTED) {
    lv_label_set_text(status_label, "Conectado");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x00FF00), 0);

    String t = getEntityState("sensor.temperatura_andrea_temperature");
    if (t != "err")
      lv_label_set_text_fmt(temp_label, "%s°C", t.c_str());

    String h = getEntityState("sensor.temperatura_andrea_humidity");
    if (h != "err")
      lv_label_set_text_fmt(hum_label, "%s%%", h.c_str());

    String l = getEntityState("input_boolean.alarma");
    if (l == "on")
      lv_obj_add_state(light_switch, LV_STATE_CHECKED);
    else
      lv_obj_clear_state(light_switch, LV_STATE_CHECKED);
  } else {
    lv_label_set_text(status_label, "Reconectando...");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xFF0000), 0);
    connectWiFi();
  }
}

void setup() {
  Serial.begin(115200);

  gfx->begin();
  gfx->setRotation(1);
  gfx->fillScreen(0x0000);

  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);

  // Initialize touch hardware
  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  Wire.setClock(TOUCH_I2C_CLOCK);
  pinMode(TOUCH_INT_PIN, INPUT_PULLUP);
  pinMode(TOUCH_RST_PIN, OUTPUT);
  digitalWrite(TOUCH_RST_PIN, LOW);
  delay(200);
  digitalWrite(TOUCH_RST_PIN, HIGH);
  delay(200);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 20);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  connectWiFi();
  lastHAUpdate = millis() - HA_UPDATE_INTERVAL; // Force immediate update
  createUI();
}

void loop() {
  lv_timer_handler();
  gfx->flush(); // Send canvas to display
  updateHA();
  delay(5);
}
