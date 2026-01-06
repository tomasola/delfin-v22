#include "localization.h"
#include "painlessMesh.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>
#include <PubSubClient.h>
#include <SPIFFS.h>
#include <WiFi.h>

#if HAS_SCREEN
#include "ui_manager.h"
#include <Arduino_GFX_Library.h>
#endif

// ========== CONFIGURACIÓN MESH ==========
#define MESH_PREFIX "Delfin_Mesh"
#define MESH_PASSWORD "delfin123"
#define MESH_PORT 5555

// ========== CONFIGURACIÓN MQTT ==========
#define MQTT_SERVER "192.168.1.XX"
#define MQTT_PORT 1883
WiFiClient espClient;
PubSubClient mqttClient(espClient);

painlessMesh mesh;
Scheduler userScheduler;

// Prototypes
void saveDevices();
void loadDevices();
void addDevice(const char *name, const char *mac);

// ========== CONFIGURACIÓN PANTALLA (Sunton S3) ==========
#if HAS_SCREEN
#define GFX_BL 1
Arduino_ESP32QSPI *bus = new Arduino_ESP32QSPI(45, 47, 21, 48, 40, 39);
Arduino_AXS15231B *g = new Arduino_AXS15231B(
    bus, -1 /* RST */, 0 /* Rotation */, true /* IPS */, 480, 320);
Arduino_Canvas *gfx = new Arduino_Canvas(480, 320, g, 0, 0, 0);
UIManager ui(gfx);
#endif

// ========== ESTRUCTURAS DE DATOS ==========
struct TrackedDevice {
  char name[32];
  char mac[18];
  float lastRSSI;
  float distance;
  uint32_t lastSeen;
  KalmanFilter *filter;
};

#define MAX_TRACKED 5
TrackedDevice trackedDevices[MAX_TRACKED];
int deviceCount = 0;

// ========== VARIABLES DE ESTADO ==========
int currentPage = 0; // 0: Mapa, 1: Dispositivos, 2: Config
uint16_t touchX, touchY;
bool lastTouched = false;

// ========== TAREAS (CORE 0: BLE, CORE 1: MESH/UI) ==========
TaskHandle_t BLETask;

void scanBLE(void *parameter) {
  BLEDevice::init("");
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  for (;;) {
    BLEScanResults *results = pBLEScan->start(2, false);
    for (int i = 0; i < results->getCount(); i++) {
      BLEAdvertisedDevice device = results->getDevice(i);
      String mac =
          device.getAddress().toString().c_str(); // Fix: Added .c_str()
      mac.toUpperCase();

      for (int j = 0; j < deviceCount; j++) {
        if (strcmp(trackedDevices[j].mac, mac.c_str()) == 0) {
          float rawRSSI = (float)device.getRSSI();
          trackedDevices[j].lastRSSI =
              trackedDevices[j].filter->update(rawRSSI);
          trackedDevices[j].distance = rssiToMeters(trackedDevices[j].lastRSSI);
          trackedDevices[j].lastSeen = millis();

          // Broadcast a la malla
          StaticJsonDocument<200> doc;
          doc["type"] = "rssi";
          doc["mac"] = mac;
          doc["val"] = trackedDevices[j].lastRSSI;
          String msg;
          serializeJson(doc, msg);
          mesh.sendBroadcast(msg);
        }
      }
    }
    pBLEScan->clearResults();
    delay(10);
  }
}

void receivedCallback(uint32_t from, String &msg) {
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, msg);
  if (error)
    return;

  Serial.printf("Mesh received from %u: %s\n", from, msg.c_str());

  const char *type = doc["type"];
  if (strcmp(type, "rssi") == 0) {
    // Aquí se procesarían los RSSI de otros nodos para trilateración
  } else if (strcmp(type, "sync_device") == 0) {
    const char *name = doc["name"];
    const char *mac = doc["mac"];

    // Evitar duplicados
    bool exists = false;
    for (int i = 0; i < deviceCount; i++) {
      if (strcmp(trackedDevices[i].mac, mac) == 0)
        exists = true;
    }

    if (!exists && deviceCount < MAX_TRACKED) {
      strncpy(trackedDevices[deviceCount].name, name, 31);
      strncpy(trackedDevices[deviceCount].mac, mac, 17);
      trackedDevices[deviceCount].filter = new KalmanFilter(0.1, 10, 1, -70);
      deviceCount++;
      saveDevices();
      Serial.printf("Synced new device: %s (%s)\n", name, mac);
    }
  } else if (strcmp(type, "sync_list") == 0) {
    JsonArray devices = doc["devices"];
    for (JsonObject d : devices) {
      const char *name = d["name"];
      const char *mac = d["mac"];

      bool exists = false;
      for (int i = 0; i < deviceCount; i++) {
        if (strcmp(trackedDevices[i].mac, mac) == 0)
          exists = true;
      }

      if (!exists && deviceCount < MAX_TRACKED) {
        strncpy(trackedDevices[deviceCount].name, name, 31);
        strncpy(trackedDevices[deviceCount].mac, mac, 17);
        trackedDevices[deviceCount].filter = new KalmanFilter(0.1, 10, 1, -70);
        deviceCount++;
        Serial.printf("Synced device from list: %s (%s)\n", name, mac);
      }
    }
    saveDevices();
  }
}

