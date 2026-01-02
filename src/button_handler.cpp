#include "button_handler.h"
#include "config.h"

// ===== AnalogKeypad =====

AnalogKeypad::AnalogKeypad(uint8_t adcPin) {
    pin = adcPin;
    lastKey = -1;
    lastPressTime = 0;
    lastReadTime = 0;
    keyPressed = false;
}

void AnalogKeypad::begin() {
    pinMode(pin, INPUT);
    analogSetAttenuation(ADC_11db); // 0-3.3V Range
    Serial.printf("Analoges Keypad initialisiert auf Pin %d\n", pin);
}

int AnalogKeypad::readKey() {
    int adcValue = analogRead(pin);
    
    // Keine Taste gedrückt (hoher Wert nahe 4095)
    if (adcValue > 2500) {
        return -1;
    }
    
    // Finde die nächstliegende Taste
    int closestKey = -1;
    int minDiff = 9999;
    
    for (int i = 0; i < NUM_KEYS; i++) {
        int diff = abs(adcValue - thresholds[i]);
        if (diff < minDiff && diff < 100) { // Toleranz von ±100
            minDiff = diff;
            closestKey = i;
        }
    }
    
    return closestKey;
}

int AnalogKeypad::loop() {
    unsigned long now = millis();
    
    // Nur alle KEYPAD_READ_INTERVAL ms lesen
    if (now - lastReadTime < KEYPAD_READ_INTERVAL) {
        return -1;
    }
    lastReadTime = now;
    
    int currentKey = readKey();
    
    // Taste wurde gedrückt (Flanke)
    if (currentKey != -1 && lastKey == -1) {
        if (!keyPressed) {
            keyPressed = true;
            lastPressTime = now;
            lastKey = currentKey;
            Serial.printf("Keypad: Taste %d gedrückt (ADC: %d)\n", currentKey, analogRead(pin));
            return currentKey;
        }
    }
    // Taste wurde losgelassen
    else if (currentKey == -1 && lastKey != -1) {
        keyPressed = false;
        lastKey = -1;
    }
    
    return -1;
}

// ===== ButtonHandler =====

ButtonHandler::ButtonHandler() {
    keypad = new AnalogKeypad(KEYPAD_PIN);
}

void ButtonHandler::begin() {
    keypad->begin();
    Serial.println("✓ Button Handler initialisiert (16-Tasten Analog Keypad)");
}

void ButtonHandler::loop() {
    int key = keypad->loop();
    
    if (key == -1) return;
    
    // Tastenbelegung
    switch (key) {
        // Motor 1-4 AUF (Tasten 0-3)
        case 0: if (onM1Open) onM1Open(); break;
        case 1: if (onM2Open) onM2Open(); break;
        case 2: if (onM3Open) onM3Open(); break;
        case 3: if (onM4Open) onM4Open(); break;
        
        // Motor 1-4 ZU (Tasten 4-7)
        case 4: if (onM1Close) onM1Close(); break;
        case 5: if (onM2Close) onM2Close(); break;
        case 6: if (onM3Close) onM3Close(); break;
        case 7: if (onM4Close) onM4Close(); break;
        
        // Motor 1-4 STOP (Tasten 8-11)
        case 8: if (onM1Stop) onM1Stop(); break;
        case 9: if (onM2Stop) onM2Stop(); break;
        case 10: if (onM3Stop) onM3Stop(); break;
        case 11: if (onM4Stop) onM4Stop(); break;
        
        // Alle AUF/ZU (Tasten 12-13)
        case 12: 
            Serial.println("Keypad: ALLE AUF");
            if (onAllOpen) onAllOpen(); 
            break;
        case 13: 
            Serial.println("Keypad: ALLE ZU");
            if (onAllClose) onAllClose(); 
            break;
        
        // Reserve (Tasten 14-15)
        case 14:
        case 15:
            Serial.printf("Keypad: Reserve-Taste %d\n", key);
            break;
    }
}
