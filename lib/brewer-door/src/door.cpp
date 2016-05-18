#include <Arduino.h>
#include "door.h"

void DoorSensor::setPin( uint8_t pin ){
  sensor_pin = pin;
  sensor_state = DOOR_SENSOR_STATE_UNKNOWN;
  sensor_last_state = DOOR_SENSOR_STATE_UNKNOWN;
  pinMode(sensor_pin, INPUT_PULLUP);
}

int DoorSensor::getValue(){
  return sensor_reading;
}
int DoorSensor::getState(){
  return sensor_state;
}

void DoorSensor::poll( )
{

  sensor_reading = analogRead(sensor_pin);
  if ( sensor_reading >= 0 && sensor_reading <= 110 ) sensor_state = DOOR_SENSOR_STATE_SHORT;
  if ( sensor_reading >= 110 && sensor_reading <= 400 ) sensor_state = DOOR_SENSOR_STATE_NORMAL;
  if ( sensor_reading >= 400 && sensor_reading <= 500 ) sensor_state = DOOR_SENSOR_STATE_TAMPER;
  if ( sensor_reading >= 500 && sensor_reading <= 799 ) sensor_state = DOOR_SENSOR_STATE_ALARM;
  if ( sensor_reading >= 800 ) sensor_state = DOOR_SENSOR_STATE_CUT;

  // Compare to previous value
  if ( sensor_state != sensor_last_state )
  {
    Serial.print("Sensor changed from ");
    Serial.print(sensor_last_state);
    Serial.print(" to ");
    Serial.println(sensor_state);
    sensor_last_state = sensor_state;
  }

}
