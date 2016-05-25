#include <Arduino.h>
#include <Streaming.h>

#include "Scheduler.h"
#include <RtcDS3231.h>
#include "EEPROM.h"

#define MATCH_ANY 255

#define RELAY_STATE_NONE 0
#define RELAY_STATE_UNLOCKED 1
#define RELAY_STATE_LOCKED 2
#define RELAY_STATE_FORCE_LOCK 3
#define RELAY_STATE_INHERIT 4
const signed char relayState[] = {'X', 'U','L', 'F', 'I'};

#define SENSOR_STATE_NONE 0
#define SENSOR_STATE_UNARMED 1
#define SENSOR_STATE_CHIME 2
#define SENSOR_STATE_ARMED 3
#define SENSOR_STATE_INHERIT 4
const signed char sensorState[] = {'X','U','C','A','I'};

#define ENV_STATE_NONE 0
#define ENV_STATE_UNARMED 1
#define ENV_STATE_ARMED 2
#define ENV_STATE_STAY 3
#define ENV_STATE_FORCE_ARMED 4
#define ENV_STATE_FORCE_DISARMED 5
#define ENV_STATE_INHERIT 6
const signed char envState[] = {'X','U','A','S','F','O','I'};

#define RULE_FLAG_NONE 0
#define RULE_FLAG_FINAL 1
const signed char ruleFlag[] = {'X','F'};

#define DEFAULT_RELAY_STATE 2
#define DEFAULT_SENSOR_STATE 1
#define DEFAULT_ENV_STATE 1


const char* dayNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void Scheduler::loadFromMemory(uint8_t memConfigStart){
  rules_count = EEPROM.read(memConfigStart);

  for (unsigned int t=0; t<sizeof(schedule); t++)
  *((char*)&schedule + t) = EEPROM.read(memConfigStart + 1 + t);
}

void Scheduler::poll( )
{
  int state = getState();
  if(state == doorScheduleState) return;
  if(state == DOOR_SCHEDULE_STATE_LOCKED){
    Serial.println("LOCKING DOOR");
  }else{
    Serial.println("UNLOCKING DOOR");
  }
  doorScheduleState = state;
}

uint8_t Scheduler::resolveSensorState(char key){
  if(key == relayState[SENSOR_STATE_UNARMED]) return SENSOR_STATE_UNARMED;
  if(key == relayState[SENSOR_STATE_CHIME]) return SENSOR_STATE_CHIME;
  if(key == relayState[SENSOR_STATE_ARMED]) return SENSOR_STATE_ARMED;
  if(key == relayState[SENSOR_STATE_INHERIT]) return SENSOR_STATE_INHERIT;
  return SENSOR_STATE_NONE;
}

uint8_t Scheduler::resolveRelayState(char key){
  if(key == relayState[RELAY_STATE_UNLOCKED]) return RELAY_STATE_UNLOCKED;
  if(key == relayState[RELAY_STATE_LOCKED]) return RELAY_STATE_LOCKED;
  if(key == relayState[RELAY_STATE_FORCE_LOCK]) return RELAY_STATE_FORCE_LOCK;
  if(key == relayState[RELAY_STATE_INHERIT]) return RELAY_STATE_INHERIT;
  return RELAY_STATE_NONE;
}

