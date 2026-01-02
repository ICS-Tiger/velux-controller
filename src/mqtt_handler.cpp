#include "mqtt_handler.h"
#include "config.h"
#include <ArduinoJson.h>

MQTTHandler* MQTTHandler::instance = nullptr;

MQTTHandler::MQTTHandler() : mqttClient(wifiClient) {
    instance = this;
    lastReconnectAttempt = 0;
}

void MQTTHandler::begin() {
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback([](char* topic, byte* payload, unsigned int length) {
        if (instance) {
            instance->callback(topic, payload, length);
        }
    });
    
    Serial.println("MQTT: Initialisiert");
}

void MQTTHandler::loop() {
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            reconnect();
        }
    } else {
        mqttClient.loop();
    }
}

void MQTTHandler::reconnect() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    Serial.print("MQTT: Verbinde...");
    
    String clientId = String(HOSTNAME) + "-" + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
        Serial.println(" verbunden!");
        
        // Subscribe zu allen Motor-Topics
        mqttClient.subscribe((String(MQTT_TOPIC_PREFIX) + "/motor1/set").c_str());
        mqttClient.subscribe((String(MQTT_TOPIC_PREFIX) + "/motor2/set").c_str());
        mqttClient.subscribe((String(MQTT_TOPIC_PREFIX) + "/motor3/set").c_str());
        mqttClient.subscribe((String(MQTT_TOPIC_PREFIX) + "/motor4/set").c_str());
        mqttClient.subscribe((String(MQTT_TOPIC_PREFIX) + "/all/set").c_str());
        
        // Online Status
        publish("status", "online");
        
    } else {
        Serial.printf(" fehlgeschlagen (rc=%d)\n", mqttClient.state());
    }
}

void MQTTHandler::callback(char* topic, byte* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    Serial.printf("MQTT: %s = %s\n", topic, message);
    
    String topicStr = String(topic);
    String prefix = String(MQTT_TOPIC_PREFIX);
    
    if (topicStr == prefix + "/motor1/set" && onMotor1Command) {
        onMotor1Command(message);
    } else if (topicStr == prefix + "/motor2/set" && onMotor2Command) {
        onMotor2Command(message);
    } else if (topicStr == prefix + "/motor3/set" && onMotor3Command) {
        onMotor3Command(message);
    } else if (topicStr == prefix + "/motor4/set" && onMotor4Command) {
        onMotor4Command(message);
    } else if (topicStr == prefix + "/all/set" && onAllCommand) {
        onAllCommand(message);
    }
}

void MQTTHandler::publish(const char* topic, const char* payload) {
    if (!mqttClient.connected()) return;
    
    String fullTopic = String(MQTT_TOPIC_PREFIX) + "/" + String(topic);
    mqttClient.publish(fullTopic.c_str(), payload, true);
}

void MQTTHandler::publishMotorState(uint8_t motorId, const char* state, uint8_t position, float current) {
    if (!mqttClient.connected()) return;
    
    StaticJsonDocument<200> doc;
    doc["state"] = state;
    doc["position"] = position;
    doc["current"] = current;
    
    String output;
    serializeJson(doc, output);
    
    String topic = "motor" + String(motorId) + "/state";
    publish(topic.c_str(), output.c_str());
}
