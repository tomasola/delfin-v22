# Sunton HA Control (v2)

Este proyecto es la base modular para el panel de control de Home Assistant.

## Características

- **Interfaz Tabview**: Organización por pestañas (Sensores, Controles).
- **Polling Optimizado**: Actualización cada 5 segundos.
- **Drivers Estables**: Configuración de pantalla AXS15231B y toque manual I2C verificados.

## Estructura de código

- `/src/main.cpp`: Lógica principal y UI.
- `/include/secrets.h`: Credenciales WiFi y Token HA.

## Uso

Para añadir un nuevo sensor:

1. Añade un label en `createUI()`.
2. Actualiza el valor en `updateHA()`.
