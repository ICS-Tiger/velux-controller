#include "button_handler.h"
#include "config.h"

// ===== AnalogKeypad (Verbesserte Mehrfach-Messung mit Güteprüfung - NON-BLOCKING) =====

AnalogKeypad::AnalogKeypad(uint8_t adcPin) {
    pin = adcPin;
    measuring = false;
    anzahlTasten = 0;
    anzahlMessungen = 0;
    unterSchwellwert = 0;
    summe = 0;
    lastReadTime = 0;
    debugOutput = true;
    
    // Standard-Kalibrierung (deine ermittelten Werte)
    kalibrierung = "1:4095,2:3697,3:3202,4:2864,5:2412,6:2250,7:2114,8:1986,9:1758,10:1672,11:1593,12:1512,13:1373,14:1072,15:869,16:729";
    
    for (int i = 0; i < NUM_KEYS; i++) {
        tastenWerte[i] = 0;
    }
}

void AnalogKeypad::begin() {
    pinMode(pin, INPUT);
    
    // ADC konfigurieren
    analogSetAttenuation(ADC_11db);  // 0-3.3V Range
    analogReadResolution(12);        // 12-bit (0-4095)
    
    // Kalibrierwerte laden
    parseKalibrierung();
    
    Serial.printf("✓ Analoges Keypad initialisiert auf GPIO %d\n", pin);
    Serial.printf("  %d Tasten kalibriert, Schwellwert: %d\n", anzahlTasten, KEYPAD_THRESHOLD);
}

void AnalogKeypad::parseKalibrierung() {
    int startIndex = 0;
    anzahlTasten = 0;
    
    while (startIndex < (int)kalibrierung.length() && anzahlTasten < NUM_KEYS) {
        int kommaIndex = kalibrierung.indexOf(',', startIndex);
        if (kommaIndex == -1) {
            kommaIndex = kalibrierung.length();
        }
        
        String eintrag = kalibrierung.substring(startIndex, kommaIndex);
        int doppelpunktIndex = eintrag.indexOf(':');
        
        if (doppelpunktIndex > 0) {
            int wert = eintrag.substring(doppelpunktIndex + 1).toInt();
            tastenWerte[anzahlTasten] = wert;
            anzahlTasten++;
        }
        
        startIndex = kommaIndex + 1;
    }
    
    if (debugOutput) {
        Serial.print("Kalibrierwerte geladen: ");
        Serial.print(anzahlTasten);
        Serial.println(" Tasten");
    }
}

void AnalogKeypad::setCalibration(const String& calibration) {
    kalibrierung = calibration;
    parseKalibrierung();
}

int AnalogKeypad::berechneTaste() {
    // Mindestens KEYPAD_MIN_MESSUNGEN erforderlich
    if (anzahlMessungen < KEYPAD_MIN_MESSUNGEN) {
        if (debugOutput) Serial.println("Keypad: Zu wenige Messwerte!");
        return -1;
    }
    
    // Erster Mittelwert
    float mittelwert1 = summe / (float)anzahlMessungen;
    
    // Bereinigter Mittelwert (ohne Ausreißer)
    long summeBereinigt = 0;
    int anzahlGueltig = 0;
    
    for (int i = 0; i < anzahlMessungen; i++) {
        float abweichung = abs(messwerte[i] - mittelwert1);
        
        if (abweichung <= KEYPAD_TOLERANCE) {
            summeBereinigt += messwerte[i];
            anzahlGueltig++;
        }
    }
    
    if (anzahlGueltig == 0) {
        if (debugOutput) Serial.println("Keypad: Keine gültigen Werte!");
        return -1;
    }
    
    int mittelwertBereinigt = summeBereinigt / anzahlGueltig;
    float guete = (anzahlGueltig / (float)anzahlMessungen) * 100.0;
    
    // Debug-Ausgabe
    if (debugOutput) {
        Serial.print("Keypad: Messungen: ");
        Serial.print(anzahlMessungen);
        Serial.print(" | Wert: ");
        Serial.print(mittelwertBereinigt);
        Serial.print(" | Güte: ");
        Serial.print(guete, 1);
        Serial.print("%");
    }
    
    // Güte prüfen
    if (guete < KEYPAD_MIN_GUETE) {
        if (debugOutput) Serial.println(" -> Güte zu schlecht!");
        return -1;
    }
    
    // Nächstgelegene Taste finden
    int naechsteTaste = -1;
    int kleinsterAbstand = 9999;
    
    for (int i = 0; i < anzahlTasten; i++) {
        int abstand = abs(tastenWerte[i] - mittelwertBereinigt);
        
        if (abstand < kleinsterAbstand) {
            kleinsterAbstand = abstand;
            naechsteTaste = i;  // 0-basiert (Taste 0-15)
        }
    }
    
    if (debugOutput) {
        Serial.print(" | Abstand: ");
        Serial.print(kleinsterAbstand);
        Serial.print(" -> Taste ");
        Serial.println(naechsteTaste);
    }
    
    return naechsteTaste;
}

