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
  UIManager(Arduino_Canvas *canvas) : _gfx(canvas) {}

  void drawHeader(const char *title) {
    _gfx->fillRect(0, 0, 480, 40, C_HEADER);
    _gfx->setTextColor(C_TEXT);
    _gfx->setTextSize(2);
    _gfx->setCursor(15, 10);
    _gfx->print(title);
  }

  void drawMap(int width, int height) {
    // Grid de fondo
    _gfx->drawRect(10, 50, 460, 230, C_GRAY);
    for (int x = 10; x <= 470; x += 50)
      _gfx->drawFastVLine(x, 50, 230, 0x1082); // Gris oscuro
    for (int y = 50; y <= 280; y += 50)
      _gfx->drawFastHLine(10, y, 460, 0x1082);
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
    _gfx->fillRect(0, 290, 480, 30, C_GRAY);
    _gfx->setCursor(10, 300);
    _gfx->setTextSize(1);
    _gfx->print(info);
  }

private:
  Arduino_Canvas *_gfx;
};

#endif
