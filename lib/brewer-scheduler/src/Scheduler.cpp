#include <Arduino.h>
#include "Scheduler.h"
#include <RtcDS3231.h>



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
  schedule[rules_count].year            = atoi(args[4]);
  schedule[rules_count].month           = atoi(args[5]);
  schedule[rules_count].day             = atoi(args[6]);
  schedule[rules_count].dayofweek       = atoi(args[7]);
  schedule[rules_count].closed_all_day  = atoi(args[8]);
  schedule[rules_count].open_hour       = atoi(args[9]);
  schedule[rules_count].open_min        = atoi(args[10]);
  schedule[rules_count].close_hour      = atoi(args[11]);
  schedule[rules_count].close_min       = atoi(args[12]);
  schedule[rules_count].last            = atoi(args[13]);
  rules_count++;
}


void Scheduler::list(char **args){
    for (int i=0; i<rules_count; i++)
    {

          Serial.print("[trueindex ");
          Serial.print(" ");
          Serial.print(i );
          Serial.print(" ");
          Serial.print(schedule[i].index);
          Serial.print(" ");
          Serial.print(schedule[i].relaygroup);
          Serial.print(" ");
          Serial.print(schedule[i].year);
          Serial.print(" ");
          Serial.print(schedule[i].month);
          Serial.print(" ");
          Serial.print(schedule[i].day);
          Serial.print(" ");
          Serial.print(schedule[i].dayofweek);
          Serial.print(" ");
          Serial.print(schedule[i].closed_all_day);
          Serial.print(" ");
          Serial.print(schedule[i].open_hour);
          Serial.print(schedule[i].open_min);
          Serial.print(" ");
          Serial.print(schedule[i].close_hour);
          Serial.print(  schedule[i].close_min);
          Serial.print(" ");
          Serial.println(schedule[i].last);
        // p("[trueindex %d] I%d R%d Y%d M%d D%d dow-%d (%d) %d:%d %d:%d %d\n",
        // i,
        // schedule[i].index,
        // schedule[i].relaygroup,
        // schedule[i].year,
        // schedule[i].month,
        // schedule[i].day,
        // schedule[i].dayofweek,
        // schedule[i].closed_all_day,
        // schedule[i].open_hour,
        // schedule[i].open_min,
        // schedule[i].close_hour,
        // schedule[i].close_min,
        // schedule[i].last);
    }
}



int resolveDayStateOfSchedule(RtcDateTime dt, ScheduleType rule){
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
  if(current >= open && current <= close){
    return DOOR_SCHEDULE_STATE_UNLOCKED;
  }else{
    return DOOR_SCHEDULE_STATE_LOCKED;
  }
}

int Scheduler::getState(){
  if(rules_count < 1) return -1;
    RtcDateTime dt = _RTC.GetDateTime();
    for (int i=0; i<rules_count; i++)
    {
        //specific data
        if(schedule[i].year > 0){
          if(schedule[i].year == dt.Year()){
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

        //look for day of week match
        if(schedule[i].dayofweek > 0){
            if(schedule[i].dayofweek == dt.DayOfWeek()){
                //we found a match for this specific DAY
                return resolveDayStateOfSchedule(dt, schedule[i]);

            }else{
              // Specific DayOfWeek set, but not this day of week, we can contineu out of this for loop
              // look for other matching rules, but we are done looking at this rule
              continue;
            }
        }
        //if we still made it here, the its just a open / close date entry
        return resolveDayStateOfSchedule(dt, schedule[i]);

    }
}
