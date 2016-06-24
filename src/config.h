
#ifndef DC_CONFIG
#define DC_CONFIG

  #include "EEPROM.h"
  #define DC_MEGA 1
  #define DS3231 1

  #if DC_MEGA
    #define WIEGAND_DATA1_PIN 2   //Wiegand WHITE Wire
    #define WIEGAND_DATA0_PIN 3   //Wiegand GREEN Wire
    #define KEYPAD_LED 4          //Wiegand BLUE Wire
    #define KEYPAD_BUZZER 5       //Wiegand YELLOW Wire
    #define MAX_SCHEDULE_SIZE 10
    #define NUM_OF_DOORS 8
    #define MAX_USERS 20
    const int doorPinMatrix[NUM_OF_DOORS][2] = { {A8,  22 }, {A9,  23 }, {A10, 24 }, {A11, 25 }, {A12, 26 }, {A13, 27 }, {A14, 28 }, {A15, 29 } };
  #else
    #define WIEGAND_DATA1_PIN 4
    #define WIEGAND_DATA0_PIN 5
    #define KEYPAD_LED 11
    #define KEYPAD_BUZZER 12
    #define MAX_SCHEDULE_SIZE 10
    #define NUM_OF_DOORS 4
    #define MAX_USERS 10
    const int doorPinMatrix[NUM_OF_DOORS][2] = { {A0,  2 }, {A1,  3 }, {A2, 4 }, {A3, 5 } };
  #endif

  #if DS3231
    #include <RtcDS3231.h>
    #include <RtcTemperature.h>
    #include <RtcUtility.h>
  #else
    #include <RtcDS1307.h>
  #endif


#endif /* DC_CONFIG */
