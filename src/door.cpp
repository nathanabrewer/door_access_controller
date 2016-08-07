#include <Arduino.h>
#include "door.h"
#include "config.h"


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
  Serial.print(F("LOCK REQUEST. Setting Pin "));
  Serial.print(relay_pin);
  Serial.print(F(" to logic state "));
  Serial.println(RELAY_LOCK);
  digitalWrite(relay_pin, RELAY_LOCK);
}

void DoorSensor::unlock( )
{
  relay_lock_state = false;
  Serial.print(F("UNLOCK REQUEST. Setting Pin "));
  Serial.print(relay_pin);
  Serial.print(F(" to logic state "));
  Serial.println(RELAY_UNLOCK);
  digitalWrite(relay_pin, RELAY_UNLOCK);
}

void DoorSensor::grantAccess( )
{
  if(relay_lock_state == false){
    Serial.println(F("grantAccess refused. relay lock state is false."));
    return;
  }
  if(granting_access == true){
    Serial.println(F("grantAccess refused. busy. already granting access"));
    return;
  }

  Serial.print(F("Granting Access to "));
  Serial.print(relay_pin);
  grant_access_time = millis();
  granting_access = true;

  //unlock(); we want to manually override the relay state. Then if the schedule actually unlocks this door at the same time, we can simply upgrade its state to unlocked as normal
  digitalWrite(relay_pin, RELAY_UNLOCK);
}

void DoorSensor::poll()
{
  if(granting_access){
    if(millis() - grant_access_time > GRANT_ACCESS_RELAY_TIME){
      granting_access = false;
      if(relay_lock_state){
        Serial.println(F("Grant Access Ended. Returning door to Locked State"));
        digitalWrite(relay_pin, RELAY_UNLOCK);
      }else{
        Serial.println(F("Grant Access Ended. Door state was previously changed to unlock. No action taken."));
      }

    }
  }

  sensor_reading = analogRead(sensor_pin);
  if ( sensor_reading >= 0 && sensor_reading <= 110 ) sensor_state = DOOR_SENSOR_STATE_SHORT;
  if ( sensor_reading >= 110 && sensor_reading <= 400 ) sensor_state = DOOR_SENSOR_STATE_NORMAL;
  if ( sensor_reading >= 400 && sensor_reading <= 500 ) sensor_state = DOOR_SENSOR_STATE_TAMPER;
  if ( sensor_reading >= 500 && sensor_reading <= 799 ) sensor_state = DOOR_SENSOR_STATE_ALARM;
  if ( sensor_reading >= 800 ) sensor_state = DOOR_SENSOR_STATE_CUT;
}
