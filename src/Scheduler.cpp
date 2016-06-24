#include <Arduino.h>
#include <Streaming.h>
#include "Scheduler.h"
#include "config.h"


#define DEFAULT_RELAY_STATE 2
#define DEFAULT_SENSOR_STATE 1
#define DEFAULT_ENV_STATE 1
#define MATCH_ANY 255

const signed char relayState[] = {'X', 'U','L', 'F', 'I'};
enum relay_state {RELAY_STATE_NONE, RELAY_STATE_UNLOCKED, RELAY_STATE_LOCKED, RELAY_STATE_FORCE_LOCK, RELAY_STATE_INHERIT};

const signed char sensorState[] = {'X','U','C','A','I'};
enum sensor_state {SENSOR_STATE_NONE, SENSOR_STATE_UNARMED, SENSOR_STATE_CHIME, SENSOR_STATE_ARMED, SENSOR_STATE_INHERIT};

const signed char envState[] = {'X','U','A','S','F','O','I'};
enum env_state {ENV_STATE_NONE, ENV_STATE_UNARMED, ENV_STATE_ARMED, ENV_STATE_STAY, ENV_STATE_FORCE_ARMED, ENV_STATE_FORCE_DISARMED, ENV_STATE_INHERIT};

const signed char ruleFlag[] = {'X','F'};
enum rule_flag {RULE_FLAG_NONE, RULE_FLAG_FINAL};


void Scheduler::init(){
    int8_t i = 0;
    for(i=0; i<NUM_OF_DOORS;i++){
      doors[i].setPin(doorPinMatrix[i][0], doorPinMatrix[i][1]);
    }
}

void Scheduler::poll( RtcDateTime dt )
{
  uint8_t i;
  for(i=0; i<NUM_OF_DOORS; i++){

    doors[i].poll();

    if( doors[i].sensor_state != doors[i].sensor_last_state){

      Serial << "INPUT <" << i << "> " <<doors[i].sensor_last_state << " to " << doors[i].sensor_state << endl;
      doors[i].sensor_last_state = doors[i].sensor_state;

      if(doors[i].sensor_state == DOOR_SENSOR_STATE_SHORT){
        Serial << "Check For RTE Event Support " << endl;
      }

    }

  }

  evalState(dt);

  //TODO: need to have the rules sorted by metric value, but lets pretend like that has already happened

  //resolved_state
  //current_state

  for (uint8_t i=0; i<rules_count; i++){
    if(!schedule_metric[i].match) continue;
    for (uint8_t d=0; d<NUM_OF_DOORS; d++){
        if(schedule[i].doors[d].relay != RELAY_STATE_NONE || resolved_state.doors[d].relay != RELAY_STATE_INHERIT)
          resolved_state.doors[d].relay = schedule[i].doors[d].relay;
        if(schedule[i].doors[d].sensor != SENSOR_STATE_NONE || resolved_state.doors[d].sensor != SENSOR_STATE_INHERIT)
          resolved_state.doors[d].sensor = schedule[i].doors[d].sensor;
    }
    if(schedule[i].env_flag != ENV_STATE_INHERIT || schedule[i].env_flag != ENV_STATE_NONE)
      resolved_state.env_flag = schedule[i].env_flag;
  }//resolved state complete


  //loop through doors again, apply resolved state as current state
  for (uint8_t d=0; d<NUM_OF_DOORS; d++){
    //check relay state
    if(current_state.doors[d].relay != resolved_state.doors[d].relay){
      if(resolved_state.doors[d].relay == RELAY_STATE_LOCKED || resolved_state.doors[d].relay == RELAY_STATE_FORCE_LOCK){
        Serial << "Locking Door " << d << endl;
        doors[d].lock();
      }else{
        Serial << "Unlocking Door " << d << endl;
        doors[d].unlock();
      }
      current_state.doors[d].relay = resolved_state.doors[d].relay;
    }

    //check door sensor state
    if(current_state.doors[d].sensor != resolved_state.doors[d].sensor){
      current_state.doors[d].sensor = resolved_state.doors[d].sensor;
    }

  }
  if(current_state.env_flag != resolved_state.env_flag){
    //site env flag changed
    current_state.env_flag = resolved_state.env_flag;
  }


}



