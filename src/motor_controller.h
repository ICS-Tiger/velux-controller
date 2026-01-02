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
    
    MotorState state;
    MotorDirection currentDirection;
    uint8_t currentPosition;
    uint8_t targetPosition;
    
    unsigned long openTime;
    unsigned long closeTime;
    
    unsigned long moveStartTime;
    unsigned long lastPositionUpdate;
    
    Adafruit_INA219 *currentSensor;
    float maxCurrent;
    float currentThreshold;
    
    bool isCalibrated;
    Preferences prefs;
    
    void updatePosition();
    void checkCurrentLimit();
    void applyMotorControl(MotorDirection dir);
    
public:
    MotorController(uint8_t motorId, uint8_t ren, uint8_t len, uint8_t i2cAddr);
    
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
    float getCurrent();
    bool isMoving() { return state != STOPPED; }
    
    void setPosition(uint8_t pos) { currentPosition = pos; }
    void setCurrentThreshold(float threshold) { currentThreshold = threshold; }
    
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
