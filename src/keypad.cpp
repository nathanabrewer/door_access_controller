#include <Arduino.h>
#include "keypad.h"
#include <Wiegand.h>

#include <avr/pgmspace.h>

static uint8_t wgmsg[10];
static uint8_t *wg_msg_ptr;

Keypad::Keypad(){
  wg.begin();
  wg_msg_ptr = wgmsg;
}

// int DoorSensor::getValue(){
//   return _SENSOR_READING;
// }
// int DoorSensor::getState(){
//   return _SENSOR_STATE;
// }

//YUCK, but it works
unsigned long keypadToLong(char *cmd)
{
  int8_t i, v, l;
  unsigned long longv;
  l = strlen(cmd);

  char temp[10];
  char t[2];
  for(i=0; i< l;i++){
    v = cmd[i];
    if(v > 47) v = v-48;
    cmd[i] = v;
    sprintf(t,"%d",v);
    temp[i] = t[0];
    //Serial.println(v);
  }
  i++;
  temp[i]='\0';
  longv = atol (temp);
  return longv;
}

void showCode(unsigned long code)
{
  Serial.print("unsigned long value: ");
  char tmp[12];
  ultoa(code, tmp, 10);
  Serial.println(code);
}

void Keypad::poll( )
{
  if(wg.available())
	{
    if(wg.getWiegandType() == 26){

      showCode(wg.getCode());

      *wg_msg_ptr = '\0';
      wg_msg_ptr = wgmsg;
      return;
    }

    char c = wg.getCode();
    switch (c)
    {
    case 13:
        // terminate the msg and reset the msg ptr. then send
        // it to the handler for processing.
        *wg_msg_ptr = '\0';
        showCode(keypadToLong((char *)wgmsg));

        wg_msg_ptr = wgmsg;
        break;

    case 27:
        // escape... clear buffer
        *wg_msg_ptr = '\0';
        wg_msg_ptr = wgmsg;
    break;

    default:
        // normal character entered. add it to the buffer
        //Serial.print(c);
        *wg_msg_ptr++ = c;
        break;
    }
		//Serial.print("Wiegand HEX = ");
		//Serial.print(wg.getCode(),HEX);
		//Serial.print(", DECIMAL = ");
		//Serial.print(wg.getCode());
		//Serial.print(", Type W");
		//Serial.println(wg.getWiegandType());
	}
}
