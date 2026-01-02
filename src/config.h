#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ===== WiFi =====
#define WIFI_SSID "BitEins24"
#define WIFI_PASSWORD "thebigboss314"
#define HOSTNAME "velux-controller"
#define OTA_PASSWORD "velux2026"

// ===== MQTT =====
#define MQTT_SERVER "192.168.0.22"
#define MQTT_PORT 1883
#define MQTT_USER "bossi"
#define MQTT_PASSWORD "bigboss1"
#define MQTT_TOPIC_PREFIX "velux"

// ===== Motor Pins (4x BTS7960 - OPTIMIERT) =====
#define RPWM_ALL 25
#define LPWM_ALL 26
#define RPWM_CHANNEL 0
#define LPWM_CHANNEL 1

#define M1_R_EN 27
#define M1_L_EN 14
#define M2_R_EN 32
#define M2_L_EN 33
#define M3_R_EN 16
#define M3_L_EN 17
#define M4_R_EN 19
#define M4_L_EN 21

// ===== Taster =====
#define BTN_M1_OPEN 22
#define BTN_M1_CLOSE 23
#define BTN_M2_OPEN 4
#define BTN_M2_CLOSE 5
#define BTN_M3_OPEN 18
#define BTN_M3_CLOSE 15
#define BTN_M4_OPEN 2
#define BTN_M4_CLOSE 0

#define BUTTON_DEBOUNCE_MS 50
#define BUTTON_LONG_PRESS_MS 1000

// ===== Strommessung =====
#define I2C_SDA 13
#define I2C_SCL 12
#define INA219_ADDR_M1 0x40
#define INA219_ADDR_M2 0x41
#define INA219_ADDR_M3 0x44
#define INA219_ADDR_M4 0x45

// ===== Webserver =====
#define WEB_SERVER_PORT 80

// ===== Motor Settings =====
#define PWM_FREQ 1000
#define PWM_RESOLUTION 8
#define MAX_RUNTIME_MS 120000
#define POSITION_UPDATE_INTERVAL 100

// ===== Sanftanlauf =====
#define SOFT_START_ENABLED true
#define SOFT_START_DURATION_MS 2000
#define SOFT_START_MIN_PWM 100
#define SOFT_START_MAX_PWM 255
#define SOFT_START_STEP_INTERVAL 50

// ===== EEPROM =====
#define PREFS_NAMESPACE "velux"

#endif
