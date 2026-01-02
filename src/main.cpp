#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "config.h"
#include "motor_controller.h"
#include "button_handler.h"
#include "mqtt_handler.h"
#include "web_server.h"

// Globale Objekte
MotorController* motor1;
MotorController* motor2;
MotorController* motor3;
MotorController* motor4;

ButtonHandler* buttons;
MQTTHandler* mqtt;
WebServerHandler* webserver;

unsigned long lastStatusUpdate = 0;

// WiFi Connect
void connectWiFi() {
    Serial.println("\n=== WiFi Verbindung ===");
    Serial.printf("SSID: %s\n", WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi verbunden!");
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\n✗ WiFi-Verbindung fehlgeschlagen!");
    }
}

// OTA Setup
void setupOTA() {
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    
    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        Serial.println("OTA Update Start: " + type);
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA Update Complete!");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    
    ArduinoOTA.begin();
    Serial.println("✓ OTA aktiviert");
}

// Status JSON generieren
String getStatusJson() {
    StaticJsonDocument<1024> doc;
    
    JsonObject m1 = doc["motor1"].to<JsonObject>();
    m1["position"] = motor1->getPosition();
    m1["current"] = motor1->getCurrent();
    m1["calibrated"] = motor1->getCalibrated();
    m1["state"] = motor1->getState();
    
    JsonObject m2 = doc["motor2"].to<JsonObject>();
    m2["position"] = motor2->getPosition();
    m2["current"] = motor2->getCurrent();
    m2["calibrated"] = motor2->getCalibrated();
    m2["state"] = motor2->getState();
    
    JsonObject m3 = doc["motor3"].to<JsonObject>();
    m3["position"] = motor3->getPosition();
    m3["current"] = motor3->getCurrent();
    m3["calibrated"] = motor3->getCalibrated();
    m3["state"] = motor3->getState();
    
    JsonObject m4 = doc["motor4"].to<JsonObject>();
    m4["position"] = motor4->getPosition();
    m4["current"] = motor4->getCurrent();
    m4["calibrated"] = motor4->getCalibrated();
    m4["state"] = motor4->getState();
    
    String output;
    serializeJson(doc, output);
    return output;
}

// Motor Command Handler
void handleMotorCommand(MotorController* motor, const char* cmd) {
    String command = String(cmd);
    command.toUpperCase();
    
    if (command == "OPEN") {
        motor->open();
    } else if (command == "CLOSE") {
        motor->close();
    } else if (command == "STOP") {
        motor->stop();
    } else {
        // Position (0-100)
        int pos = command.toInt();
        if (pos >= 0 && pos <= 100) {
            motor->moveToPosition(pos);
        }
    }
}

