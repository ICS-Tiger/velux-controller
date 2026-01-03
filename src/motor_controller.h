#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include <Preferences.h>
#include <Adafruit_INA219.h>

enum MotorState {
    STOPPED,
    OPENING,
    CLOSING,
    LEARNING_OPEN,
    LEARNING_CLOSE
};

enum MotorDirection {
    DIR_STOP,
    DIR_OPEN,
    DIR_CLOSE
};

class MotorController {
private:
    uint8_t id;
    uint8_t pinREN;
    uint8_t pinLEN;
    uint8_t inaAddress;
    
    Adafruit_INA219* ina219;
    
    MotorState state;
    MotorDirection currentDirection;
    uint8_t currentPosition;
    uint8_t targetPosition;
    
    unsigned long openTime;
    unsigned long closeTime;
    
    unsigned long moveStartTime;
    unsigned long lastPositionUpdate;
    unsigned long lastCurrentCheck;
    unsigned long overcurrentStartTime;
    
    bool isCalibrated;
    bool overcurrentDetected;
    float currentCurrent_mA;
    
    Preferences prefs;
    
    void updatePosition();
    void applyMotorControl(MotorDirection dir);
    void checkCurrent();
    
public:
    MotorController(uint8_t motorId, uint8_t rEN, uint8_t lEN, uint8_t inaAddr);
    
    void begin();
    void loop();
    
    void moveToPosition(uint8_t position);
    void open();
    void close();
    void stop();
    
    void startLearnOpen();
    void startLearnClose();
    void finishLearn();
    void cancelLearn();
    
    uint8_t getPosition() { return currentPosition; }
    MotorState getState() { return state; }
    MotorDirection getDirection() { return currentDirection; }
    bool getCalibrated() { return isCalibrated; }
    unsigned long getOpenTime() { return openTime; }
    unsigned long getCloseTime() { return closeTime; }
    bool isMoving() { return state != STOPPED; }
    float getCurrent() { return currentCurrent_mA; }
    bool hasOvercurrent() { return overcurrentDetected; }
    
    void setPosition(uint8_t pos) { currentPosition = pos; }
    
    void saveConfig();
    void loadConfig();
    void resetConfig();
};

class PWMController {
private:
    static uint8_t activeMotorsOpen;
    static uint8_t activeMotorsClose;
    static uint8_t currentPWM;
    
    static bool softStartActive;
    static unsigned long softStartBegin;
    static unsigned long lastPWMUpdate;
    
    static void updateSoftStart();
    
public:
    static void begin();
    static void loop();
    
    static void motorStarted(MotorDirection dir);
    static void motorStopped(MotorDirection dir);
    
    static void setPWM(MotorDirection dir, uint8_t pwm);
    static uint8_t getCurrentPWM() { return currentPWM; }
    
    static bool isSoftStartActive() { return softStartActive; }
    static void resetSoftStart();
    
    static uint8_t getActiveMotorCount() { return activeMotorsOpen + activeMotorsClose; }
    static bool hasConflict() { return (activeMotorsOpen > 0 && activeMotorsClose > 0); }
};

#endif
