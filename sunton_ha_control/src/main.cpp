#include "config.h"
#include "secrets.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <Wire.h>
#include <lvgl.h>

// Pins Sunton 3.5"
#define GFX_BL 1
#define TOUCH_ADDR 0x3B
#define TOUCH_SDA 4
#define TOUCH_SCL 8
#define TOUCH_I2C_CLOCK 400000
#define TOUCH_RST_PIN 12
#define AXS_MAX_TOUCH_NUMBER 1

// GFX Setup
Arduino_ESP32QSPI *bus = new Arduino_ESP32QSPI(45, 47, 21, 48, 40, 39);
Arduino_AXS15231B *g =
    new Arduino_AXS15231B(bus, GFX_NOT_DEFINED, 0, false, 320, 480);
Arduino_Canvas *gfx = new Arduino_Canvas(320, 480, g, 0, 0, 0);

static const uint32_t screenWidth = 480;
static const uint32_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 30];

WiFiClient client;
HTTPClient http;
unsigned long lastUpdate = 0;

struct Zone {
  String name;
  const char *lights[2];
  uint8_t numLights;
  const char *led;
  const char *covers[2];
  uint8_t numCovers;
  const char *tempSensor;
  const char *humSensor;
};

Zone zones[] = {{NAME_HABITACION1,
                 {LIGHT_HAB1_LUZ1, LIGHT_HAB1_LUZ2},
                 2,
                 LED_HAB1,
                 {COVER_HABITACION1, ""},
                 1,
                 SENSOR_TEMP_HABITACION1,
                 SENSOR_HUM_HABITACION1},
                {NAME_HABITACION2,
                 {LIGHT_HAB2_LUZ1, LIGHT_HAB2_LUZ2},
                 2,
                 LED_HAB2,
                 {COVER_HABITACION2, ""},
                 1,
                 SENSOR_TEMP_HABITACION2,
                 SENSOR_HUM_HABITACION2},
                {NAME_HABITACION3,
                 {LIGHT_HAB3_LUZ1, LIGHT_HAB3_LUZ2},
                 2,
                 LED_HAB3,
                 {COVER_HABITACION3, ""},
                 1,
                 SENSOR_TEMP_HABITACION3,
                 SENSOR_HUM_HABITACION3},
                {NAME_SALON,
                 {LIGHT_SALON_LUZ1, LIGHT_SALON_LUZ2},
                 2,
                 LED_SALON,
                 {COVER_SALON_1, COVER_SALON_2},
                 2,
                 SENSOR_TEMP_SALON,
                 SENSOR_HUM_SALON},
                {NAME_PASILLO,
                 {LIGHT_PASILLO_LUZ1, LIGHT_PASILLO_LUZ2},
                 2,
                 LED_PASILLO,
                 {"", ""},
                 0,
                 "",
                 ""}};

const uint8_t NUM_ZONES = 5;
lv_obj_t *tabview;
lv_obj_t *statusLabel;
lv_obj_t *zoneTempLabels[5];
lv_obj_t *zoneHumLabels[5];

static lv_style_t style_screen;
static lv_style_t style_card;
static lv_style_t style_title;
static lv_style_t style_value;
static lv_style_t style_navbar;
static lv_style_t style_btn_nav;
static lv_style_t style_btn_scene;

