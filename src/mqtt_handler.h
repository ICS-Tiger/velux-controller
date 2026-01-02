#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

class MQTTHandler {
private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    unsigned long lastReconnectAttempt;
    
    void reconnect();
    void callback(char* topic, byte* payload, unsigned int length);
    static MQTTHandler* instance;
    
public:
    MQTTHandler();
    void begin();
    void loop();
    
    void publish(const char* topic, const char* payload);
    void publishMotorState(uint8_t motorId, const char* state, uint8_t position, float current);
    
    void (*onMotor1Command)(const char* cmd) = nullptr;
    void (*onMotor2Command)(const char* cmd) = nullptr;
    void (*onMotor3Command)(const char* cmd) = nullptr;
    void (*onMotor4Command)(const char* cmd) = nullptr;
    void (*onAllCommand)(const char* cmd) = nullptr;
};

#endif
