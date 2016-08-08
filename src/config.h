
#ifndef DC_CONFIG
#define DC_CONFIG

#include "EEPROM.h"
#include <Arduino.h>
#include <pins_arduino.h>


#define BAUD_RATE 57600
#define GRANT_ACCESS_RELAY_TIME 10000

#define UNO_BOARD 1
#define MEGA_BOARD 2
#define NUMATO_BOARD 3
#define ESP_BOARD 4

// UNO 1
// MEGA 2
// NUMATO 3
// ESP 4
//#define BOARD_TYPE 3

#if(BOARD_TYPE == NUMATO_BOARD)
    #include <avr/pgmspace.h>
    #define RELAY_LOCK LOW
    #define RELAY_UNLOCK HIGH
    #define DS3231 0
    #define DC_MEGA 0
    #define WIEGAND_DATA1_PIN 4
    #define WIEGAND_DATA0_PIN 5
    #define KEYPAD_LED 11
    #define KEYPAD_BUZZER 12
    #define MAX_SCHEDULE_SIZE 5
    #define NUM_OF_DOORS 4
    #define MAX_USERS 4
    const int doorPinMatrix[NUM_OF_DOORS][2] = { {A0,  2 }, {A1,  3 }, {A2, 9 }, {A3, 7 } };
#endif

#if(BOARD_TYPE == MEGA_BOARD)
    #include <avr/pgmspace.h>
    #define RELAY_LOCK HIGH
    #define RELAY_UNLOCK LOW
    #define DS3231 1
    #define DC_MEGA 1
    #define WIEGAND_DATA1_PIN 2   //Wiegand WHITE Wire
    #define WIEGAND_DATA0_PIN 3   //Wiegand GREEN Wire
    #define KEYPAD_LED 4          //Wiegand BLUE Wire
    #define KEYPAD_BUZZER 5       //Wiegand YELLOW Wire
    #define MAX_SCHEDULE_SIZE 10
    #define NUM_OF_DOORS 8
    #define MAX_USERS 20
    const int doorPinMatrix[NUM_OF_DOORS][2] = { {A8,  22 }, {A9,  23 }, {A10, 24 }, {A11, 25 }, {A12, 26 }, {A13, 27 }, {A14, 28 }, {A15, 29 } };
#endif

#if(BOARD_TYPE == UNO_BOARD)
    #include <avr/pgmspace.h>
    #define RELAY_LOCK HIGH
    #define RELAY_UNLOCK LOW
    #define DS3231 1
    #define DC_MEGA 0
    #define WIEGAND_DATA1_PIN 4
    #define WIEGAND_DATA0_PIN 5
    #define KEYPAD_LED 12
    #define KEYPAD_BUZZER 6
    #define MAX_SCHEDULE_SIZE 10
    #define NUM_OF_DOORS 4
    #define MAX_USERS 5
    const int doorPinMatrix[NUM_OF_DOORS][2] = { {A0,  2 }, {A1,  3 }, {A2, 4 }, {A3, 5 } };
#endif

#if(BOARD_TYPE == ESP_BOARD)

extern "C" {
#include "user_interface.h"
}
    #define RELAY_LOCK HIGH
    #define RELAY_UNLOCK LOW
    #define DS3231 1
    #define DC_MEGA 0
    #define WIEGAND_DATA1_PIN 4
    #define WIEGAND_DATA0_PIN 5
    #define KEYPAD_LED 12
    #define KEYPAD_BUZZER 6
    #define MAX_SCHEDULE_SIZE 10
    #define NUM_OF_DOORS 1
    #define MAX_USERS 5
    const int doorPinMatrix[NUM_OF_DOORS][2] = { {3, 4 } };
#endif

#if (DS3231 == 1)
  #include <RtcDS3231.h>
  #include <RtcTemperature.h>
  #include <RtcUtility.h>
#else
  #include <RtcDS1307.h>
#endif


#endif /* DC_CONFIG */
