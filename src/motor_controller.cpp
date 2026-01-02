#include "motor_controller.h"
#include "config.h"

uint8_t PWMController::activeMotorsOpen = 0;
uint8_t PWMController::activeMotorsClose = 0;
uint8_t PWMController::currentPWM = 0;
bool PWMController::softStartActive = false;
unsigned long PWMController::softStartBegin = 0;
unsigned long PWMController::lastPWMUpdate = 0;

void PWMController::begin() {
    ledcSetup(RPWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(LPWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(RPWM_ALL, RPWM_CHANNEL);
    ledcAttachPin(LPWM_ALL, LPWM_CHANNEL);
    
    ledcWrite(RPWM_CHANNEL, 0);
    ledcWrite(LPWM_CHANNEL, 0);
    
    Serial.println("PWM-Controller: Initialisiert (gemeinsame PWM mit Sanftanlauf)");
}

void PWMController::loop() {
    if (!softStartActive) return;
    
    unsigned long now = millis();
    
    if (now - lastPWMUpdate >= SOFT_START_STEP_INTERVAL) {
        updateSoftStart();
        lastPWMUpdate = now;
    }
}

void PWMController::updateSoftStart() {
    if (!softStartActive) return;
    
    unsigned long elapsed = millis() - softStartBegin;
    
    if (elapsed >= SOFT_START_DURATION_MS) {
        softStartActive = false;
        currentPWM = SOFT_START_MAX_PWM;
        Serial.printf("PWM-Controller: Sanftanlauf beendet (PWM=%d)\n", currentPWM);
    } else {
        float progress = (float)elapsed / SOFT_START_DURATION_MS;
        currentPWM = SOFT_START_MIN_PWM + (progress * (SOFT_START_MAX_PWM - SOFT_START_MIN_PWM));
    }
    
    if (activeMotorsOpen > 0 && activeMotorsClose == 0) {
        ledcWrite(RPWM_CHANNEL, currentPWM);
        ledcWrite(LPWM_CHANNEL, 0);
    } else if (activeMotorsClose > 0 && activeMotorsOpen == 0) {
        ledcWrite(RPWM_CHANNEL, 0);
        ledcWrite(LPWM_CHANNEL, currentPWM);
    } else if (activeMotorsOpen > 0 && activeMotorsClose > 0) {
        Serial.println("PWM-Controller: KONFLIKT! Verschiedene Richtungen!");
        ledcWrite(RPWM_CHANNEL, 0);
        ledcWrite(LPWM_CHANNEL, 0);
        currentPWM = 0;
    } else {
        ledcWrite(RPWM_CHANNEL, 0);
        ledcWrite(LPWM_CHANNEL, 0);
        currentPWM = 0;
    }
}

void PWMController::motorStarted(MotorDirection dir) {
    if (dir == DIR_OPEN) {
        activeMotorsOpen++;
    } else if (dir == DIR_CLOSE) {
        activeMotorsClose++;
    }
    
    if (getActiveMotorCount() == 1 && SOFT_START_ENABLED) {
        softStartActive = true;
        softStartBegin = millis();
        lastPWMUpdate = softStartBegin;
        currentPWM = SOFT_START_MIN_PWM;
        Serial.printf("PWM-Controller: Sanftanlauf gestartet (%d->%d über %dms)\n", 
                     SOFT_START_MIN_PWM, SOFT_START_MAX_PWM, SOFT_START_DURATION_MS);
    } else if (!softStartActive) {
        currentPWM = SOFT_START_MAX_PWM;
    }
    
    updateSoftStart();
}

void PWMController::motorStopped(MotorDirection dir) {
    if (dir == DIR_OPEN && activeMotorsOpen > 0) {
        activeMotorsOpen--;
    } else if (dir == DIR_CLOSE && activeMotorsClose > 0) {
        activeMotorsClose--;
    }
    
    if (getActiveMotorCount() == 0) {
        ledcWrite(RPWM_CHANNEL, 0);
        ledcWrite(LPWM_CHANNEL, 0);
        currentPWM = 0;
        softStartActive = false;
    } else {
        updateSoftStart();
    }
}

void PWMController::setPWM(MotorDirection dir, uint8_t pwm) {
    currentPWM = pwm;
    
    if (dir == DIR_OPEN) {
        ledcWrite(RPWM_CHANNEL, pwm);
        ledcWrite(LPWM_CHANNEL, 0);
    } else if (dir == DIR_CLOSE) {
        ledcWrite(RPWM_CHANNEL, 0);
        ledcWrite(LPWM_CHANNEL, pwm);
    } else {
        ledcWrite(RPWM_CHANNEL, 0);
        ledcWrite(LPWM_CHANNEL, 0);
    }
}

void PWMController::resetSoftStart() {
    softStartActive = false;
    currentPWM = SOFT_START_MAX_PWM;
}

// ===== MotorController =====

MotorController::MotorController(uint8_t motorId, uint8_t ren, uint8_t len, uint8_t i2cAddr) {
    id = motorId;
    pinREN = ren;
    pinLEN = len;
    
    state = STOPPED;
    currentDirection = DIR_STOP;
    currentPosition = 0;
    targetPosition = 0;
    
    openTime = 0;
    closeTime = 0;
    
    moveStartTime = 0;
    lastPositionUpdate = 0;
    
    currentSensor = new Adafruit_INA219(i2cAddr);
    maxCurrent = 0;
    currentThreshold = 3000;
    
    isCalibrated = false;
}

void MotorController::begin() {
    pinMode(pinREN, OUTPUT);
    pinMode(pinLEN, OUTPUT);
    digitalWrite(pinREN, LOW);
    digitalWrite(pinLEN, LOW);
    
    if (!currentSensor->begin()) {
        Serial.printf("Motor %d: INA219 nicht gefunden!\n", id);
    } else {
        currentSensor->setCalibration_32V_2A();
        Serial.printf("Motor %d: INA219 initialisiert\n", id);
    }
    
    loadConfig();
}

void MotorController::loop() {
    if (state == STOPPED) return;
    
    unsigned long now = millis();
    
    if (now - lastPositionUpdate >= POSITION_UPDATE_INTERVAL) {
        updatePosition();
        lastPositionUpdate = now;
    }
    
    checkCurrentLimit();
    
    if (state == OPENING || state == CLOSING) {
        if (currentPosition == targetPosition) {
            stop();
        }
    }
    
    if (now - moveStartTime > MAX_RUNTIME_MS) {
        Serial.printf("Motor %d: Maximale Laufzeit überschritten!\n", id);
        stop();
    }
}

void MotorController::updatePosition() {
    if (state == STOPPED) return;
    
    unsigned long elapsed = millis() - moveStartTime;
    unsigned long totalTime = (state == OPENING || state == LEARNING_OPEN) ? openTime : closeTime;
    
    if (totalTime == 0) return;
    
    if (state == OPENING || state == LEARNING_OPEN) {
        currentPosition = min((uint8_t)100, (uint8_t)((elapsed * 100) / totalTime));
    } else if (state == CLOSING || state == LEARNING_CLOSE) {
        currentPosition = max((uint8_t)0, (uint8_t)(100 - ((elapsed * 100) / totalTime)));
    }
}

void MotorController::checkCurrentLimit() {
    float current = getCurrent();
    
    if (current > maxCurrent) {
        maxCurrent = current;
    }
    
    if (current > currentThreshold) {
        if (state == LEARNING_OPEN || state == LEARNING_CLOSE) {
            finishLearn();
        } else {
            Serial.printf("Motor %d: Stromgrenze erreicht (%.2fmA)\n", id, current);
            stop();
        }
    }
}

float MotorController::getCurrent() {
    return currentSensor->getCurrent_mA();
}

void MotorController::applyMotorControl(MotorDirection dir) {
    switch(dir) {
        case DIR_OPEN:
            digitalWrite(pinREN, HIGH);
            digitalWrite(pinLEN, LOW);
            break;
        case DIR_CLOSE:
            digitalWrite(pinLEN, HIGH);
            digitalWrite(pinREN, LOW);
            break;
        case DIR_STOP:
        default:
            digitalWrite(pinREN, LOW);
            digitalWrite(pinLEN, LOW);
            break;
    }
}

void MotorController::moveToPosition(uint8_t position) {
    if (!isCalibrated) {
        Serial.printf("Motor %d: Nicht kalibriert!\n", id);
        return;
    }
    
    targetPosition = constrain(position, 0, 100);
    
    if (targetPosition > currentPosition) {
        open();
    } else if (targetPosition < currentPosition) {
        close();
    }
}

void MotorController::open() {
    if (PWMController::hasConflict()) {
        Serial.printf("Motor %d: Konflikt - andere Richtung aktiv!\n", id);
        return;
    }
    
    Serial.printf("Motor %d: Öffne auf %d%%\n", id, targetPosition);
    
    state = OPENING;
    currentDirection = DIR_OPEN;
    moveStartTime = millis();
    lastPositionUpdate = moveStartTime;
    maxCurrent = 0;
    
    applyMotorControl(DIR_OPEN);
    PWMController::motorStarted(DIR_OPEN);
}

void MotorController::close() {
    if (PWMController::hasConflict()) {
        Serial.printf("Motor %d: Konflikt - andere Richtung aktiv!\n", id);
        return;
    }
    
    Serial.printf("Motor %d: Schließe auf %d%%\n", id, targetPosition);
    
    state = CLOSING;
    currentDirection = DIR_CLOSE;
    moveStartTime = millis();
    lastPositionUpdate = moveStartTime;
    maxCurrent = 0;
    
    applyMotorControl(DIR_CLOSE);
    PWMController::motorStarted(DIR_CLOSE);
}

void MotorController::stop() {
    Serial.printf("Motor %d: Stoppe bei %d%%\n", id, currentPosition);
    
    MotorDirection oldDirection = currentDirection;
    
    state = STOPPED;
    currentDirection = DIR_STOP;
    
    applyMotorControl(DIR_STOP);
    PWMController::motorStopped(oldDirection);
}

void MotorController::startLearnOpen() {
    Serial.printf("Motor %d: Lerne Öffnungszeit\n", id);
    
    state = LEARNING_OPEN;
    currentDirection = DIR_OPEN;
    currentPosition = 0;
    targetPosition = 100;
    moveStartTime = millis();
    maxCurrent = 0;
    
    applyMotorControl(DIR_OPEN);
    PWMController::motorStarted(DIR_OPEN);
    PWMController::resetSoftStart();
}

void MotorController::startLearnClose() {
    Serial.printf("Motor %d: Lerne Schließzeit\n", id);
    
    state = LEARNING_CLOSE;
    currentDirection = DIR_CLOSE;
    currentPosition = 100;
    targetPosition = 0;
    moveStartTime = millis();
    maxCurrent = 0;
    
    applyMotorControl(DIR_CLOSE);
    PWMController::motorStarted(DIR_CLOSE);
    PWMController::resetSoftStart();
}

void MotorController::finishLearn() {
    unsigned long learnTime = millis() - moveStartTime;
    
    if (state == LEARNING_OPEN) {
        openTime = learnTime;
        currentPosition = 100;
        Serial.printf("Motor %d: Öffnungszeit: %lums (%.1fs)\n", id, openTime, openTime/1000.0);
    } else if (state == LEARNING_CLOSE) {
        closeTime = learnTime;
        currentPosition = 0;
        Serial.printf("Motor %d: Schließzeit: %lums (%.1fs)\n", id, closeTime, closeTime/1000.0);
    }
    
    isCalibrated = (openTime > 0 && closeTime > 0);
    
    if (isCalibrated) {
        saveConfig();
    }
    
    stop();
}

void MotorController::cancelLearn() {
    Serial.printf("Motor %d: Anlernen abgebrochen\n", id);
    stop();
}

void MotorController::saveConfig() {
    prefs.begin(PREFS_NAMESPACE, false);
    
    String prefix = "m" + String(id) + "_";
    prefs.putULong((prefix + "open").c_str(), openTime);
    prefs.putULong((prefix + "close").c_str(), closeTime);
    prefs.putUChar((prefix + "pos").c_str(), currentPosition);
    prefs.putBool((prefix + "cal").c_str(), isCalibrated);
    
    prefs.end();
    
    Serial.printf("Motor %d: Konfiguration gespeichert\n", id);
}

void MotorController::loadConfig() {
    prefs.begin(PREFS_NAMESPACE, true);
    
    String prefix = "m" + String(id) + "_";
    openTime = prefs.getULong((prefix + "open").c_str(), 0);
    closeTime = prefs.getULong((prefix + "close").c_str(), 0);
    currentPosition = prefs.getUChar((prefix + "pos").c_str(), 0);
    isCalibrated = prefs.getBool((prefix + "cal").c_str(), false);
    
    prefs.end();
    
    Serial.printf("Motor %d: Config geladen (Open:%lums Close:%lums Pos:%d%% Cal:%d)\n",
                 id, openTime, closeTime, currentPosition, isCalibrated);
}

void MotorController::resetConfig() {
    openTime = 0;
    closeTime = 0;
    currentPosition = 0;
    isCalibrated = false;
    
    saveConfig();
}
