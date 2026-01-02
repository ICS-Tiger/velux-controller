#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

enum ButtonEvent {
    BTN_NONE,
    BTN_PRESSED,
    BTN_RELEASED,
    BTN_LONG_PRESS
};

class Button {
private:
    uint8_t pin;
    bool lastState;
    bool currentState;
    unsigned long pressedTime;
    unsigned long lastDebounceTime;
    bool longPressTriggered;
    
public:
    Button(uint8_t buttonPin);
    void begin();
    ButtonEvent update();
    bool isPressed() { return currentState; }
};

class ButtonHandler {
private:
    Button* btnM1Open;
    Button* btnM1Close;
    Button* btnM2Open;
    Button* btnM2Close;
    Button* btnM3Open;
    Button* btnM3Close;
    Button* btnM4Open;
    Button* btnM4Close;
    
public:
    ButtonHandler();
    void begin();
    void loop();
    
    void (*onM1Open)() = nullptr;
    void (*onM1Close)() = nullptr;
    void (*onM1Stop)() = nullptr;
    
    void (*onM2Open)() = nullptr;
    void (*onM2Close)() = nullptr;
    void (*onM2Stop)() = nullptr;
    
    void (*onM3Open)() = nullptr;
    void (*onM3Close)() = nullptr;
    void (*onM3Stop)() = nullptr;
    
    void (*onM4Open)() = nullptr;
    void (*onM4Close)() = nullptr;
    void (*onM4Stop)() = nullptr;
};

#endif
