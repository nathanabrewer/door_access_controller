#include <Arduino.h>

const int DOOR_SENSOR_STATE_SHORT = 0;
const int DOOR_SENSOR_STATE_NORMAL = 1;
const int DOOR_SENSOR_STATE_TAMPER =2;
const int DOOR_SENSOR_STATE_ALARM = 3;
const int DOOR_SENSOR_STATE_ALARM_TAMPER = 4;
const int DOOR_SENSOR_STATE_CUT = 5;
const int DOOR_SENSOR_STATE_UNKNOWN = 6;

const int RELAY_UNLOCKED = 1;
const int RELAY_LOCKED = 0;

const int DEVICE_STATUS_IDLE = 1;
const int DEVICE_STATUS_INGRESS = 2;
const int DEVICE_STATUS_EGRESS = 3;
const int DEVICE_STATUS_BAD_AUTH = 4;


class DoorSensor{
private:
    uint8_t sensor_pin;
    uint8_t relay_pin;
    int sensor_state;
    int sensor_last_state;
    int sensor_reading;
    bool relay_lock_state;

public:
    void setPin(uint8_t sensor_pin, uint8_t relay_pin);
    void poll();
    void lock();
    void unlock();
    int getState();
    int getValue();

};