// ========== GESTIÓN DE DISPOSITIVOS ==========
void saveDevices() {
  File file = SPIFFS.open("/devices.json", FILE_WRITE);
  if (!file)
    return;
  StaticJsonDocument<1024> doc;
  JsonArray array = doc.to<JsonArray>();
  for (int i = 0; i < deviceCount; i++) {
    JsonObject obj = array.createNestedObject();
    obj["name"] = trackedDevices[i].name;
    obj["mac"] = trackedDevices[i].mac;
  }
  serializeJson(doc, file);
  file.close();
}

void loadDevices() {
  if (!SPIFFS.exists("/devices.json"))
    return;
  File file = SPIFFS.open("/devices.json", FILE_READ);
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    return;
  JsonArray array = doc.as<JsonArray>();
  deviceCount = 0;
  for (JsonObject obj : array) {
    if (deviceCount < MAX_TRACKED) {
      strncpy(trackedDevices[deviceCount].name, obj["name"], 31);
      strncpy(trackedDevices[deviceCount].mac, obj["mac"], 17);
      trackedDevices[deviceCount].filter = new KalmanFilter(0.1, 10, 1, -70);
      deviceCount++;
    }
  }
  file.close();
}

void addDevice(const char *name, const char *mac) {
  if (deviceCount < MAX_TRACKED) {
    strncpy(trackedDevices[deviceCount].name, name, 31);
    strncpy(trackedDevices[deviceCount].mac, mac, 17);
    trackedDevices[deviceCount].filter = new KalmanFilter(0.1, 10, 1, -70);
    deviceCount++;
    saveDevices();

    StaticJsonDocument<256> doc;
    doc["type"] = "sync_device";
    doc["name"] = name;
    doc["mac"] = mac;
    String msg;
    serializeJson(doc, msg);
    mesh.sendBroadcast(msg);
  }
}

// ========== TOUCH HANDLING ==========
bool getTouchPoint(uint16_t &x, uint16_t &y) {
#if HAS_SCREEN
  uint8_t data[8] = {0};
  const uint8_t read_cmd[11] = {0xb5, 0xab, 0xa5, 0x5a, 0x00, 0x00,
                                0x00, 0x08, 0x00, 0x00, 0x00};
  Wire.beginTransmission(0x3B);
  Wire.write(read_cmd, 11);
  if (Wire.endTransmission() != 0)
    return false;
  if (Wire.requestFrom(0x3B, 8) != 8)
    return false;
  for (int i = 0; i < 8; i++)
    data[i] = Wire.read();
  if (data[1] > 0) {
    uint16_t rawX = ((data[2] & 0x0F) << 8) | data[3];
    uint16_t rawY = ((data[4] & 0x0F) << 8) | data[5];
    y = map(rawX, 0, 320, 320, 0);
    x = rawY;
    return true;
  }
#endif
  return false;
}

void setup() {
  Serial.begin(115200);
#if HAS_SCREEN
  Wire.begin(4, 8);
#endif

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
  } else {
    loadDevices();
  }

#if HAS_SCREEN
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
  delay(100);
  gfx->begin();
  gfx->setRotation(0);
#endif

  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection([](uint32_t nodeId) {
    Serial.printf("New Mesh Connection, nodeId = %u\n", nodeId);
  });
  mesh.onDroppedConnection([](uint32_t nodeId) {
    Serial.printf("Dropped Mesh Connection, nodeId = %u\n", nodeId);
  });

  xTaskCreatePinnedToCore(scanBLE, "BLETask", 10000, NULL, 1, &BLETask, 0);
}

void loop() {
  mesh.update();

  // MQTT Keepalive & Publish
  if (!mqttClient.connected()) {
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.connect("DelfinMeshNode");
  }
  mqttClient.loop();

  static uint32_t lastMQTTPub = 0;
  if (millis() - lastMQTTPub > 5000) {
    for (int i = 0; i < deviceCount; i++) {
      char topic[64];
      snprintf(topic, 64, "delfin/ips/%s/distance", trackedDevices[i].name);
      char val[16];
      dtostrf(trackedDevices[i].distance, 1, 2, val);
      mqttClient.publish(topic, val);
    }
    lastMQTTPub = millis();
  }

  bool touched = getTouchPoint(touchX, touchY);
#if HAS_SCREEN
  if (touched && !lastTouched) {
    // Navegación básica por botones del footer
    if (touchY > 290) {
      if (touchX < 160)
        currentPage = 0;
      else if (touchX < 320)
        currentPage = 1;
      else
        currentPage = 2;
    }
  }
#endif
  lastTouched = touched;

#if HAS_SCREEN
  // Renderizado
  gfx->fillScreen(C_BG);

  if (currentPage == 0) {
    ui.drawHeader("MAPA DE LOCALIZACIÓN");
    ui.drawMap(460, 230);
    // Dibujar dispositivos tracked
    for (int i = 0; i < deviceCount; i++) {
      // Lógica de trilateración para poner el punto
      ui.drawUser(240, 160, trackedDevices[i].name); // Demo center
    }
  } else if (currentPage == 1) {
    ui.drawHeader("DISPOSITIVOS");
    gfx->setCursor(20, 60);
    for (int i = 0; i < deviceCount; i++) {
      gfx->printf("%s: %.2fm\n", trackedDevices[i].name,
                  trackedDevices[i].distance);
    }
  } else {
    ui.drawHeader("CONFIGURACIÓN MESH");
    gfx->setCursor(20, 60);
    gfx->printf("Nodo ID: %u\n", mesh.getNodeId());
    gfx->printf("Nodos activos: %u\n", mesh.getNodeList().size());
  }

  ui.drawFooter("MAPA          DEVICES          CONFIG");
  gfx->flush();
#endif
  delay(30);
}
