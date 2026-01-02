#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

class WebServerHandler {
private:
    AsyncWebServer server;
    
public:
    WebServerHandler();
    void begin();
    
    void (*onMotor1Command)(const char* cmd) = nullptr;
    void (*onMotor2Command)(const char* cmd) = nullptr;
    void (*onMotor3Command)(const char* cmd) = nullptr;
    void (*onMotor4Command)(const char* cmd) = nullptr;
    
    void (*onMotor1Learn)(const char* type) = nullptr;
    void (*onMotor2Learn)(const char* type) = nullptr;
    void (*onMotor3Learn)(const char* type) = nullptr;
    void (*onMotor4Learn)(const char* type) = nullptr;
    
    String (*getStatusJson)() = nullptr;
};

#endif
