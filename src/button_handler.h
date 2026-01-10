#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
#include <RCSwitch.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#define NUM_KEYS 16
#define NUM_RF_CODES 16

// Konstanten für die verbesserte Tastenerkennung
#define KEYPAD_THRESHOLD 500        // Mindest-ADC-Wert für "Taste gedrückt"
#define KEYPAD_TOLERANCE 50         // Toleranz für Ausreißer-Erkennung
#define KEYPAD_MIN_GUETE 75.0       // Mindest-Güte in Prozent
#define KEYPAD_RELEASE_COUNT 5      // Anzahl Messungen < threshold für "losgelassen"
#define KEYPAD_MIN_MESSUNGEN 10     // Mindestanzahl gültiger Messungen
#define KEYPAD_MAX_MESSUNGEN 1000   // Maximale Anzahl Messungen
#define KEYPAD_MESS_INTERVAL 10     // Intervall zwischen Messungen in ms

class AnalogKeypad {
private:
    uint8_t pin;
    bool measuring;                 // Aktuell in Messung?
    unsigned long lastReadTime;     // Für non-blocking Timing
    
    // Kalibrierwerte für 16 Tasten (ADC-Werte)
    int tastenWerte[NUM_KEYS];
    int anzahlTasten;
    
    // Standard-Kalibrierung (kann über setCalibration geändert werden)
    String kalibrierung;
    
    // Messdaten
    int messwerte[KEYPAD_MAX_MESSUNGEN];
    int anzahlMessungen;
    int unterSchwellwert;
    long summe;
    
    void parseKalibrierung();
    int berechneTaste();
    
public:
    AnalogKeypad(uint8_t adcPin);
    void begin();
    int loop();
    
    // Kalibrierung setzen (Format: "1:4095,2:3697,3:3202,...")
    void setCalibration(const String& calibration);
    String getCalibration() { return kalibrierung; }
    
    // Debug-Ausgabe aktivieren/deaktivieren
    bool debugOutput;
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
    
    // FreeRTOS Task für Keypad auf Core 0
    static TaskHandle_t keypadTaskHandle;
    static QueueHandle_t keyQueue;
    static ButtonHandler* instance;
    static void keypadTask(void* parameter);
    void processKey(int key);
    
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
