#include <Arduino.h>
#include "Scheduler.h"
#include <RtcDS3231.h>
#include "EEPROM.h"

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


void Scheduler::add(char **args){

  Serial.print("Saved Rule to schedule slot: ");
  Serial.println(rules_count);

  schedule[rules_count].relaygroup      = atoi(args[2]);
  schedule[rules_count].index           = atoi(args[3]);
  int year = atoi(args[4]);
  if(year > 2000) year -= 2000;
  schedule[rules_count].year            = year;
  schedule[rules_count].month           = atoi(args[5]);
  schedule[rules_count].day             = atoi(args[6]);
  schedule[rules_count].dayofweek       = atoi(args[7]);
  schedule[rules_count].closed_all_day  = atoi(args[8]);
  schedule[rules_count].open_hour       = atoi(args[9]);
  schedule[rules_count].open_min        = atoi(args[10]);
  schedule[rules_count].close_hour      = atoi(args[11]);
  schedule[rules_count].close_min       = atoi(args[12]);
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
            if(schedule[i].closed_all_day){
              Serial.print("-- Closed all day for ");
            }else{
              Serial.print("-- Open between ");
                Serial.print(schedule[i].open_hour);
                Serial.print(":");
                Serial.print(schedule[i].open_min);
                Serial.print(" and ");
                Serial.print(schedule[i].close_hour);
                Serial.print(":");
                Serial.print(schedule[i].close_min);
              Serial.print(" for ");
            }
            Serial.print(schedule[i].month);
            Serial.print("/");
            Serial.print(schedule[i].day);
            Serial.print("/");
            Serial.println(schedule[i].year);
            continue;
          }

          if(schedule[i].dayofweek < 7){
            if(schedule[i].closed_all_day){
              Serial.print("-- Closed all day on ");
            }else{
              Serial.print("-- Open between ");
                Serial.print(schedule[i].open_hour);
                Serial.print(":");
                Serial.print(schedule[i].open_min);
                Serial.print(" and ");
                Serial.print(schedule[i].close_hour);
                Serial.print(":");
                Serial.print(schedule[i].close_min);
              Serial.print(" on ");
            }
            Serial.println(dayNames[schedule[i].dayofweek ]);
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
  if(rule.closed_all_day == 1){
    //SET RELAY CLOSED, RETURN, FINISHED LOOKING FOR RULE MATCH
    return DOOR_SCHEDULE_STATE_LOCKED;
  }

  //open hours set?
  if(rule.open_hour < 0 || rule.close_hour < 0){
    Serial.println("Open or Close Hour not set. Error.");
    return -1;
  }
  //so... are we open?  ...24h format, convert to minutes
  int open = (rule.open_hour * 60) + rule.open_min;
  int close = (rule.close_hour * 60) + rule.close_min;
  int current = (dt.Hour() * 60) + dt.Minute();

  minutes_till_open = current - open;
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

  if(rules_count < 1) return DOOR_SCHEDULE_STATE_LOCKED;

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
        if(schedule[i].dayofweek != 255){
            if(schedule[i].dayofweek == dt.DayOfWeek()){
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
        if(schedule[i].dayofweek != 255 || schedule[i].year != 255) continue;
        //if we still made it here, the its just a open / close date entry
        return resolveDayStateOfSchedule(dt, schedule[i]);
      }

      //no rules matching, assume state of LOCKED
      return DOOR_SCHEDULE_STATE_LOCKED;

    }
