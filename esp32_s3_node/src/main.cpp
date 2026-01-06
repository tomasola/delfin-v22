#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#define GFX_BL 1

Arduino_ESP32QSPI *bus = new Arduino_ESP32QSPI(45, 47, 21, 48, 40, 39);
Arduino_AXS15231B *display =
    new Arduino_AXS15231B(bus, GFX_NOT_DEFINED, 0, true, 320, 480);

void setup() {
  Serial.begin(115200);
  Serial.println("Sunton Display Test");

  // Backlight
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);

  // Initialize display
  display->begin();
  display->setRotation(1); // Landscape

  Serial.println("Display initialized!");
}

int colorIndex = 0;
uint16_t colors[] = {0xF800, 0x07E0, 0x001F, 0xFFE0, 0xF81F, 0x07FF, 0xFFFF};
String colorNames[] = {"RED",     "GREEN", "BLUE", "YELLOW",
                       "MAGENTA", "CYAN",  "WHITE"};

void loop() {
  // Fill screen with color
  display->fillScreen(colors[colorIndex]);

  // Draw text
  display->setTextColor(0x0000); // Black text
  display->setTextSize(3);
  display->setCursor(50, 100);
  display->println("SUNTON DISPLAY");

  display->setTextSize(2);
  display->setCursor(50, 140);
  display->print("Color: ");
  display->println(colorNames[colorIndex]);

  display->setCursor(50, 170);
  display->print("Time: ");
  display->print(millis() / 1000);
  display->println(" s");

  Serial.printf("Showing %s (Time: %lu s)\n", colorNames[colorIndex].c_str(),
                millis() / 1000);

  // Next color
  colorIndex = (colorIndex + 1) % 7;

  delay(2000);
}
