# Sunton HA Panel - Arduino/PlatformIO

Panel de control para Home Assistant usando pantalla táctil Sunton ESP32-S3 3.5" ST7796.

## Características
- ✅ Pantalla ST7796 320x480 (8-bit parallel)
- ✅ Táctil capacitivo GT911
- ✅ 6 botones personalizables
- ✅ Integración con Home Assistant vía REST API
- ✅ WiFi

## Configuración Inicial

### 1. Instalar PlatformIO
Si usas VS Code, instala la extensión PlatformIO IDE.

### 2. Configurar Home Assistant

Edita `src/main.cpp` y cambia:

```cpp
const char* ha_url = "http://192.168.1.XXX:8123";  // Tu IP de Home Assistant
const char* ha_token = "TU_TOKEN_AQUI";  // Tu token de acceso
```

**Para obtener el token:**
1. Ve a tu perfil en Home Assistant
2. Scroll hasta "Tokens de acceso de larga duración"
3. Crea un nuevo token
4. Cópialo en el código

### 3. Personalizar Botones

En `src/main.cpp`, modifica el array `buttons[]`:

```cpp
Button buttons[] = {
    {x, y, ancho, alto, "Texto", "entity_id", "servicio", COLOR},
    // Ejemplo:
    {10, 40, 140, 80, "Luz Cocina", "light.cocina", "toggle", TFT_BLUE},
};
```

**Servicios comunes:**
- `toggle` - Alternar encendido/apagado
- `turn_on` - Encender
- `turn_off` - Apagar

### 4. Compilar y Subir

```bash
# Desde la carpeta del proyecto
pio run -t upload
```

O usa el botón "Upload" en PlatformIO IDE.

## Estructura del Proyecto

```
sunton_ha_panel/
├── platformio.ini      # Configuración de PlatformIO
├── src/
│   └── main.cpp        # Código principal
└── README.md           # Este archivo
```

## Pines Utilizados

| Función | Pin GPIO |
|---------|----------|
| Display CS | 10 |
| Display DC | 11 |
| Display WR | 12 |
| Display RD | 13 |
| Display RST | 4 |
| Display BL | 45 |
| Display D0-D7 | 9,46,3,8,18,17,16,15 |
| Touch SDA | 33 |
| Touch SCL | 32 |
| Touch INT | 21 |
| Touch RST | 25 |

## Solución de Problemas

### La pantalla no enciende
- Verifica las conexiones de alimentación
- Comprueba que el pin de backlight (45) esté configurado

### No se conecta a WiFi
- Verifica SSID y contraseña en `main.cpp`
- Comprueba que tu red sea 2.4GHz (ESP32 no soporta 5GHz)

### Los botones no funcionan
- Verifica que el token de HA sea correcto
- Comprueba que los `entity_id` existan en Home Assistant
- Revisa el monitor serial para ver errores HTTP

## Próximas Mejoras
- [ ] Implementar lectura táctil GT911 completa
- [ ] Añadir feedback visual al presionar botones
- [ ] Mostrar estados de entidades en tiempo real
- [ ] Añadir más páginas de control
