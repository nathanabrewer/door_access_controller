
#include <Arduino.h>
#include <Streaming.h>
#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDateTime.h>
// #include <RtcDS1307.h>
#include <RtcDS3231.h>
#include <RtcTemperature.h>
#include <RtcUtility.h>

#include <door.h>
#include "keypad.h"
#include "scheduler.h"

#include <avr/pgmspace.h>


static uint8_t msg[60]; // command line message buffer and pointer
static uint8_t *msg_ptr;

#define countof(a) (sizeof(a) / sizeof(a[0]))

RtcDS3231 Rtc;

int loopCount=0;
int rules_count = 0;

DoorSensor door1;
DoorSensor door2;
Scheduler schedule1;

Keypad keypad;


void setup()
{
  msg_ptr = msg;

  Serial.begin(57600);


  RTCSetup();
  //keypad =  Keypad();
  door1.setPin(A0);
  door2.setPin(A1);

  schedule1.loadFromMemory(1);


  cmd_display();

}
void command_help()
{
    Serial.println(F("help\tthis menu"));
    Serial.println(F("list\tlist current schedule"));
    Serial.println(F("add\tadd to schedule"));
    Serial.println(F("clear\tclear schedule"));
    Serial.println(F("status\tstats... open/close, time, temp"));
    Serial.println(F("set time <SecondsSince2000>\tSet the Time, Duh"));
    //JS: Math.floor((b.getTime() - a.getTime())/1000)-(7*60*60);
}

void command_save(){
  Serial << "Saving Schedule..." << endl;
  schedule1.save(1);
}
void command_clear(){
  Serial << "Clearing Schedule..." << endl;
  schedule1.clearAll();
}

void command_list(){
  schedule1.list();
}



void loop()
{
  while (Serial.available())
  {
      cmd_handler();
  }
  loopCount++;
  if(loopCount == 25500){
    if (!Rtc.IsDateTimeValid()){
        Serial.println("RTC lost confidence in the DateTime!");
    }
    loopCount=0;
    schedule1.poll();
    door1.poll();
    keypad.poll();
  }
}



void command_status(){
  RtcTemperature temp = Rtc.GetTemperature();
  RtcDateTime now = Rtc.GetDateTime();
  schedule1.status();


  Serial.print(F("Tempature of controller and RTC is currently "));
  Serial.print(temp.AsFloat());
  Serial.println("C ");
  Serial.print(F("Current Time: "));
  printDateTime(now);
  Serial.println(" ");
  Serial.print(F("free SRAM: "));
  Serial.println(freeRam());
  Serial.println(" ");

}


void RTCSetup(){
  //--------RTC SETUP ------------
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.println("BUILD DATE TIME:");
  Serial.println(__DATE__);
  Serial.println(__TIME__);

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








void cmd_display()
{
    char buf[50];
    Serial.println();
    Serial.println("DOOR CONTROLLER");
    Serial.println("");
    Serial.println("");
    Serial.print("# ");
}

const char testcommand[] = "TEST";
void cmd_parse(char *cmd)
{
  uint8_t i;
  char *argv[30];

  argv[i] = strtok(cmd, " ");
  do { argv[++i] = strtok(NULL, " ");
  } while ((i < 30) && (argv[i] != NULL));

  if (memcmp(argv[0], "save", 4) == 0)
    return command_save();

  if (memcmp(argv[0], "help", 4) == 0)
    return command_help();

  if (memcmp(argv[0], "list", 4) == 0)
    return command_list();

  if (memcmp(argv[0], "clear", 5) == 0)
    return command_clear();

  if (memcmp(argv[0], "status", 6) == 0)
    return command_status();

  if (memcmp(argv[0], "set", 3) == 0){
    if (memcmp(argv[1], "time", 3) == 0){
          int32_t t = atol(argv[2]);

          RtcDateTime settime = RtcDateTime(t);
          Serial.print("Setting Date Time: ");
          Serial.println(t);
          Rtc.SetDateTime(settime);
          return;
    }
  }

  if (memcmp(argv[0], "+SCH", 3) == 0) {
    if(argv[14][0] != 'X' && argv[14][0] != 'F'){
      Serial.println(F("General Error with formatting +SCH command."));
      Serial.println(F("Example:"));
      Serial.println(F("+SCH 2016 05 255 255 01 00 05 00 LU LU LU LU O X"));
      Serial.println(F("+SCH 255 255 255 255 00 00 23 59 LA LA LA LA A X"));

      return;
    }
    schedule1.add(argv);
    return;

    // char *ptr = &cmd[0];
    // ptr = &cmd[5];
    //     char year[5];
    //     memcpy ( year, ptr, 4 );
    //     year[5] = '\0';
    //     ptr += 5;
  }

  Serial.println(F("Unknown Command (try help)"));
  Serial.println(F("---------------------------------"));

}

int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void cmd_handler()
{
    char c = Serial.read();
    switch (c)
    {
    case '\n':
        // terminate the msg and reset the msg ptr. then send
        // it to the handler for processing.
        *msg_ptr = '\0';
        Serial.print("\r\n");
        cmd_parse((char *)msg);
        msg_ptr = msg;
        break;

    case '\b':
        // backspace
        Serial.print(c);
        if (msg_ptr > msg)
        {
            msg_ptr--;
        }
        break;

    default:
        // normal character entered. add it to the buffer
        Serial.print(c);
        *msg_ptr++ = c;
        break;
    }
}
