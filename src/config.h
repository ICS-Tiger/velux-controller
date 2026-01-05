#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ===== WiFi =====
#define WIFI_SSID "Kpriv24"
#define WIFI_PASSWORD "LTPR020591"
//#define WIFI_SSID "BitEins24"
//#define WIFI_PASSWORD "thebigboss314"
#define HOSTNAME "velux-controller"
#define OTA_PASSWORD "velux2026"

// ===== MQTT =====
#define MQTT_SERVER "192.168.0.22"
#define MQTT_PORT 1883
#define MQTT_USER "bossi"
#define MQTT_PASSWORD "bigboss1"
#define MQTT_TOPIC_PREFIX "velux"

// ===== Motor Pins (4x BTS7960 - OPTIMIERT) =====
// PWM-Pins (gemeinsam für alle Motoren)
#define RPWM_ALL 25
#define LPWM_ALL 26
#define RPWM_CHANNEL 0
#define LPWM_CHANNEL 1

// Relais für Motorstromversorgung
#define MOTOR_POWER_RELAY_PIN 23
#define RELAY_PRE_ON_DELAY_MS 300      // Relais schaltet 300ms VOR Motoren ein
#define RELAY_POST_OFF_DELAY_MS 20000  // Relais schaltet 20s NACH letztem Motor aus

// Enable-Pins (Paare liegen nebeneinander)
#define M1_R_EN 32
#define M1_L_EN 33
#define M2_R_EN 27
#define M2_L_EN 14
#define M3_R_EN 16
#define M3_L_EN 17
#define M4_R_EN 18
#define M4_L_EN 19

// ===== INA219 Stromsensoren =====
#define INA219_ADDR_M1 0x40
#define INA219_ADDR_M2 0x41
#define INA219_ADDR_M3 0x44
#define INA219_ADDR_M4 0x45

// Überstromschutz
#define MAX_CURRENT_MA 3000.0     // 3A Maximum
#define OVERCURRENT_TIME_MS 500    // Überstrom für 500ms = Abschaltung
#define CURRENT_CHECK_INTERVAL 100 // Stromprüfung alle 100ms

// ===== Analoges Keypad (16 Tasten an einem ADC-Pin) =====
#define KEYPAD_PIN 34
#define KEYPAD_DEBOUNCE_MS 50
#define KEYPAD_READ_INTERVAL 50

// Tastenbelegung:
// 0-3: Motor 1-4 AUF
// 4-7: Motor 1-4 STOP  
// 8-11: Motor 1-4 ZU
// 12: Alle AUF
// 13: Alle ZU
// 14-15: Reserve

// ===== 433 MHz RF-Empfänger =====
#define RF_RECEIVER_PIN 35
#define RF_RECEIVER_INTERRUPT 35  // ESP32: Pin = Interrupt
#define RF_LEARNING_MODE_TIMEOUT 30000  // 30 Sekunden für RF-Code-Anlernen

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
