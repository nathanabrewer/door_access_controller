#include <Arduino.h>
#include "door.h"

void DoorSensor::setPin( uint8_t sp, uint8_t rp ){
  sensor_pin = sp;
  relay_pin = rp;

  sensor_state = DOOR_SENSOR_STATE_UNKNOWN;
  sensor_last_state = DOOR_SENSOR_STATE_UNKNOWN;
  pinMode(sensor_pin, INPUT_PULLUP);
  //pinMode(sensor_pin, INPUT);

  pinMode(relay_pin, OUTPUT);

  digitalWrite(relay_pin, HIGH);
}

int DoorSensor::getValue(){
  return sensor_reading;
}
int DoorSensor::getState(){
  return sensor_state;
}
int DoorSensor::getRelayState(){
  return relay_lock_state;
}
void DoorSensor::lock( )
{
  relay_lock_state = true;
  digitalWrite(relay_pin, HIGH);
}
void DoorSensor::unlock( )
{
  relay_lock_state = false;
  digitalWrite(relay_pin, LOW);
}

void DoorSensor::poll()
{
  sensor_reading = analogRead(sensor_pin);
  if ( sensor_reading >= 0 && sensor_reading <= 110 ) sensor_state = DOOR_SENSOR_STATE_SHORT;
  if ( sensor_reading >= 110 && sensor_reading <= 400 ) sensor_state = DOOR_SENSOR_STATE_NORMAL;
  if ( sensor_reading >= 400 && sensor_reading <= 500 ) sensor_state = DOOR_SENSOR_STATE_TAMPER;
  if ( sensor_reading >= 500 && sensor_reading <= 799 ) sensor_state = DOOR_SENSOR_STATE_ALARM;
  if ( sensor_reading >= 800 ) sensor_state = DOOR_SENSOR_STATE_CUT;

  // Compare to previous value
  // if ( sensor_state != sensor_last_state )
  // {
  //   sensor_state_change(id, sensor_last_state, sensor_state);
  //   sensor_last_state = sensor_state;
  // }

}
