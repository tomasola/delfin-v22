#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino_GFX_Library.h>

// Colores Modernos
#define C_BG 0x0000
#define C_HEADER 0x10A2 // Azul oscuro premium
#define C_TEXT 0xFFFF
#define C_GRAY 0x2104
#define C_ACCENT 0x07E0  // Verde lima
#define C_PRIMARY 0x01DF // Cian
#define C_DANGER 0xF800

class UIManager {
public:
  // Canvas reducido para evitar crash por falta de memoria
  // 240x320 en lugar de 320x480 = 150KB en lugar de 300KB
  UIManager(Arduino_Canvas *canvas) : _gfx(canvas) {}

  void drawHeader(const char *title) {
    _gfx->fillRect(0, 0, 240, 30, C_HEADER); // Adjusted width to 240
    _gfx->setTextColor(C_TEXT);
    _gfx->setTextSize(2);
    _gfx->setCursor(10, 8);
    _gfx->print(title);
  }

  void drawMap(int width, int height) {
    // Grid de fondo ajustado a 240x320
    _gfx->drawRect(5, 35, 230, 200, C_GRAY);
    for (int x = 5; x <= 235; x += 40)
      _gfx->drawFastVLine(x, 35, 200, 0x1082);
    for (int y = 35; y <= 235; y += 40)
      _gfx->drawFastHLine(5, y, 230, 0x1082);
  }

  void drawNode(int x, int y, const char *label, bool active) {
    _gfx->fillRoundRect(x - 20, y - 10, 40, 20, 4,
                        active ? C_ACCENT : C_DANGER);
    _gfx->setTextColor(C_BG);
    _gfx->setTextSize(1);
    _gfx->setCursor(x - 15, y - 4);
    _gfx->print(label);
  }

  void drawUser(int x, int y, const char *name) {
    _gfx->fillCircle(x, y, 6, C_PRIMARY);
    _gfx->drawCircle(x, y, 10, C_PRIMARY);
    _gfx->setTextColor(C_TEXT);
    _gfx->setCursor(x + 12, y - 4);
    _gfx->print(name);
  }

  void drawFooter(const char *info) {
    _gfx->fillRect(0, 290, 320, 30, C_GRAY);
    _gfx->setCursor(5, 300);
    _gfx->setTextSize(1);
    _gfx->print(info);
  }

private:
  Arduino_Canvas *_gfx;
};

#endif
