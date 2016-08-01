#include <Arduino.h>
#include "door.h"

#define RELAY_LOCK HIGH
#define RELAY_UNLOCK LOW

void DoorSensor::setPin( uint8_t sp, uint8_t rp ){
  sensor_pin = sp;
  relay_pin = rp;

  sensor_state = DOOR_SENSOR_STATE_UNKNOWN;
  pinMode(sensor_pin, INPUT_PULLUP);
  pinMode(relay_pin, OUTPUT);

  digitalWrite(relay_pin, RELAY_LOCK);
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
  Serial.print("LOCK REQUEST. Setting Pin ");
  Serial.print(relay_pin);
  Serial.print(" to logic state ");
  Serial.println(RELAY_LOCK);
  digitalWrite(relay_pin, RELAY_LOCK);
}

void DoorSensor::unlock( )
{
  relay_lock_state = false;
  Serial.print("UNLOCK REQUEST. Setting Pin ");
  Serial.print(relay_pin);
  Serial.print(" to logic state ");
  Serial.println(RELAY_UNLOCK);
  digitalWrite(relay_pin, RELAY_UNLOCK);
}

void DoorSensor::poll()
{
  sensor_reading = analogRead(sensor_pin);
  if ( sensor_reading >= 0 && sensor_reading <= 110 ) sensor_state = DOOR_SENSOR_STATE_SHORT;
  if ( sensor_reading >= 110 && sensor_reading <= 400 ) sensor_state = DOOR_SENSOR_STATE_NORMAL;
  if ( sensor_reading >= 400 && sensor_reading <= 500 ) sensor_state = DOOR_SENSOR_STATE_TAMPER;
  if ( sensor_reading >= 500 && sensor_reading <= 799 ) sensor_state = DOOR_SENSOR_STATE_ALARM;
  if ( sensor_reading >= 800 ) sensor_state = DOOR_SENSOR_STATE_CUT;
}