int AnalogKeypad::loop() {
    unsigned long now = millis();
    
    // Non-blocking: Nur alle KEYPAD_MESS_INTERVAL ms messen
    if (now - lastReadTime < KEYPAD_MESS_INTERVAL) {
        return -1;
    }
    lastReadTime = now;
    
    int currentValue = analogRead(pin);
    
    // Taste wurde gedrückt (über Schwellwert)
    if (currentValue > KEYPAD_THRESHOLD) {
        if (!measuring) {
            // Neue Messung starten
            measuring = true;
            anzahlMessungen = 0;
            unterSchwellwert = 0;
            summe = 0;
        }
        
        // Messwert aufnehmen
        if (anzahlMessungen < KEYPAD_MAX_MESSUNGEN) {
            messwerte[anzahlMessungen] = currentValue;
            summe += currentValue;
            anzahlMessungen++;
        }
        unterSchwellwert = 0;  // Reset Counter
        
        return -1;  // Noch in Messung
    }
    else {
        // Unter Schwellwert
        if (measuring) {
            unterSchwellwert++;
            
            // Taste losgelassen? (KEYPAD_RELEASE_COUNT mal unter Schwellwert)
            if (unterSchwellwert >= KEYPAD_RELEASE_COUNT) {
                measuring = false;
                
                // Taste berechnen
                int taste = berechneTaste();
                
                // Reset für nächste Messung
                anzahlMessungen = 0;
                unterSchwellwert = 0;
                summe = 0;
                
                return taste;
            }
        }
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

// ===== ButtonHandler mit FreeRTOS Task auf Core 0 =====

TaskHandle_t ButtonHandler::keypadTaskHandle = nullptr;
QueueHandle_t ButtonHandler::keyQueue = nullptr;
ButtonHandler* ButtonHandler::instance = nullptr;

ButtonHandler::ButtonHandler() {
    keypad = new AnalogKeypad(KEYPAD_PIN);
    rfReceiver = new RFReceiver();
    instance = this;
}

// Keypad-Task läuft auf Core 0 (unabhängig vom Hauptloop)
void ButtonHandler::keypadTask(void* parameter) {
    AnalogKeypad* kp = (AnalogKeypad*)parameter;
    
    Serial.println("Keypad-Task gestartet auf Core 0");
    
    for (;;) {
        int key = kp->loop();
        
        if (key != -1) {
            // Taste erkannt -> in Queue senden
            xQueueSend(keyQueue, &key, 0);
        }
        
        vTaskDelay(1);  // Minimal delay für Watchdog
    }
}

void ButtonHandler::begin() {
    keypad->begin();
    rfReceiver->begin();
    
    // Queue für Tastenübertragung erstellen (max 10 Tasten puffern)
    keyQueue = xQueueCreate(10, sizeof(int));
    
    // Keypad-Task auf Core 0 starten (Core 1 = Arduino loop)
    xTaskCreatePinnedToCore(
        keypadTask,           // Task-Funktion
        "KeypadTask",         // Name
        4096,                 // Stack-Größe
        keypad,               // Parameter (Keypad-Objekt)
        2,                    // Priorität (höher = wichtiger)
        &keypadTaskHandle,    // Task-Handle
        0                     // Core 0 (nicht Core 1 wo loop() läuft)
    );
    
    Serial.println("✓ Button Handler initialisiert (Keypad auf Core 0, RF auf Core 1)");
}

void ButtonHandler::loop() {
    int key = -1;
    
    // Keypad: Taste aus Queue lesen (non-blocking)
    if (xQueueReceive(keyQueue, &key, 0) == pdTRUE) {
        // Taste aus Queue erhalten
    }
    
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
