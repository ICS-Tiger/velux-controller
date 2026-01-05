#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
#include <RCSwitch.h>
#include <Preferences.h>

#define NUM_KEYS 16
#define NUM_RF_CODES 16

class AnalogKeypad {
private:
    uint8_t pin;
    int lastKey;
    unsigned long lastPressTime;
    unsigned long lastReadTime;
    bool keyPressed;
    
    // ADC-Schwellwerte für 16 Tasten (anpassbar nach Kalibrierung)
    const int thresholds[NUM_KEYS] = {
        50,   // Taste 0
        200,  // Taste 1
        350,  // Taste 2
        500,  // Taste 3
        650,  // Taste 4
        800,  // Taste 5
        950,  // Taste 6
        1100, // Taste 7
        1250, // Taste 8
        1400, // Taste 9
        1550, // Taste 10
        1700, // Taste 11
        1850, // Taste 12
        2000, // Taste 13
        2150, // Taste 14
        2300  // Taste 15
    };
    
    int readKey();
    
public:
    AnalogKeypad(uint8_t adcPin);
    void begin();
    int loop();
};

class RFReceiver {
private:
    RCSwitch* rcSwitch;
    Preferences prefs;
    unsigned long rfCodes[NUM_RF_CODES];
    bool learningMode;
    int learningKey;
    unsigned long learningStartTime;
    
    void loadRFCodes();
    void saveRFCode(int key, unsigned long code);
    int findKeyForCode(unsigned long code);
    
public:
    RFReceiver();
    void begin();
    int loop();
    
    void startLearning(int key);
    void cancelLearning();
    bool isLearning() { return learningMode; }
    int getLearningKey() { return learningKey; }
    unsigned long getRFCode(int key) { return rfCodes[key]; }
    void clearRFCode(int key);
    void clearAllRFCodes();
};

class ButtonHandler {
private:
    AnalogKeypad* keypad;
    RFReceiver* rfReceiver;
    
public:
    ButtonHandler();
    void begin();
    void loop();
    
    RFReceiver* getRFReceiver() { return rfReceiver; }
    
    // Motor-Callbacks
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
    
    // Spezial-Callbacks
    void (*onAllOpen)() = nullptr;
    void (*onAllClose)() = nullptr;
};

#endif
