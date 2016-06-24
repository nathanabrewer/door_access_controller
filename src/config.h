
#ifndef DC_CONFIG
#define DC_CONFIG

  #include "EEPROM.h"
  #define DC_MEGA 1
  #define DS3231 1

  #if DC_MEGA
    #define WIEGAND_DATA1_PIN 2
    #define WIEGAND_DATA0_PIN 3
    #define KEYPAD_LED 4
    #define KEYPAD_BUZZER 5
  #else
    #define WIEGAND_DATA1_PIN 4
    #define WIEGAND_DATA0_PIN 5
    #define KEYPAD_LED 11
    #define KEYPAD_BUZZER 12
  #endif

  #if DS3231
    #include <RtcDS3231.h>
    #include <RtcTemperature.h>
    #include <RtcUtility.h>
  #else
    #include <RtcDS1307.h>
  #endif

  #if DC_MEGA
    #define MAX_SCHEDULE_SIZE 10
    #define NUM_OF_DOORS 8
    #define MAX_USERS 20
  #else
    #define MAX_SCHEDULE_SIZE 10
    #define NUM_OF_DOORS 4
    #define MAX_USERS 10
  #endif

#endif /* DC_CONFIG */