// Learn Handler
void handleLearn(MotorController* motor, const char* type) {
    String learnType = String(type);
    learnType.toLowerCase();
    
    if (learnType == "open") {
        motor->startLearnOpen();
    } else if (learnType == "close") {
        motor->startLearnClose();
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("╔═══════════════════════════════════════╗");
    Serial.println("║  VELUX ROLLADEN CONTROLLER v1.0      ║");
    Serial.println("║  ESP32 + BTS7960 + INA219            ║");
    Serial.println("╚═══════════════════════════════════════╝");
    Serial.println();
    
    // I2C Bus initialisieren
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.println("✓ I2C Bus initialisiert");
    
    // PWM Controller
    PWMController::begin();
    
    // Motoren initialisieren
    Serial.println("\n=== Motor Initialisierung ===");
    motor1 = new MotorController(1, M1_R_EN, M1_L_EN, INA219_ADDR_M1);
    motor2 = new MotorController(2, M2_R_EN, M2_L_EN, INA219_ADDR_M2);
    motor3 = new MotorController(3, M3_R_EN, M3_L_EN, INA219_ADDR_M3);
    motor4 = new MotorController(4, M4_R_EN, M4_L_EN, INA219_ADDR_M4);
    
    motor1->begin();
    motor2->begin();
    motor3->begin();
    motor4->begin();
    
    // Taster initialisieren
    Serial.println("\n=== Taster Initialisierung ===");
    buttons = new ButtonHandler();
    buttons->begin();
    
    // Button Callbacks
    buttons->onM1Open = []() { motor1->open(); };
    buttons->onM1Close = []() { motor1->close(); };
    buttons->onM1Stop = []() { motor1->stop(); };
    
    buttons->onM2Open = []() { motor2->open(); };
    buttons->onM2Close = []() { motor2->close(); };
    buttons->onM2Stop = []() { motor2->stop(); };
    
    buttons->onM3Open = []() { motor3->open(); };
    buttons->onM3Close = []() { motor3->close(); };
    buttons->onM3Stop = []() { motor3->stop(); };
    
    buttons->onM4Open = []() { motor4->open(); };
    buttons->onM4Close = []() { motor4->close(); };
    buttons->onM4Stop = []() { motor4->stop(); };
    
    // WiFi verbinden
    connectWiFi();
    
    // OTA Setup
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n=== OTA Setup ===");
        setupOTA();
    }
    
    // MQTT initialisieren
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n=== MQTT Initialisierung ===");
        mqtt = new MQTTHandler();
        mqtt->begin();
        
        // MQTT Callbacks
        mqtt->onMotor1Command = [](const char* cmd) { handleMotorCommand(motor1, cmd); };
        mqtt->onMotor2Command = [](const char* cmd) { handleMotorCommand(motor2, cmd); };
        mqtt->onMotor3Command = [](const char* cmd) { handleMotorCommand(motor3, cmd); };
        mqtt->onMotor4Command = [](const char* cmd) { handleMotorCommand(motor4, cmd); };
        
        mqtt->onAllCommand = [](const char* cmd) {
            handleMotorCommand(motor1, cmd);
            handleMotorCommand(motor2, cmd);
            handleMotorCommand(motor3, cmd);
            handleMotorCommand(motor4, cmd);
        };
        
        // Webserver initialisieren
        Serial.println("\n=== Webserver Initialisierung ===");
        webserver = new WebServerHandler();
        
        // Webserver Callbacks
        webserver->onMotor1Command = [](const char* cmd) { handleMotorCommand(motor1, cmd); };
        webserver->onMotor2Command = [](const char* cmd) { handleMotorCommand(motor2, cmd); };
        webserver->onMotor3Command = [](const char* cmd) { handleMotorCommand(motor3, cmd); };
        webserver->onMotor4Command = [](const char* cmd) { handleMotorCommand(motor4, cmd); };
        
        webserver->onMotor1Learn = [](const char* type) { handleLearn(motor1, type); };
        webserver->onMotor2Learn = [](const char* type) { handleLearn(motor2, type); };
        webserver->onMotor3Learn = [](const char* type) { handleLearn(motor3, type); };
        webserver->onMotor4Learn = [](const char* type) { handleLearn(motor4, type); };
        
        webserver->getStatusJson = getStatusJson;
        
        webserver->begin();
    }
    
    Serial.println("\n╔═══════════════════════════════════════╗");
    Serial.println("║         SYSTEM BEREIT!                ║");
    Serial.println("╚═══════════════════════════════════════╝");
    Serial.println();
    Serial.printf("Webinterface: http://%s\n", WiFi.localIP().toString().c_str());
    Serial.printf("MQTT Prefix: %s\n", MQTT_TOPIC_PREFIX);
    Serial.println();
}

void loop() {
    // OTA Handle
    ArduinoOTA.handle();
    
    // PWM Controller Update (Sanftanlauf)
    PWMController::loop();
    
    // Motor Updates
    motor1->loop();
    motor2->loop();
    motor3->loop();
    motor4->loop();
    
    // Button Updates
    buttons->loop();
    
    // MQTT Update
    if (mqtt) {
        mqtt->loop();
    }
    
    // Status via MQTT publishen (alle 2 Sekunden)
    unsigned long now = millis();
    if (now - lastStatusUpdate > 2000) {
        lastStatusUpdate = now;
        
        if (mqtt) {
            mqtt->publishMotorState(1, "running", motor1->getPosition(), motor1->getCurrent());
            mqtt->publishMotorState(2, "running", motor2->getPosition(), motor2->getCurrent());
            mqtt->publishMotorState(3, "running", motor3->getPosition(), motor3->getCurrent());
            mqtt->publishMotorState(4, "running", motor4->getPosition(), motor4->getCurrent());
        }
    }
    
    delay(10);
}
