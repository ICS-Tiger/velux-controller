#include "button_handler.h"
#include "config.h"

// ===== LedFeedback (Non-blocking LED-Steuerung) =====

LedFeedback::LedFeedback(uint8_t ledPin, bool isActiveHigh) {
    pin = ledPin;
    activeHigh = isActiveHigh;
    state = LED_IDLE;
    startTime = 0;
    blinkCount = LED_ERROR_BLINK_COUNT;
    currentBlink = 0;
    ledOn = false;
}

void LedFeedback::begin() {
    pinMode(pin, OUTPUT);
    setLed(false);
    Serial.printf("✓ LED-Feedback initialisiert auf GPIO %d\n", pin);
}

void LedFeedback::setLed(bool on) {
    ledOn = on;
    if (activeHigh) {
        digitalWrite(pin, on ? HIGH : LOW);
    } else {
        digitalWrite(pin, on ? LOW : HIGH);
    }
}

void LedFeedback::showOK() {
    state = LED_OK;
    startTime = millis();
    setLed(true);
    if (Serial) Serial.println("LED: OK (1s an)");
}

void LedFeedback::showError() {
    state = LED_ERROR_BLINK;
    startTime = millis();
    currentBlink = 0;
    blinkCount = LED_ERROR_BLINK_COUNT;
    setLed(true);
    if (Serial) Serial.println("LED: Fehler (3x blinken)");
}

bool LedFeedback::isBusy() {
    return state != LED_IDLE;
}

void LedFeedback::loop() {
    if (state == LED_IDLE) return;
    
    unsigned long elapsed = millis() - startTime;
    
    if (state == LED_OK) {
        // LED für LED_OK_DURATION_MS an
        if (elapsed >= LED_OK_DURATION_MS) {
            setLed(false);
            state = LED_IDLE;
        }
    }
    else if (state == LED_ERROR_BLINK) {
        // Blink-Zyklus: LED_ERROR_BLINK_MS an, LED_ERROR_BLINK_MS aus
        int cycleTime = LED_ERROR_BLINK_MS * 2;  // Ein kompletter An/Aus-Zyklus
        int cyclePos = elapsed % cycleTime;
        
        bool shouldBeOn = (cyclePos < LED_ERROR_BLINK_MS);
        
        if (shouldBeOn != ledOn) {
            setLed(shouldBeOn);
        }
        
        // Nach blinkCount Zyklen beenden
        if (elapsed >= (unsigned long)(blinkCount * cycleTime)) {
            setLed(false);
            state = LED_IDLE;
        }
    }
}

// ===== AnalogKeypad (Verbesserte Mehrfach-Messung mit früher Erkennung - NON-BLOCKING) =====

AnalogKeypad::AnalogKeypad(uint8_t adcPin) {
    pin = adcPin;
    measuring = false;
    anzahlTasten = 0;
    anzahlMessungen = 0;
    unterSchwellwert = 0;
    summe = 0;
    lastReadTime = 0;
    debugOutput = true;
    
    // Neue Member initialisieren
    locked = false;
    lockCounter = 0;
    erkanntesTaste = -1;
    earlyDetected = false;
    lastResult = KEYPAD_NO_KEY;
    
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
    float guete;
    return berechneTasteMitGuete(&guete);
}

