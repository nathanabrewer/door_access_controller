#include <Arduino.h>
#include "keypad.h"
#include <Wiegand.h>

Keypad::Keypad(){
  wg.begin();
}

// int DoorSensor::getValue(){
//   return _SENSOR_READING;
// }
// int DoorSensor::getState(){
//   return _SENSOR_STATE;
// }

void Keypad::poll( )
{
  if(wg.available())
	{
		Serial.print("Wiegand HEX = ");
		Serial.print(wg.getCode(),HEX);
		Serial.print(", DECIMAL = ");
		Serial.print(wg.getCode());
		Serial.print(", Type W");
		Serial.println(wg.getWiegandType());
	}
}