void initPremiumStyles() {
  lv_style_init(&style_screen);
  lv_style_set_bg_color(&style_screen, lv_color_hex(0x0A0B10));

  lv_style_init(&style_card);
  lv_style_set_bg_color(&style_card, lv_color_hex(0x161922));
  lv_style_set_border_width(&style_card, 1);
  lv_style_set_border_color(&style_card, lv_color_hex(0x232732));
  lv_style_set_radius(&style_card, 12);
  lv_style_set_pad_all(&style_card, 10);

  lv_style_init(&style_title);
  lv_style_set_text_font(&style_title, &lv_font_montserrat_14);
  lv_style_set_text_color(&style_title, lv_color_hex(0x8C92AC));

  lv_style_init(&style_value);
  lv_style_set_text_font(&style_value, &lv_font_montserrat_20);
  lv_style_set_text_color(&style_value, lv_color_hex(0xFFFFFF));

  lv_style_init(&style_navbar);
  lv_style_set_bg_color(&style_navbar, lv_color_hex(0x11131A));
  lv_style_set_border_width(&style_navbar, 0);

  lv_style_init(&style_btn_nav);
  lv_style_set_radius(&style_btn_nav, 8);
  lv_style_set_bg_color(&style_btn_nav, lv_color_hex(0x1E222D));

  lv_style_init(&style_btn_scene);
  lv_style_set_radius(&style_btn_scene, 10);
  lv_style_set_bg_color(&style_btn_scene, lv_color_hex(0x3D5AFE));
}

// HA Comms
String getEntityState(const char *entity_id) {
  if (WiFi.status() != WL_CONNECTED || strlen(entity_id) == 0)
    return "err";
  String url = String(HA_URL) + "/api/states/" + String(entity_id);
  if (http.begin(client, url)) {
    http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));
    int code = http.GET();
    if (code == 200) {
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, http.getString());
      http.end();
      return doc["state"].as<String>();
    }
    http.end();
  }
  return "err";
}

void callService(const char *domain, const char *service,
                 const char *entity_id) {
  if (WiFi.status() != WL_CONNECTED || strlen(entity_id) == 0)
    return;
  String url = String(HA_URL) + "/api/services/" + String(domain) + "/" +
               String(service);
  if (http.begin(client, url)) {
    http.addHeader("Authorization", "Bearer " + String(HA_TOKEN));
    http.addHeader("Content-Type", "application/json");
    http.POST("{\"entity_id\":\"" + String(entity_id) + "\"}");
    http.end();
  }
}

// GFX Flush - ARREGLO NITIDEZ
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  // Revertimos a draw16bitRGBBitmap para máxima nitidez (validado en test
  // previo)
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  lv_disp_flush_ready(disp);
}

// Touch Handling
bool getTouchPoint(uint16_t &x, uint16_t &y) {
  uint8_t data[8] = {0};
  const uint8_t cmd[11] = {0xb5, 0xab, 0xa5, 0x5a, 0x00, 0x00,
                           0x00, 0x08, 0x00, 0x00, 0x00};
  Wire.beginTransmission(TOUCH_ADDR);
  Wire.write(cmd, 11);
  if (Wire.endTransmission() != 0)
    return false;
  if (Wire.requestFrom((uint16_t)TOUCH_ADDR, (uint8_t)8) != 8)
    return false;
  for (int i = 0; i < 8; i++)
    data[i] = Wire.read();
  if (data[1] > 0 && data[1] <= 10) {
    uint16_t rx = ((data[2] & 0x0F) << 8) | data[3];
    uint16_t ry = ((data[4] & 0x0F) << 8) | data[5];
    if (rx > 320 || ry > 480)
      return false;
    y = map(rx, 0, 320, 320, 0);
    x = ry;
    return true;
  }
  return false;
}

void my_touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  uint16_t tx, ty;
  if (getTouchPoint(tx, ty)) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = tx;
    data->point.y = ty;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

// UI Callbacks
void nav_event(lv_event_t *e) {
  uint8_t idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
  lv_tabview_set_act(tabview, idx, LV_ANIM_OFF);
}

struct LEvent {
  uint8_t z;
  uint8_t l;
};
static LEvent levs[5][2];

void light_event(lv_event_t *e) {
  LEvent *d = (LEvent *)lv_event_get_user_data(e);
  if (lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED))
    callService("light", "turn_on", zones[d->z].lights[d->l]);
  else
    callService("light", "turn_off", zones[d->z].lights[d->l]);
}

