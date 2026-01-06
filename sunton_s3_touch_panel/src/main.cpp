#include <Arduino.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>
#include <NimBLEDevice.h>
#include <SD.h>
#include <SPI.h>
#include <USB.h>
#include <USBHIDKeyboard.h>
#include <Wire.h>
#include <lvgl.h>

// BLE UUIDs
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define DATA_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define IMAGE_CHAR_UUID "ae5946d7-1501-443b-8772-c06d649d5c4b"

// BLE Globals
NimBLEServer *pServer = NULL;
NimBLECharacteristic *pDataChar = NULL;
NimBLECharacteristic *pImageChar = NULL;
bool deviceConnected = false;

// Image Buffer
uint8_t *imgBuffer = NULL;
size_t imgBufferSize = 0;
size_t imgLoadedSize = 0;

// Forward declarations
void printLabel();

class MyServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer) { deviceConnected = true; };
  void onDisconnect(NimBLEServer *pServer) { deviceConnected = false; }
};

class DataCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, value);
      if (!error) {
        const char *command = doc["command"];
        if (strcmp(command, "START_IMAGE") == 0) {
          imgBufferSize = doc["size"];
          if (imgBuffer)
            free(imgBuffer);
          imgBuffer = (uint8_t *)malloc(imgBufferSize);
          imgLoadedSize = 0;
          Serial.printf("Expecting image of size: %d\n", imgBufferSize);
        } else if (strcmp(command, "PRINT") == 0) {
          Serial.println("Print command received via BLE");
          printLabel();
        }
      }
    }
  }
};

#include <string.h>

class ImageCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (imgBuffer && imgLoadedSize + value.length() <= imgBufferSize) {
      memcpy(imgBuffer + imgLoadedSize, value.data(), value.length());
      imgLoadedSize += value.length();
      if (imgLoadedSize == imgBufferSize) {
        Serial.println("Image fully received!");
        // Display image on the panel
        // Clear screen first? Or draw on top.
        // The Sunton display is 480x320.
        gfx->fillScreen(0x0000); // Black
        gfx->drawJpg(imgBuffer, imgBufferSize, 0, 0, 0, 0);
        gfx->flush();
        Serial.println("Image displayed on panel");
      }
    }
  }
};

// Pins Sunton 3.5" (AXS15231B)
#define GFX_BL 1
#define TOUCH_ADDR 0x3B
#define TOUCH_SDA 4
#define TOUCH_SCL 8
#define TOUCH_I2C_CLOCK 400000
#define TOUCH_RST_PIN 12

// SD Card Pins
#define SD_SCK 12
#define SD_MISO 13
#define SD_MOSI 11
#define SD_CS 10

// HID Keys
#define KEY_RETURN 0xB0
#define KEY_ESC 0xB1
#define KEY_TAB 0xB3
#define KEY_PRTSC 0xCE

// Objects
USBHIDKeyboard Keyboard;
Arduino_ESP32QSPI *bus = new Arduino_ESP32QSPI(45, 47, 21, 48, 40, 39);
Arduino_AXS15231B *g =
    new Arduino_AXS15231B(bus, GFX_NOT_DEFINED, 0, false, 320, 480);
Arduino_Canvas *gfx = new Arduino_Canvas(320, 480, g, 0, 0, 0);

static const uint32_t screenWidth = 480;
static const uint32_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 30];

// Global State
bool sdReady = false;
bool keyboardReady = false;
lv_obj_t *statusLabel;

// GFX Flush
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
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

// SD Macro Parser (Duckyscript Lite)
void processSDCommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0 || cmd.startsWith("//"))
    return;

  if (cmd.startsWith("DELAY ")) {
    delay(cmd.substring(6).toInt());
  } else if (cmd.startsWith("STRING ")) {
    Keyboard.print(cmd.substring(7));
  } else if (cmd == "ENTER") {
    Keyboard.press(KEY_RETURN);
    Keyboard.releaseAll();
  } else if (cmd.startsWith("GUI ") || cmd.startsWith("WINDOWS ")) {
    char k = cmd.substring(cmd.indexOf(' ') + 1).charAt(0);
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press(k);
    Keyboard.releaseAll();
  } else if (cmd.startsWith("ALT ")) {
    char k = cmd.substring(4).charAt(0);
    Keyboard.press(KEY_LEFT_ALT);
    Keyboard.press(k);
    Keyboard.releaseAll();
  } else if (cmd == "TAB") {
    Keyboard.press(KEY_TAB);
    Keyboard.releaseAll();
  }
}

void executeSDPayload(const char *path) {
  if (!sdReady)
    return;
  File f = SD.open(path);
  if (!f)
    return;
  while (f.available()) {
    processSDCommand(f.readStringUntil('\n'));
  }
  f.close();
}

// Shortcut Actions
void printLabel() {
  Keyboard.press(KEY_LEFT_ALT);
  Keyboard.press('p');
  Keyboard.releaseAll();
}

void openCMD() {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  Keyboard.releaseAll();
  delay(400);
  Keyboard.print("cmd");
  delay(100);
  Keyboard.press(KEY_RETURN);
  Keyboard.releaseAll();
}

void openPowerShell() {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  Keyboard.releaseAll();
  delay(400);
  Keyboard.print("powershell");
  delay(100);
  Keyboard.press(KEY_RETURN);
  Keyboard.releaseAll();
}

