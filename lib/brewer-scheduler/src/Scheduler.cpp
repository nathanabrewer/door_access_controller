#include <Arduino.h>
#include "Scheduler.h"
#include <RtcDS3231.h>
#include "EEPROM.h"

#define RELAY_STATE_NONE 0
#define RELAY_STATE_UNLOCKED 1
#define RELAY_STATE_LOCKED 2
#define RELAY_STATE_FORCE_LOCK 3
#define RELAY_STATE_INHERIT 4
const unsigned char relayState[] = {'X', 'U','L', 'F', 'I'};

#define SENSOR_STATE_NONE 0
#define SENSOR_STATE_UNARMED 1
#define SENSOR_STATE_CHIME 2
#define SENSOR_STATE_ARMED 3
#define SENSOR_STATE_INHERIT 4
const unsigned char sensorState[] = {'X','U','C','A','I'};

#define ENV_STATE_NONE 0
#define ENV_STATE_UNARMED 1
#define ENV_STATE_ARMED 2
#define ENV_STATE_STAY 3
#define ENV_STATE_FORCE_ARMED 4
#define ENV_STATE_FORCE_DISARMED 5
#define ENV_STATE_INHERIT 6
const unsigned char envState[] = {'X','U','A','S','F','O','I'};

#define RULE_FLAG_NONE 0
#define RULE_FLAG_FINAL 1
const unsigned char ruleFlag[] = {'X','F'};

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
  if(key == relayState[SENSOR_STATE_UNARMED]) Serial.println("SENSOR_STATE_UNARMED");
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

  Serial.print("Saved Rule to schedule slot: ");
  Serial.println(rules_count);

  int year = atoi(args[2]);
  if(year > 2000) year -= 2000;
  schedule[rules_count].year            = year;
  schedule[rules_count].month           = atoi(args[3]);
  schedule[rules_count].day             = atoi(args[4]);
  schedule[rules_count].dow             = atoi(args[5]);
  schedule[rules_count].open_hour       = atoi(args[6]);
  schedule[rules_count].open_min        = atoi(args[7]);
  schedule[rules_count].close_hour      = atoi(args[8]);
  schedule[rules_count].close_min       = atoi(args[9]);

  schedule[rules_count].door1_relay = resolveRelayState(args[10][0]);
  schedule[rules_count].door1_sensor = resolveSensorState(args[10][1]);

  schedule[rules_count].door2_relay = resolveRelayState(args[11][0]);
  schedule[rules_count].door2_sensor = resolveSensorState(args[11][1]);

  schedule[rules_count].door3_relay = resolveRelayState(args[12][0]);
  schedule[rules_count].door3_sensor = resolveSensorState(args[12][1]);

  schedule[rules_count].door4_relay = resolveRelayState(args[13][0]);
  schedule[rules_count].door4_sensor = resolveSensorState(args[13][1]);



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

void Scheduler::list(char **args){
          Serial.println("- Basic Door Controller [ Scheduled Rule Index ]");
    for (int i=0; i<rules_count; i++)
    {
          //Serial.print("- ");
          //Serial.print(i);
          //Serial.print(".");
          //Serial.println(schedule[i].index);
          //Serial.print("- Relay Group ");
          //Serial.println(schedule[i].relaygroup);

          if(schedule[i].year != 255){
            // if(schedule[i].closed_all_day){
            //   Serial.print("-- Closed all day for ");
            // }else{
              Serial.print("-- Open between ");
                Serial.print(schedule[i].open_hour);
                Serial.print(":");
                Serial.print(schedule[i].open_min);
                Serial.print(" and ");
                Serial.print(schedule[i].close_hour);
                Serial.print(":");
                Serial.print(schedule[i].close_min);
              Serial.print(" for ");
          //  }

            Serial.print(schedule[i].month);
            Serial.print("/");
            Serial.print(schedule[i].day);
            Serial.print("/");
            Serial.println(schedule[i].year);
            continue;
          }

          if(schedule[i].dow < 7){
            // if(schedule[i].closed_all_day){
            //   Serial.print("-- Closed all day on ");
            // }else{
              Serial.print("-- Open between ");
                Serial.print(schedule[i].open_hour);
                Serial.print(":");
                Serial.print(schedule[i].open_min);
                Serial.print(" and ");
                Serial.print(schedule[i].close_hour);
                Serial.print(":");
                Serial.print(schedule[i].close_min);
              Serial.print(" on ");
          //  }
            Serial.println(dayNames[schedule[i].dow ]);
            continue;
          }

          Serial.print("-- Open between ");

          Serial.print(schedule[i].open_hour);
          Serial.print(":");
          Serial.print(schedule[i].open_min);
          Serial.print(" and ");
          Serial.print(schedule[i].close_hour);
          Serial.print(":");
          Serial.println(schedule[i].close_min);

    }
}



uint8_t Scheduler::resolveDayStateOfSchedule(RtcDateTime dt, ScheduleType rule){
  //closed all day?
  // if(rule.closed_all_day == 1){
  //   minutes_till_open = 0;
  //   minutes_till_close = 0;
  //   //SET RELAY CLOSED, RETURN, FINISHED LOOKING FOR RULE MATCH
  //   return DOOR_SCHEDULE_STATE_LOCKED;
  // }

  //open hours set?
  if(rule.open_hour < 0 || rule.close_hour < 0){
    minutes_till_open = 0;
    minutes_till_close = 0;
    Serial.println("Open or Close Hour not set. Error.");
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
    for (int i=0; i<rules_count; i++)
    {
        //specific data
        if(schedule[i].year != 255){
          if(schedule[i].year == (dt.Year()-2000)){
            if(schedule[i].month == dt.Month()){
              if(schedule[i].day == dt.Day()){

                  //we found a match for this specific DAY
                  return resolveDayStateOfSchedule(dt, schedule[i]);


              }else{
                // Specific Date, but no day match, we can continue out of this for loop
                // look for other matching rules, but we are done looking at this rule
                continue;
              }
            }else{
              // Specific Date, but no month match, we can continue out of this for loop
              // look for other matching rules, but we are done looking at this rule
              continue;
            }
          }else{
            // Specific Date, but no match on year, we can continue out of this for loop
            // look for other matching rules, but we are done looking at this rule
            continue;
          }
        }


      }

      for (int i=0; i<rules_count; i++)
      {
        //look for day of week match
        if(schedule[i].dow != 255){
            if(schedule[i].dow == dt.DayOfWeek()){
                //we found a match for this specific DAY
                return resolveDayStateOfSchedule(dt, schedule[i]);

            }else{
              // Specific DayOfWeek set, but not this day of week, we can contineu out of this for loop
              // look for other matching rules, but we are done looking at this rule
              continue;
            }
        }
      }

      //last loop
      for (int i=0; i<rules_count; i++)
      {
        if(schedule[i].dow != 255 || schedule[i].year != 255) continue;
        //if we still made it here, the its just a open / close date entry
        return resolveDayStateOfSchedule(dt, schedule[i]);
      }

      minutes_till_open = 0;
      minutes_till_close = 0;
      //no rules matching, assume state of LOCKED
      return DOOR_SCHEDULE_STATE_LOCKED;

    }
