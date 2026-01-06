#ifndef CONFIG_H
#define CONFIG_H

// WiFi & Home Assistant (Defaults)
#define HA_UPDATE_INTERVAL 5000
#define LVGL_BUFFER_SIZE (480 * 20)

// Nombres de Zonas (Shorter for Tab Bar)
#define NAME_HABITACION1 "Hab 1"
#define NAME_HABITACION2 "Hab 2"
#define NAME_HABITACION3 "Hab 3"
#define NAME_SALON "Salon"
#define NAME_PASILLO "Pasillo"

// Entidades Habitación 1
#define LIGHT_HAB1_LUZ1 "light.hab1_primaria"
#define LIGHT_HAB1_LUZ2 "light.hab1_secundaria"
#define LED_HAB1 "light.hab1_led"
#define COVER_HABITACION1 "cover.persiana_hab1"
#define SENSOR_TEMP_HABITACION1 "sensor.hab1_temperatura"
#define SENSOR_HUM_HABITACION1 "sensor.hab1_humedad"
#define SENSOR_PRESENCIA_HABITACION1 "binary_sensor.hab1_presencia"

// Entidades Habitación 2
#define LIGHT_HAB2_LUZ1 "light.hab2_primaria"
#define LIGHT_HAB2_LUZ2 "light.hab2_secundaria"
#define LED_HAB2 "light.hab2_led"
#define COVER_HABITACION2 "cover.persiana_hab2"
#define SENSOR_TEMP_HABITACION2 "sensor.hab2_temperatura"
#define SENSOR_HUM_HABITACION2 "sensor.hab2_humedad"
#define SENSOR_PRESENCIA_HABITACION2 "binary_sensor.hab2_presencia"

// Entidades Habitación 3
#define LIGHT_HAB3_LUZ1 "light.hab3_primaria"
#define LIGHT_HAB3_LUZ2 "light.hab3_secundaria"
#define LED_HAB3 "light.hab3_led"
#define COVER_HABITACION3 "cover.persiana_hab3"
#define SENSOR_TEMP_HABITACION3 "sensor.hab3_temperatura"
#define SENSOR_HUM_HABITACION3 "sensor.hab3_humedad"
#define SENSOR_PRESENCIA_HABITACION3 "binary_sensor.hab3_presencia"

// Entidades Salón
#define LIGHT_SALON_LUZ1 "light.salon_primaria"
#define LIGHT_SALON_LUZ2 "light.salon_secundaria"
#define LED_SALON "light.salon_led"
#define COVER_SALON_1 "cover.persiana_salon_1"
#define COVER_SALON_2 "cover.persiana_salon_2"
#define SENSOR_TEMP_SALON "sensor.salon_temperatura"
#define SENSOR_HUM_SALON "sensor.salon_humedad"
#define SENSOR_PRESENCIA_SALON "binary_sensor.salon_presencia"

// Entidades Pasillo
#define LIGHT_PASILLO_LUZ1 "light.pasillo_1"
#define LIGHT_PASILLO_LUZ2 "light.pasillo_2"
#define LED_PASILLO "light.pasillo_led"
#define SENSOR_PRESENCIA_PASILLO "binary_sensor.pasillo_presencia"

// Luces Baño
#define LIGHT_BANO1 "light.bano_1"
#define LIGHT_BANO2 "light.bano_2"

// Escenas
#define SCENE_BUENOS_DIAS "scene.buenos_dias"
#define SCENE_SALIR_CASA "scene.salir_casa"
#define SCENE_LLEGAR_CASA "scene.llegar_casa"
#define SCENE_DORMIR "scene.dormir"
#define SCENE_CINE "scene.modo_cine"

#endif
