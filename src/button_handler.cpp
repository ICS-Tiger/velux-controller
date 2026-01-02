#include "button_handler.h"
#include "config.h"

Button::Button(uint8_t buttonPin) {
    pin = buttonPin;
    lastState = HIGH;
    currentState = HIGH;
    pressedTime = 0;
    lastDebounceTime = 0;
    longPressTriggered = false;
}

void Button::begin() {
    pinMode(pin, INPUT_PULLUP);
}

ButtonEvent Button::update() {
    bool reading = digitalRead(pin);
    unsigned long now = millis();
    
    if (reading != lastState) {
        lastDebounceTime = now;
    }
    
    if ((now - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
        if (reading != currentState) {
            currentState = reading;
            
            if (currentState == LOW) {
                pressedTime = now;
                longPressTriggered = false;
                lastState = reading;
                return BTN_PRESSED;
            } else {
                lastState = reading;
                return BTN_RELEASED;
            }
        }
    }
    
    if (currentState == LOW && !longPressTriggered && 
        (now - pressedTime) > BUTTON_LONG_PRESS_MS) {
        longPressTriggered = true;
        lastState = reading;
        return BTN_LONG_PRESS;
    }
    
    lastState = reading;
    return BTN_NONE;
}

ButtonHandler::ButtonHandler() {
    btnM1Open = new Button(BTN_M1_OPEN);
    btnM1Close = new Button(BTN_M1_CLOSE);
    btnM2Open = new Button(BTN_M2_OPEN);
    btnM2Close = new Button(BTN_M2_CLOSE);
    btnM3Open = new Button(BTN_M3_OPEN);
    btnM3Close = new Button(BTN_M3_CLOSE);
    btnM4Open = new Button(BTN_M4_OPEN);
    btnM4Close = new Button(BTN_M4_CLOSE);
}

void ButtonHandler::begin() {
    btnM1Open->begin();
    btnM1Close->begin();
    btnM2Open->begin();
    btnM2Close->begin();
    btnM3Open->begin();
    btnM3Close->begin();
    btnM4Open->begin();
    btnM4Close->begin();
    
    Serial.println("Button-Handler: Initialisiert (8 Taster)");
}

void ButtonHandler::loop() {
    ButtonEvent evt;
    
    evt = btnM1Open->update();
    if (evt == BTN_PRESSED && onM1Open) onM1Open();
    if (evt == BTN_LONG_PRESS && onM1Stop) onM1Stop();
    
    evt = btnM1Close->update();
    if (evt == BTN_PRESSED && onM1Close) onM1Close();
    if (evt == BTN_LONG_PRESS && onM1Stop) onM1Stop();
    
    evt = btnM2Open->update();
    if (evt == BTN_PRESSED && onM2Open) onM2Open();
    if (evt == BTN_LONG_PRESS && onM2Stop) onM2Stop();
    
    evt = btnM2Close->update();
    if (evt == BTN_PRESSED && onM2Close) onM2Close();
    if (evt == BTN_LONG_PRESS && onM2Stop) onM2Stop();
    
    evt = btnM3Open->update();
    if (evt == BTN_PRESSED && onM3Open) onM3Open();
    if (evt == BTN_LONG_PRESS && onM3Stop) onM3Stop();
    
    evt = btnM3Close->update();
    if (evt == BTN_PRESSED && onM3Close) onM3Close();
    if (evt == BTN_LONG_PRESS && onM3Stop) onM3Stop();
    
    evt = btnM4Open->update();
    if (evt == BTN_PRESSED && onM4Open) onM4Open();
    if (evt == BTN_LONG_PRESS && onM4Stop) onM4Stop();
    
    evt = btnM4Close->update();
    if (evt == BTN_PRESSED && onM4Close) onM4Close();
    if (evt == BTN_LONG_PRESS && onM4Stop) onM4Stop();
}
