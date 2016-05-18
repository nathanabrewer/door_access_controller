
#include <Arduino.h>
#include <LinkedList.h>
#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDateTime.h>
#include <RtcDS1307.h>
#include <RtcDS3231.h>
#include <RtcTemperature.h>
#include <RtcUtility.h>
#include <Cmd.h>

#include <door.h>
//#include <keypad.h>

#include <scheduler.h>

#define countof(a) (sizeof(a) / sizeof(a[0]))

RtcDS3231 Rtc;

int loopCount=0;
int rules_count = 0;

DoorSensor door1;
DoorSensor door2;
Scheduler schedule;
//Keypad *keypad = new Keypad;


void setup()
{
  cmdInit(57600);
  RTCSetup();
  //keypad =  Keypad();
  door1.setPin(A0);
  door2.setPin(A1);
  cmdAdd("say", command_say_hello);
  cmdAdd("add", command_add);
  cmdAdd("help", command_help);
  cmdAdd("list", command_list);
  cmdAdd("status", command_status);

  Serial.println("add sch <index> <relaygroup> <yr> <mnth> <day> <dow> <closedallsay> <open_hour> <open_min> <close_hour> <close_min>");
  Serial.println("");
  Serial.println("When specifying year/month do not specify dayofweek");
  Serial.println("-1 value instead of null");
  Serial.println("");
  Serial.println("> add sch 1 1 2016 05 15 -1 0 03 58 4 19 0");
  Serial.println("> add sch 2 1 -1 -1 -1 -1 1 -1 -1 -1 -1 0");
  Serial.println("> add sch 3 1 -1 -1 -1 -1 1 -1 -1 -1 -1 0");
  Serial.println("> add sch 4 1 -1 -1 -1 -1 0 17 30 18 30 1");
  Serial.println("");
  Serial.println("try status date for the current time, list all for repeat of current schedule");
  Serial.println("");


}
void command_list(int arg_cnt, char **args){
  schedule.list(args);
}
void loop()
{
  cmdPoll();
  poll();

  //keypad->poll();
  //sensorB->poll();
  //delay(100);
}
void command_status(int arg_cnt, char **args){
  RtcTemperature temp = Rtc.GetTemperature();
  RtcDateTime now = Rtc.GetDateTime();

  Serial.print(temp.AsFloat());
  Serial.print("C ");
  printDateTime(now);
  p("\n");

}

void command_say_hello(int arg_cnt, char **args)
{
Serial.println("Hello.");
}
void command_help(int arg_cnt, char **args)
{
Serial.println("No Menu Help Yet.");
}

void command_add(int arg_cnt, char **args)
{
  if(strcmp(args[1],"schedule")){
    if(arg_cnt < 14){
      Serial.print("ERROR: Refusing to add Rule, Invalid argument count :");
      Serial.print(arg_cnt);
      return;
    }
    return schedule.add(args);

  }else{
    Serial.print("unknown ");
    Serial.println(args[1]);

  }

}

void poll(){

    loopCount++;
    if(loopCount == 25500){
      if (!Rtc.IsDateTimeValid()){
          Serial.println("RTC lost confidence in the DateTime!");
      }
      loopCount=0;
      schedule.poll();
      door1.poll();
    }
}




void RTCSetup(){
  //--------RTC SETUP ------------
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  //Rtc.SetDateTime(compiled);
  if (!Rtc.IsDateTimeValid())
  {
      Serial.println("RTC lost confidence in the DateTime!");
      Rtc.SetDateTime(compiled);
  }
  if (!Rtc.GetIsRunning())
  {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }
  RtcDateTime now = Rtc.GetDateTime();
  if (now > compiled)
  {
      Serial.println("RTC is older than compile time!  (Updating DateTime)");
      Rtc.SetDateTime(compiled);
  }
  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

void printDateTime(const RtcDateTime& dt)
{

  char datestring[20];

  snprintf_P(datestring,
      countof(datestring),
      PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
      dt.Month(),
      dt.Day(),
      dt.Year(),
      dt.Hour(),
      dt.Minute(),
      dt.Second() );
    Serial.print(datestring);
    Serial.println(dt.DayOfWeek());
    Serial.println(dt.Year());
}
void p(char *fmt, ... ){
        char buf[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 128, fmt, args);
        va_end (args);
        //rpi.print(buf);
        Serial.print(buf);
}
