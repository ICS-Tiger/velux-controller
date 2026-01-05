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
    
    // ADC konfigurieren
    adcAttachPin(pin);
    analogSetPinAttenuation(pin, ADC_11db); // 0-3.3V Range
    analogReadResolution(12); // 12-bit (0-4095)
    
    Serial.printf("✓ Analoges Keypad initialisiert auf GPIO %d\n", pin);
}

int AnalogKeypad::readKey() {
    int adcValue = analogRead(pin);
    
    // Keine Taste gedrückt (niedriger Wert nahe 0)
    if (adcValue < 50) {
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

// ===== RFReceiver =====

RFReceiver::RFReceiver() {
    rcSwitch = new RCSwitch();
    learningMode = false;
    learningKey = -1;
    learningStartTime = 0;
    
    for (int i = 0; i < NUM_RF_CODES; i++) {
        rfCodes[i] = 0;
    }
}

void RFReceiver::begin() {
    rcSwitch->enableReceive(RF_RECEIVER_INTERRUPT);
    loadRFCodes();
    Serial.printf("✓ RF-Empfänger initialisiert auf GPIO %d\n", RF_RECEIVER_PIN);
}

void RFReceiver::loadRFCodes() {
    prefs.begin("rf_codes", true);
    
    for (int i = 0; i < NUM_RF_CODES; i++) {
        String key = "code_" + String(i);
        rfCodes[i] = prefs.getULong(key.c_str(), 0);
        if (rfCodes[i] != 0) {
            Serial.printf("RF-Code %d: %lu\n", i, rfCodes[i]);
        }
    }
    
    prefs.end();
}

void RFReceiver::saveRFCode(int key, unsigned long code) {
    if (key < 0 || key >= NUM_RF_CODES) return;
    
    rfCodes[key] = code;
    
    prefs.begin("rf_codes", false);
    String keyName = "code_" + String(key);
    prefs.putULong(keyName.c_str(), code);
    prefs.end();
    
    Serial.printf("✓ RF-Code %d gespeichert: %lu\n", key, code);
}

int RFReceiver::findKeyForCode(unsigned long code) {
    for (int i = 0; i < NUM_RF_CODES; i++) {
        if (rfCodes[i] == code) {
            return i;
        }
    }
    return -1;
}

void RFReceiver::startLearning(int key) {
    if (key < 0 || key >= NUM_RF_CODES) return;
    
    learningMode = true;
    learningKey = key;
    learningStartTime = millis();
    Serial.printf(">>> RF-Lernmodus für Taste %d gestartet (30s)\n", key);
}

void RFReceiver::cancelLearning() {
    learningMode = false;
    learningKey = -1;
    Serial.println("RF-Lernmodus abgebrochen");
}

void RFReceiver::clearRFCode(int key) {
    if (key < 0 || key >= NUM_RF_CODES) return;
    
    rfCodes[key] = 0;
    
    prefs.begin("rf_codes", false);
    String keyName = "code_" + String(key);
    prefs.remove(keyName.c_str());
    prefs.end();
    
    Serial.printf("RF-Code %d gelöscht\n", key);
}

void RFReceiver::clearAllRFCodes() {
    prefs.begin("rf_codes", false);
    prefs.clear();
    prefs.end();
    
    for (int i = 0; i < NUM_RF_CODES; i++) {
        rfCodes[i] = 0;
    }
    
    Serial.println("Alle RF-Codes gelöscht");
}

int RFReceiver::loop() {
    // Lernmodus-Timeout prüfen
    if (learningMode) {
        if (millis() - learningStartTime > RF_LEARNING_MODE_TIMEOUT) {
            Serial.println("RF-Lernmodus: Timeout");
            cancelLearning();
            return -1;
        }
    }
    
    // RF-Signal empfangen?
    if (rcSwitch->available()) {
        unsigned long code = rcSwitch->getReceivedValue();
        
        if (code != 0) {
            // Im Lernmodus: Code speichern
            if (learningMode) {
                saveRFCode(learningKey, code);
                Serial.printf("✓✓✓ RF-Code für Taste %d angelernt: %lu\n", learningKey, code);
                int learnedKey = learningKey;
                cancelLearning();
                rcSwitch->resetAvailable();
                return learnedKey;
            }
            // Normal-Modus: Code suchen und Taste zurückgeben
            else {
                int key = findKeyForCode(code);
                if (key != -1) {
                    Serial.printf("RF empfangen: Code %lu -> Taste %d\n", code, key);
                    rcSwitch->resetAvailable();
                    return key;
                } else {
                    Serial.printf("RF empfangen: Unbekannter Code %lu\n", code);
                }
            }
        }
        
        rcSwitch->resetAvailable();
    }
    
    return -1;
}

// ===== ButtonHandler =====

ButtonHandler::ButtonHandler() {
    keypad = new AnalogKeypad(KEYPAD_PIN);
    rfReceiver = new RFReceiver();
}

void ButtonHandler::begin() {
    keypad->begin();
    rfReceiver->begin();
    Serial.println("✓ Button Handler initialisiert (16-Tasten Analog Keypad + RF-Empfänger)");
}

void ButtonHandler::loop() {
    // Keypad prüfen
    int key = keypad->loop();
    
    // Wenn keine Keypad-Taste, dann RF prüfen
    if (key == -1) {
        key = rfReceiver->loop();
    }
    
    if (key == -1) return;
    
    // Tastenbelegung
    switch (key) {
        // Motor 1-4 AUF (Tasten 0-3)
        case 0: 
            Serial.println("Keypad: Motor 1 AUF");
            if (onM1Open) onM1Open(); 
            break;
        case 1: 
            Serial.println("Keypad: Motor 2 AUF");
            if (onM2Open) onM2Open(); 
            break;
        case 2: 
            Serial.println("Keypad: Motor 3 AUF");
            if (onM3Open) onM3Open(); 
            break;
        case 3: 
            Serial.println("Keypad: Motor 4 AUF");
            if (onM4Open) onM4Open(); 
            break;
        
        // Motor 1-4 STOP (Tasten 4-7)
        case 4: 
            Serial.println("Keypad: Motor 1 STOP");
            if (onM1Stop) onM1Stop(); 
            break;
        case 5: 
            Serial.println("Keypad: Motor 2 STOP");
            if (onM2Stop) onM2Stop(); 
            break;
        case 6: 
            Serial.println("Keypad: Motor 3 STOP");
            if (onM3Stop) onM3Stop(); 
            break;
        case 7: 
            Serial.println("Keypad: Motor 4 STOP");
            if (onM4Stop) onM4Stop(); 
            break;
        
        // Motor 1-4 ZU (Tasten 8-11)
        case 8: 
            Serial.println("Keypad: Motor 1 ZU");
            if (onM1Close) onM1Close(); 
            break;
        case 9: 
            Serial.println("Keypad: Motor 2 ZU");
            if (onM2Close) onM2Close(); 
            break;
        case 10: 
            Serial.println("Keypad: Motor 3 ZU");
            if (onM3Close) onM3Close(); 
            break;
        case 11: 
            Serial.println("Keypad: Motor 4 ZU");
            if (onM4Close) onM4Close(); 
            break;
        
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