const char* dayNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void Scheduler::loadFromMemory(uint8_t memConfigStart){
  rules_count = EEPROM.read(memConfigStart);
  if(rules_count > 20){
    Serial.println("Invalid Rules Count");
    rules_count = 0;
  }
  for (unsigned int t=0; t<sizeof(schedule); t++)
  *((char*)&schedule + t) = EEPROM.read(memConfigStart + 1 + t);
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

  int p = 9;
  for (uint8_t d=0; d<NUM_OF_DOORS; d++){
    schedule[rules_count].doors[d].relay = resolveRelayState(args[p][0]);
    schedule[rules_count].doors[d].sensor = resolveSensorState(args[p][1]);
    p++;
  }

  schedule[rules_count].env_flag = ENV_STATE_NONE;
  schedule[rules_count].rule_flag  = RULE_FLAG_NONE;

  for (int i=0; i<6; i++)
  {
    if(args[p][0] == envState[i]){
      schedule[rules_count].env_flag = i;
      break;
    }
  }
  p++;

  if(args[p][0] == ruleFlag[RULE_FLAG_FINAL]){
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
    Serial.print(F("YEAR-MONTH-DAY\tDOW\tSTART-END"));

    for (uint8_t d=0; d<NUM_OF_DOORS; d++){
      Serial << "\tDR" << d;
    }

    Serial.println(F("\tSITE\tRULE\t_metric\t_match\t_aged\t_until"));

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

      for (uint8_t d=0; d<NUM_OF_DOORS; d++){
        Serial << "\t" << _BYTE(relayState[ schedule[i].doors[d].relay]) << _BYTE(sensorState[schedule[i].doors[d].sensor]);
      }

      Serial << "\t"
        << _BYTE(envState[ schedule[i].env_flag ])
      << "\t"
        << _BYTE(ruleFlag[ schedule[i].rule_flag ])
      << "\t"
        << _DEC(schedule_metric[i].metric)
      << "\t"
        << _DEC(schedule_metric[i].match)
      << "\t"
        << _DEC(schedule_metric[i].minutes_since)
      << "\t"
        << _DEC(schedule_metric[i].minutes_until)
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



void Scheduler::status(){

  uint8_t i;

  for(i=0; i<NUM_OF_DOORS; i++){
    Serial << "Door\t" << i << ",\tValue:\t" << doors[i].getValue() << ",\tState:\t" <<  doors[i].getState() << "\tRelay:\t" <<  doors[i].getRelayState() << endl;
  }


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

void Scheduler::setDoorState(uint8_t door, uint8_t state ){
  Serial << "Set Door " << door << " State " << state << endl;

  if(state == RELAY_STATE_LOCKED) doors[door].lock();
  if(state == RELAY_STATE_UNLOCKED) doors[door].unlock();

}



void Scheduler::evalState(RtcDateTime dt){
  if(rules_count < 1) return;

    //RtcDateTime dt = _RTC.GetDateTime();
    for (uint8_t i=0; i<rules_count; i++)
    {
      schedule_metric[i].metric = 0;
      schedule_metric[i].match = false;
      schedule_metric[i].minutes_until = -1;
      schedule_metric[i].minutes_since = -1;
    }

    for (uint8_t i=0; i<rules_count; i++)
    {
        //specific date. Is exact match, metric point.
        //no exact match, look for ANY match, no metric point

        if(schedule[i].year == (dt.Year()-2000) ){
          schedule_metric[i].metric+=50;
        }else{
          if(schedule[i].year != MATCH_ANY)
            continue;
        }

        if(schedule[i].month == dt.Month() ){
          schedule_metric[i].metric+=50;
        }else{
          if(schedule[i].month != MATCH_ANY)
            continue;
        }

        if(schedule[i].day == dt.Day() ){
          schedule_metric[i].metric+=50;
        }else{
          if(schedule[i].day != MATCH_ANY)
            continue;

        }

        if(schedule[i].dow == dt.DayOfWeek() ){
          schedule_metric[i].metric+=50;
        }else{
          if(schedule[i].dow != MATCH_ANY)
            continue;

        }

        if(schedule_metric[i].metric > 150){
          //THIS IS NOT VALID, match my year, month, day, and dow??
        }

        int open = (schedule[i].open_hour * 60) + schedule[i].open_min;
        int close = (schedule[i].close_hour * 60) + schedule[i].close_min;
        int current = (dt.Hour() * 60) + dt.Minute();

        if(current >= open && current <= close){
          schedule_metric[i].match = true;
          int tm = ( ( ((1440 - (close - open) ) * 100) /1440 ) * 50 )/ 100;
          schedule_metric[i].metric+=tm;

        }

        int minutes_until = open - current;
        int minutes_since = close - current;
        if(minutes_until > 0){
          schedule_metric[i].minutes_until = minutes_until;
        }else if(minutes_since > 0){
          schedule_metric[i].minutes_since = minutes_since;
        }

      }

      return;
    }