void scene_event(lv_event_t *e) {
  const char *sid = (const char *)lv_event_get_user_data(e);
  callService("scene", "turn_on", sid);
}

// UI Builder
void createHomePage() {
  lv_obj_t *t = lv_tabview_add_tab(tabview, "Home");
  lv_obj_set_flex_flow(t, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(t, 15, 0);
  lv_obj_set_style_pad_gap(t, 15, 0);

  lv_obj_t *sb = lv_obj_create(t);
  lv_obj_set_size(sb, 450, 60);
  lv_obj_add_style(sb, &style_card, 0);
  statusLabel = lv_label_create(sb);
  lv_label_set_text(statusLabel, "Sistema Iniciando...");
  lv_obj_add_style(statusLabel, &style_title, 0);
  lv_obj_center(statusLabel);

  lv_obj_t *sg = lv_obj_create(t);
  lv_obj_set_size(sg, 450, 140);
  lv_obj_set_style_bg_opa(sg, 0, 0);
  lv_obj_set_style_border_width(sg, 0, 0);
  lv_obj_set_flex_flow(sg, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(sg, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_gap(sg, 12, 0);

  // ELIMINADOS EMOJIS (Cauasaban cuadrados)
  const char *sn[] = {"CINE", "NOCHE", "DIA", "SALIR"};
  const char *si[] = {SCENE_CINE, SCENE_DORMIR, SCENE_BUENOS_DIAS,
                      SCENE_SALIR_CASA};
  for (int i = 0; i < 4; i++) {
    lv_obj_t *b = lv_btn_create(sg);
    lv_obj_set_size(b, 130, 45);
    lv_obj_add_style(b, &style_btn_scene, 0);
    if (i == 1)
      lv_obj_set_style_bg_color(b, lv_color_hex(0x311B92), 0);
    if (i == 2)
      lv_obj_set_style_bg_color(b, lv_color_hex(0xFFB300), 0);
    if (i == 3)
      lv_obj_set_style_bg_color(b, lv_color_hex(0x43A047), 0);
    lv_obj_add_event_cb(b, scene_event, LV_EVENT_CLICKED, (void *)si[i]);
    lv_obj_t *l = lv_label_create(b);
    lv_label_set_text(l, sn[i]);
    lv_obj_center(l);
  }
}

void createZonePage(uint8_t idx) {
  Zone *z = &zones[idx];
  lv_obj_t *t = lv_tabview_add_tab(tabview, z->name.c_str());
  lv_obj_set_style_pad_all(t, 12, 0);
  lv_obj_set_flex_flow(t, LV_FLEX_FLOW_COLUMN);

  lv_obj_t *c = lv_obj_create(t);
  lv_obj_set_size(c, 456, 170);
  lv_obj_add_style(c, &style_card, 0);

  lv_obj_t *sl = lv_obj_create(c);
  lv_obj_set_size(sl, 180, 130);
  lv_obj_set_style_bg_opa(sl, 0, 0);
  lv_obj_set_style_border_width(sl, 0, 0);
  lv_obj_set_flex_flow(sl, LV_FLEX_FLOW_COLUMN);

  if (strlen(z->tempSensor) > 0) {
    zoneTempLabels[idx] = lv_label_create(sl);
    lv_label_set_text(zoneTempLabels[idx], "--C");
    lv_obj_add_style(zoneTempLabels[idx], &style_value, 0);
    zoneHumLabels[idx] = lv_label_create(sl);
    lv_label_set_text(zoneHumLabels[idx], "Hum: --%");
    lv_obj_add_style(zoneHumLabels[idx], &style_title, 0);
  }

  lv_obj_t *cl = lv_obj_create(c);
  lv_obj_set_size(cl, 230, 140);
  lv_obj_align(cl, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_set_style_bg_opa(cl, 0, 0);
  lv_obj_set_style_border_width(cl, 0, 0);
  lv_obj_set_flex_flow(cl, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_gap(cl, 10, 0);

  for (int i = 0; i < z->numLights; i++) {
    lv_obj_t *row = lv_obj_create(cl);
    lv_obj_set_size(row, 220, 38);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x1F222D), 0);
    lv_obj_set_style_radius(row, 8, 0);
    lv_obj_set_style_border_width(row, 0, 0);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text_fmt(lbl, "Luz %d", i + 1);
    lv_obj_add_style(lbl, &style_title, 0);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 10, 0);

    lv_obj_t *sw = lv_switch_create(row);
    lv_obj_set_size(sw, 45, 23);
    lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -10, 0);
    levs[idx][i] = {(uint8_t)idx, (uint8_t)i};
    lv_obj_add_event_cb(sw, light_event, LV_EVENT_VALUE_CHANGED, &levs[idx][i]);
  }
}

void createUI() {
  lv_obj_t *scr = lv_scr_act();
  lv_obj_add_style(scr, &style_screen, 0);

  tabview = lv_tabview_create(scr, LV_DIR_TOP, 0);
  lv_obj_set_size(tabview, 480, 260);
  lv_obj_set_pos(tabview, 0, 60);

  createHomePage();
  for (int i = 0; i < NUM_ZONES; i++)
    createZonePage(i);

  lv_obj_t *nv = lv_obj_create(scr);
  lv_obj_set_size(nv, 480, 60);
  lv_obj_add_style(nv, &style_navbar, 0);
  lv_obj_set_flex_flow(nv, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(nv, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_gap(nv, 6, 0);

  const char *bn[] = {"HOME", "H1", "H2", "H3", "SALON"};
  for (int i = 0; i < 5; i++) {
    lv_obj_t *b = lv_btn_create(nv);
    lv_obj_set_size(b, 85, 48);
    lv_obj_add_style(b, &style_btn_nav, 0);
    lv_obj_add_event_cb(b, nav_event, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
    lv_obj_t *l = lv_label_create(b);
    lv_label_set_text(l, bn[i]);
    lv_obj_center(l);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(">>> V3.1 SHARP & FIX FONTS <<<");

  if (!gfx->begin())
    Serial.println("Gfx FAIL");
  gfx->setRotation(1);
  gfx->fillScreen(0x0000);
  gfx->flush();

  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);

  pinMode(TOUCH_RST_PIN, OUTPUT);
  digitalWrite(TOUCH_RST_PIN, LOW);
  delay(100);
  digitalWrite(TOUCH_RST_PIN, HIGH);
  delay(100);

  Wire.begin(TOUCH_SDA, TOUCH_SCL);

  lv_init();
  initPremiumStyles();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 30);

  static lv_disp_drv_t d_drv;
  lv_disp_drv_init(&d_drv);
  d_drv.hor_res = 480;
  d_drv.ver_res = 320;
  d_drv.flush_cb = my_disp_flush;
  d_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&d_drv);

  static lv_indev_drv_t i_drv;
  lv_indev_drv_init(&i_drv);
  i_drv.type = LV_INDEV_TYPE_POINTER;
  i_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&i_drv);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  createUI();
}

void loop() {
  lv_timer_handler();
  gfx->flush();
  if (millis() - lastUpdate > 5000) {
    lastUpdate = millis();
    if (WiFi.status() == WL_CONNECTED) {
      lv_label_set_text(statusLabel, "Sistema Online");
      for (int i = 0; i < NUM_ZONES; i++) {
        if (strlen(zones[i].tempSensor) > 0) {
          String s = getEntityState(zones[i].tempSensor);
          if (s != "err")
            lv_label_set_text_fmt(zoneTempLabels[i], "%sC", s.c_str());
          String hum = getEntityState(zones[i].humSensor);
          if (hum != "err")
            lv_label_set_text_fmt(zoneHumLabels[i], "Hum: %s%%", hum.c_str());
        }
      }
    }
  }
}