void openNotepad() {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  Keyboard.releaseAll();
  delay(400);
  Keyboard.print("notepad");
  delay(100);
  Keyboard.press(KEY_RETURN);
  Keyboard.releaseAll();
}

void lockPC() {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('l');
  Keyboard.releaseAll();
}

void openTaskManager() {
  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(KEY_LEFT_SHIFT);
  Keyboard.press(KEY_ESC);
  Keyboard.releaseAll();
}

// UI Event
static void btn_event_cb(lv_event_t *e) {
  uintptr_t type = (uintptr_t)lv_event_get_user_data(e);
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    String msg = "Executing...";
    switch (type) {
    case 0:
      msg = "Printing Label";
      printLabel();
      break;
    case 1:
      msg = "CMD";
      openCMD();
      break;
    case 2:
      msg = "PowerShell";
      openPowerShell();
      break;
    case 3:
      msg = "Notepad";
      openNotepad();
      break;
    case 4:
      msg = "Task Mgr";
      openTaskManager();
      break;
    case 5:
      msg = "Locking PC";
      lockPC();
      break;
    case 6:
      msg = "Custom 1";
      executeSDPayload("/payloads/custom1.txt");
      break;
    case 7:
      msg = "Custom 2";
      executeSDPayload("/payloads/custom2.txt");
      break;
    case 8:
      msg = "Win+R";
      Keyboard.press(KEY_LEFT_GUI);
      Keyboard.press('r');
      Keyboard.releaseAll();
      break;
    case 9:
      msg = "Screenshot";
      Keyboard.press(KEY_LEFT_GUI);
      Keyboard.press(KEY_PRTSC);
      Keyboard.releaseAll();
      break;
    case 10:
      msg = "Browser";
      Keyboard.press(KEY_LEFT_GUI);
      Keyboard.press('r');
      Keyboard.releaseAll();
      delay(400);
      Keyboard.print("https://google.com");
      delay(100);
      Keyboard.press(KEY_RETURN);
      Keyboard.releaseAll();
      break;
    case 11:
      msg = "VS Code";
      Keyboard.press(KEY_LEFT_GUI);
      Keyboard.print("code");
      delay(400);
      Keyboard.press(KEY_RETURN);
      Keyboard.releaseAll();
      break;
    }
    lv_label_set_text(statusLabel, msg.c_str());
    delay(500);
    lv_label_set_text(statusLabel, "Ready");
  }
}

void createMacroUI() {
  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x0A0B10), 0);

  lv_obj_t *header = lv_obj_create(scr);
  lv_obj_set_size(header, 480, 45);
  lv_obj_set_style_bg_color(header, lv_color_hex(0x161922), 0);
  lv_obj_set_style_border_width(header, 0, 0);
  lv_obj_t *title = lv_label_create(header);
  lv_label_set_text(title, "DELFIN MACRO PANEL");
  lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_center(title);

  statusLabel = lv_label_create(scr);
  lv_obj_align(statusLabel, LV_ALIGN_BOTTOM_MID, 0, -5);
  lv_label_set_text(statusLabel, "Initializing...");
  lv_obj_set_style_text_color(statusLabel, lv_color_hex(0x8C92AC), 0);

  lv_obj_t *cont = lv_obj_create(scr);
  lv_obj_set_size(cont, 470, 240);
  lv_obj_align(cont, LV_ALIGN_CENTER, 0, 10);
  lv_obj_set_style_bg_opa(cont, 0, 0);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_gap(cont, 10, 0);

  const char *labels[] = {"PRINT LABEL", "CMD",      "PSHELL",   "NOTEPAD",
                          "TASK MGR",    "LOCK PC",  "CUSTOM 1", "CUSTOM 2",
                          "RUN DIALOG",  "SNAPSHOT", "BROWSER",  "VS CODE"};
  uint32_t colors[] = {0x43A047, 0x1E88E5, 0x3949AB, 0x7CB342,
                       0x00ACC1, 0xD81B60, 0xFDD835, 0xFFB300,
                       0x8E24AA, 0x546E7A, 0xFB8C00, 0x3D5AFE};

  for (int i = 0; i < 12; i++) {
    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 105, 65);
    lv_obj_set_style_bg_color(btn, lv_color_hex(colors[i]), 0);
    lv_obj_set_style_radius(btn, 8, 0);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED,
                        (void *)(uintptr_t)i);

    lv_obj_t *l = lv_label_create(btn);
    lv_label_set_text(l, labels[i]);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_12, 0);
    lv_obj_center(l);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

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

  Keyboard.begin();
  USB.begin();
  keyboardReady = true;

  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (SD.begin(SD_CS)) {
    sdReady = true;
    if (!SD.exists("/payloads"))
      SD.mkdir("/payloads");
  }

  lv_init();
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

  createMacroUI();
  lv_label_set_text(statusLabel, sdReady ? "Ready (SD OK)" : "Ready (No SD)");

  // --- BLE Initialization ---
  NimBLEDevice::init("DelfinPanel");
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  NimBLEService *pService = pServer->createService(SERVICE_UUID);

  pDataChar = pService->createCharacteristic(
      DATA_CHAR_UUID,
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  pDataChar->setCallbacks(new DataCallbacks());

  pImageChar = pService->createCharacteristic(
      IMAGE_CHAR_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  pImageChar->setCallbacks(new ImageCallbacks());

  pService->start();

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
  Serial.println("BLE Server Started as 'DelfinPanel'");
}

void loop() {
  lv_timer_handler();
  gfx->flush();
  delay(5);
}