void Scheduler::add(char **args){

  Serial << "Saved Rule to schedule slot: " << endl;
  Serial << rules_count << endl;

  int year = atoi(args[1]);
  if(year > 2000) year -= 2000;
  schedule[rules_count].year            = year;
  schedule[rules_count].month           = atoi(args[2]);
  schedule[rules_count].day             = atoi(args[3]);
  schedule[rules_count].dow             = atoi(args[4]);
  schedule[rules_count].open_hour       = atoi(args[5]);
  schedule[rules_count].open_min        = atoi(args[6]);
  schedule[rules_count].close_hour      = atoi(args[7]);
  schedule[rules_count].close_min       = atoi(args[8]);


  schedule[rules_count].door1_relay = resolveRelayState(args[9][0]);
  schedule[rules_count].door1_sensor = resolveSensorState(args[9][1]);

  schedule[rules_count].door2_relay = resolveRelayState(args[10][0]);
  schedule[rules_count].door2_sensor = resolveSensorState(args[10][1]);

  schedule[rules_count].door3_relay = resolveRelayState(args[11][0]);
  schedule[rules_count].door3_sensor = resolveSensorState(args[11][1]);

  schedule[rules_count].door4_relay = resolveRelayState(args[12][0]);
  schedule[rules_count].door4_sensor = resolveSensorState(args[12][1]);

  schedule[rules_count].env_flag = ENV_STATE_NONE;
  schedule[rules_count].rule_flag  = RULE_FLAG_NONE;

  for (int i=0; i<6; i++)
  {
    if(args[13][0] == envState[i]){
      schedule[rules_count].env_flag = i;
      break;
    }
  }
  if(args[14][0] == ruleFlag[RULE_FLAG_FINAL]){
    schedule[rules_count].rule_flag = RULE_FLAG_FINAL;
  }
  rules_count++;

}

void Scheduler::clearAll(){
  rules_count = 0;
}

void Scheduler::save(uint8_t memConfigStart){


  EEPROM.write(memConfigStart, rules_count);
  char *s = (char *) &schedule;

  for (unsigned int t=0; t<sizeof(schedule); t++)
  {
    char value = *(s + t);
    EEPROM.write(memConfigStart + 1 + t, value);
        if (EEPROM.read(memConfigStart + 1 + t) != value)
        {
          Serial.print("x");
        }else{
          Serial.print(".");
        }
  }

  Serial.print("Saved ");
  Serial.print(sizeof(schedule)+1);
  Serial.println(" bytes of data");

}

void printPad(int8_t num){
  if(num == 255 || num == -1){
    Serial.print("ANY");
    return;
  }
  if(num < 9){
    Serial.print("0");
  }
  Serial.print(num);
}

void Scheduler::list(){

    Serial.println(F("-- Basic Door Controller"));
    Serial.println(F("YEAR-MONTH-DAY\tDOW\tSTART-END\tDR1\tDR2\tDR3\tDR4\tSITE\tRULE\t_metric\t_match"));

    for (int i=0; i<rules_count; i++)
    {
      Serial << "20";
      printPad(schedule[i].year);
      Serial << "-";
      printPad(schedule[i].month);
      Serial << "-";
      printPad(schedule[i].day);

      Serial << "\t";
      printPad(schedule[i].dow);
      Serial << "\t";

      printPad(schedule[i].open_hour);
      Serial << ":";
      printPad(schedule[i].open_min);
      Serial << "-";
      printPad(schedule[i].close_hour);
      Serial << ":";
      printPad(schedule[i].close_min);

      Serial << "\t"
        << _BYTE(relayState[ schedule[i].door1_relay])
        << _BYTE(sensorState[schedule[i].door1_sensor])
      << "\t"
        << _BYTE(relayState[ schedule[i].door2_relay ])
        << _BYTE(sensorState[schedule[i].door2_sensor])
      << "\t"
        << _BYTE(relayState[ schedule[i].door3_relay])
        << _BYTE(sensorState[schedule[i].door3_sensor])
      << "\t"
        << _BYTE(relayState[ schedule[i].door4_relay])
        << _BYTE(sensorState[schedule[i].door4_sensor])
      << "\t"
        << _BYTE(envState[ schedule[i].env_flag ])
      << "\t"
        << _BYTE(ruleFlag[ schedule[i].rule_flag ])
      << "\t"
        << _DEC(schedule[i].metric)
      << "\t"
        << _DEC(schedule[i].match)
      << endl;
    }
}

