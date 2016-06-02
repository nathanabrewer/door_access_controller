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


// unsigned long keypadToLong(char *cmd)
// {
//   //YUCK, this is UGLY! ...but it works for now
//   int8_t i, v, l;
//   unsigned long longv;
//   l = strlen(cmd);
//   longv = 0;
//   for(i=0; i < l ;i++){
//     v = cmd[i];
//     Serial.println( (pow(10,( l-i-1) )*(int)v) );
//     longv += pow(10,( l-i-1) )*v;
//   }
// Serial.println(longv);
//   return longv;
// }

unsigned long keypadToLong(char *cmd)
{
  //YUCK, this is UGLY! ...but it works for now
  //having issues with null terminated \0 ...it is truncating a keypad code at zero
  int8_t i, v, l;
  unsigned long longv;
  l = strlen(cmd);
  char temp[10];
  char t[2];
  for(i=0; i< l;i++){
    v = cmd[i];
    if(v > 47) v = v-48; //bring back to actual value
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

  if(code == 6321032){
    Serial.println(F("Welcome User XYZ"));
  }
  if(code == 8818){
    Serial.println(F("Hey Nate!"));
  }

  Serial.print(F("RX KEY/CARD >> "));
  char tmp[12];
  ultoa(code, tmp, 10);
  Serial.println(code);
}

void Keypad::poll( )
{
  if(wg.available())
	{
    Serial.print(".");

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
        Serial.print(c);
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