int AnalogKeypad::berechneTasteMitGuete(float* gueteOut) {
    *gueteOut = 0.0;
    
    // Mindestens ein paar Messungen erforderlich
    if (anzahlMessungen < 3) {
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
    *gueteOut = guete;
    
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
    
    // Debug-Ausgabe
    if (debugOutput) {
        Serial.print("Keypad: Messungen: ");
        Serial.print(anzahlMessungen);
        Serial.print(" | Wert: ");
        Serial.print(mittelwertBereinigt);
        Serial.print(" | Güte: ");
        Serial.print(guete, 1);
        Serial.print("% | Taste: ");
        Serial.println(naechsteTaste);
    }
    
    return naechsteTaste;
}

int AnalogKeypad::loop() {
    unsigned long now = millis();
    
    // Non-blocking: Nur alle KEYPAD_MESS_INTERVAL ms messen
    if (now - lastReadTime < KEYPAD_MESS_INTERVAL) {
        return KEYPAD_NO_KEY;
    }
    lastReadTime = now;
    
    int currentValue = analogRead(pin);
    
    // === Sperr-Modus nach früher Erkennung ===
    if (locked) {
        if (currentValue > KEYPAD_THRESHOLD) {
            lockCounter++;
            // Nach KEYPAD_LOCK_MAX Messungen entsperren
            if (lockCounter >= KEYPAD_LOCK_MAX) {
                locked = false;
                if (debugOutput) Serial.println("Keypad: Sperre aufgehoben (max)");
            }
            return KEYPAD_LOCKED;
        } else {
            // Taste losgelassen während Sperre
            unterSchwellwert++;
            if (unterSchwellwert >= KEYPAD_RELEASE_COUNT && lockCounter >= KEYPAD_LOCK_MIN) {
                // Mindest-Sperrzeit erreicht und Taste losgelassen
                locked = false;
                if (debugOutput) Serial.println("Keypad: Sperre aufgehoben (losgelassen)");
                unterSchwellwert = 0;
            }
            return KEYPAD_LOCKED;
        }
    }
    
    // === Normale Messung ===
    
    // Taste wurde gedrückt (über Schwellwert)
    if (currentValue > KEYPAD_THRESHOLD) {
        if (!measuring) {
            // Neue Messung starten
            measuring = true;
            anzahlMessungen = 0;
            unterSchwellwert = 0;
            summe = 0;
            earlyDetected = false;
            erkanntesTaste = -1;
        }
        
        // Messwert aufnehmen
        if (anzahlMessungen < KEYPAD_MAX_MESSUNGEN) {
            messwerte[anzahlMessungen] = currentValue;
            summe += currentValue;
            anzahlMessungen++;
        }
        unterSchwellwert = 0;  // Reset Counter
        
        // === Frühe Prüfung nach KEYPAD_EARLY_CHECK_COUNT Messungen ===
        if (!earlyDetected && anzahlMessungen >= KEYPAD_EARLY_CHECK_COUNT) {
            float guete = 0;
            int taste = berechneTasteMitGuete(&guete);
            
            if (taste >= 0 && guete >= KEYPAD_EARLY_GUETE) {
                // Frühe Erkennung erfolgreich!
                earlyDetected = true;
                erkanntesTaste = taste;
                locked = true;
                lockCounter = 0;
                
                if (debugOutput) {
                    Serial.print("Keypad: Frühe Erkennung! Taste ");
                    Serial.print(taste);
                    Serial.print(" mit Güte ");
                    Serial.print(guete, 1);
                    Serial.println("%");
                }
                
                // Reset für nächste Messung
                measuring = false;
                anzahlMessungen = 0;
                summe = 0;
                lastResult = KEYPAD_NO_KEY;  // OK-Ergebnis
                
                return taste;  // Taste zurückgeben
            }
        }
        
        // === Fehlerprüfung nach KEYPAD_MAX_ATTEMPTS Messungen ===
        if (anzahlMessungen >= KEYPAD_MAX_ATTEMPTS) {
            float guete = 0;
            int taste = berechneTasteMitGuete(&guete);
            
            if (taste >= 0 && guete >= KEYPAD_FINAL_GUETE) {
                // Späte Erkennung noch OK
                if (debugOutput) {
                    Serial.print("Keypad: Späte Erkennung! Taste ");
                    Serial.print(taste);
                    Serial.print(" mit Güte ");
                    Serial.print(guete, 1);
                    Serial.println("%");
                }
                
                locked = true;
                lockCounter = 0;
                measuring = false;
                anzahlMessungen = 0;
                summe = 0;
                lastResult = KEYPAD_NO_KEY;
                
                return taste;
            } else {
                // Fehler - Güte zu schlecht
                if (debugOutput) {
                    Serial.print("Keypad: FEHLER nach ");
                    Serial.print(anzahlMessungen);
                    Serial.print(" Messungen. Güte: ");
                    Serial.print(guete, 1);
                    Serial.println("%");
                }
                
                measuring = false;
                anzahlMessungen = 0;
                summe = 0;
                lastResult = KEYPAD_ERROR;
                
                return KEYPAD_ERROR;  // Fehler-Signal
            }
        }
        
        return KEYPAD_MEASURING;  // Noch in Messung
    }
    else {
        // Unter Schwellwert
        if (measuring) {
            unterSchwellwert++;
            
            // Taste losgelassen? (KEYPAD_RELEASE_COUNT mal unter Schwellwert)
            if (unterSchwellwert >= KEYPAD_RELEASE_COUNT) {
                measuring = false;
                
                // Taste berechnen (wenn noch nicht früh erkannt)
                if (!earlyDetected && anzahlMessungen >= KEYPAD_MIN_MESSUNGEN) {
                    float guete = 0;
                    int taste = berechneTasteMitGuete(&guete);
                    
                    // Reset für nächste Messung
                    anzahlMessungen = 0;
                    unterSchwellwert = 0;
                    summe = 0;
                    
                    if (taste >= 0 && guete >= KEYPAD_MIN_GUETE) {
                        lastResult = KEYPAD_NO_KEY;
                        return taste;
                    } else {
                        lastResult = KEYPAD_ERROR;
                        return KEYPAD_ERROR;
                    }
                }
                
                // Reset für nächste Messung
                anzahlMessungen = 0;
                unterSchwellwert = 0;
                summe = 0;
            }
        }
    }
    
    return KEYPAD_NO_KEY;
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
QueueHandle_t ButtonHandler::ledQueue = nullptr;
ButtonHandler* ButtonHandler::instance = nullptr;

ButtonHandler::ButtonHandler() {
    keypad = new AnalogKeypad(KEYPAD_PIN);
    rfReceiver = new RFReceiver();
    ledFeedback = new LedFeedback(LED_FEEDBACK_PIN, LED_ACTIVE_HIGH);
    instance = this;
}

// Keypad-Task läuft auf Core 0 (unabhängig vom Hauptloop)
void ButtonHandler::keypadTask(void* parameter) {
    AnalogKeypad* kp = (AnalogKeypad*)parameter;
    
    Serial.println("Keypad-Task gestartet auf Core 0");
    
    for (;;) {
        int key = kp->loop();
        
        // Gültige Taste erkannt (>= 0)
        if (key >= 0) {
            // Taste in Queue senden
            xQueueSend(keyQueue, &key, 0);
            
            // LED-OK Signal senden
            int ledCmd = 1;  // 1 = OK
            xQueueSend(ledQueue, &ledCmd, 0);
        }
        // Fehler erkannt
        else if (key == KEYPAD_ERROR) {
            // LED-Fehler Signal senden
            int ledCmd = 2;  // 2 = Error
            xQueueSend(ledQueue, &ledCmd, 0);
        }
        // KEYPAD_MEASURING, KEYPAD_LOCKED, KEYPAD_NO_KEY ignorieren
        
        vTaskDelay(1);  // Minimal delay für Watchdog
    }
}

void ButtonHandler::begin() {
    keypad->begin();
    rfReceiver->begin();
    ledFeedback->begin();
    
    // Queue für Tastenübertragung erstellen (max 10 Tasten puffern)
    keyQueue = xQueueCreate(10, sizeof(int));
    
    // Queue für LED-Befehle erstellen
    ledQueue = xQueueCreate(5, sizeof(int));
    
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
    
    Serial.println("✓ Button Handler initialisiert (Keypad auf Core 0, RF auf Core 1, LED-Feedback aktiv)");
}

void ButtonHandler::loop() {
    // LED-Feedback verarbeiten (non-blocking)
    ledFeedback->loop();
    
    // LED-Befehle aus Queue verarbeiten
    int ledCmd = 0;
    if (xQueueReceive(ledQueue, &ledCmd, 0) == pdTRUE) {
        if (ledCmd == 1) {
            ledFeedback->showOK();
        } else if (ledCmd == 2) {
            ledFeedback->showError();
        }
    }
    
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