// void Scheduler::sort() {
//     for(int i=0; i<rules_count; i++) {
//         for(int o=0; o<(rules_count-1+i); o++) {
//                 if(schedule[o].metric > schedule[o+1].metric) {
//                     ScheduleType t = schedule[o];
//                     schedule[o] = schedule[o+1];
//                     schedule[o+1] = t;
//                 }
//         }
//     }
// }

uint8_t Scheduler::resolveDayStateOfSchedule(RtcDateTime dt, ScheduleType rule){

  //open hours set?
  if(rule.open_hour < 0 || rule.close_hour < 0){
    minutes_till_open = 0;
    minutes_till_close = 0;
    Serial << "Open or Close Hour not set. Error." << endl;
    return -1;
  }
  //so... are we open?  ...24h format, convert to minutes
  int open = (rule.open_hour * 60) + rule.open_min;
  int close = (rule.close_hour * 60) + rule.close_min;
  int current = (dt.Hour() * 60) + dt.Minute();

  minutes_till_open = open - current;
  minutes_till_close = close - current;

  if(minutes_till_open < 0) minutes_till_open = 0;
  if(minutes_till_close < 0) minutes_till_close = 0;

  //Serial.print(open); Serial.print(" "); Serial.print(close); Serial.print(" "); Serial.println(current);

  if(current >= open && current <= close){
    return DOOR_SCHEDULE_STATE_UNLOCKED;
  }else{
    return DOOR_SCHEDULE_STATE_LOCKED;
  }
}

void Scheduler::status(){
  if(minutes_till_open > 0){
    Serial.print("Site will be opening in ");
    Serial.print(minutes_till_open);
    Serial.println(" Minutes");
  }
  if(minutes_till_close > 0){
    Serial.print("Site will be closing in ");
    Serial.print(minutes_till_close);
    Serial.println(" Minutes");
  }
}

uint8_t Scheduler::getState(){

  if(rules_count < 1){
    minutes_till_open = 0;
    minutes_till_close = 0;
    return DOOR_SCHEDULE_STATE_LOCKED;
  }

    RtcDateTime dt = _RTC.GetDateTime();
    for (uint8_t i=0; i<rules_count; i++)
    {
      schedule[i].metric = 0;
      schedule[i].match = false;
    }

    for (uint8_t i=0; i<rules_count; i++)
    {
        //specific date. Is exact match, metric point.
        //no exact match, look for ANY match, no metric point

        if(schedule[i].year == (dt.Year()-2000) ){
          schedule[i].metric+=50;
        }else{
          if(schedule[i].year != MATCH_ANY)
            continue;
        }

        if(schedule[i].month == dt.Month() ){
          schedule[i].metric+=50;
        }else{
          if(schedule[i].month != MATCH_ANY)
            continue;
        }

        if(schedule[i].day == dt.Day() ){
          schedule[i].metric+=50;
        }else{
          if(schedule[i].day != MATCH_ANY)
            continue;

        }

        if(schedule[i].dow == dt.DayOfWeek() ){
          schedule[i].metric+=50;
        }else{
          if(schedule[i].dow != MATCH_ANY)
            continue;

        }

        if(schedule[i].metric > 150){
          //THIS IS NOT VALID, match my year, month, day, and dow??
        }

        int open = (schedule[i].open_hour * 60) + schedule[i].open_min;
        int close = (schedule[i].close_hour * 60) + schedule[i].close_min;
        int current = (dt.Hour() * 60) + dt.Minute();

        if(current >= open && current <= close){
          schedule[i].match = true;
          int tm = ( ( ((1440 - (close - open) ) * 100) /1440 ) * 50 )/ 100;
          schedule[i].metric+=tm;
        }

        //
        // minutes_till_open = open - current;
        // minutes_till_close = close - current;
        //
        // if(minutes_till_open < 0) minutes_till_open = 0;
        // if(minutes_till_close < 0) minutes_till_close = 0;

      }

      //return DOOR_SCHEDULE_STATE_LOCKED;
      return 0;
    }
