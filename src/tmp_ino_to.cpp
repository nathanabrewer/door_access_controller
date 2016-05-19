#include <Arduino.h>
#line 1 "/Volumes/Brewer/Code/PlatformIO/door_access_controller/src/door_access_controller.ino"
#include <Arduino.h>
#include <LinkedList.h>
#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDateTime.h>
#include <RtcDS1307.h>
#include <RtcDS3231.h>
#include <RtcTemperature.h>
#include <RtcUtility.h>
#include <door.h>
//#include <keypad.h>

#include <scheduler.h>


typedef struct _cmd_t
{
    char *cmd;
    void (*func)(int argc, char **argv);
    struct _cmd_t *next;
} cmd_t;
static uint8_t msg[50]; // command line message buffer and pointer
static uint8_t *msg_ptr;

// linked list for command table
static cmd_t *cmd_tbl_list, *cmd_tbl;


void cmdAdd(char *name, void (*func)(int argc, char **argv))
{
    cmd_tbl = (cmd_t *)malloc(sizeof(cmd_t));// alloc memory for command struct
    char *cmd_name = (char *)malloc(strlen(name)+1);// alloc memory for command name
    strcpy(cmd_name, name);// copy command name
    cmd_name[strlen(name)] = '\0';// terminate the command name
    cmd_tbl->cmd = cmd_name;// fill out structure
    cmd_tbl->func = func;
    cmd_tbl->next = cmd_tbl_list;
    cmd_tbl_list = cmd_tbl;
}


/*
add sch 1 1 2016 05 15 -1 0 03 58 4 19 0
add sch 2 1 -1 -1 -1 4 0 05 58 8 19 0
add sch 4 1 -1 -1 -1 -1 0 2 30 5 54 1
add sch 0 0 0 0 0 0 0 0 0 0 0 0

[trueindex  0 1 1 2016 5 15 255 0 358 419 0
[trueindex  1 1 1 2016 5 15 255 0 358 419 0
[trueindex  2 1 1 2016 5 15 255 0 358 419 0
[trueindex  3 1 1 2016 5 15 255 0 358 419 0
[trueindex  4 1 4 -1 255 255 255 0 122 554 1
[trueindex  5 1 4 -1 255 255 255 0 122 554 1
[trueindex  6 10 10 10 10 10 10 10 1010 1010 20

*/

#define countof(a) (sizeof(a) / sizeof(a[0]))

RtcDS3231 Rtc;

int loopCount=0;
int rules_count = 0;

DoorSensor door1;
DoorSensor door2;
Scheduler schedule1;
//Keypad *keypad = new Keypad;


void setup();
void command_help(int arg_cnt, char **args);

void command_save(int arg_ctn, char **args);
void command_clear(int arg_cnt, char **args);

void command_list(int arg_cnt, char **args);

void loop();
void command_settime(int arg_cnt, char **args);

void command_status(int arg_cnt, char **args);

void command_add(int arg_cnt, char **args);






void RTCSetup();

void printDateTime(const RtcDateTime& dt);

void p(char *fmt, ... );








void cmd_display();


void cmd_parse(char *cmd);


void cmd_handler();
#line 70 "/Volumes/Brewer/Code/PlatformIO/door_access_controller/src/door_access_controller.ino"
void setup()
{
  msg_ptr = msg;
  cmd_tbl_list = NULL;
  Serial.begin(57600);


  RTCSetup();
  //keypad =  Keypad();
  door1.setPin(A0);
  door2.setPin(A1);

  schedule1.loadFromMemory(1);

  cmdAdd("add", command_add);
  cmdAdd("help", command_help);
  cmdAdd("list", command_list);
  cmdAdd("status", command_status);
  cmdAdd("clear", command_clear);
  cmdAdd("save", command_save);
  cmdAdd("settime", command_settime);


  cmd_display();

}
void command_help(int arg_cnt, char **args)
{
    Serial.println("help me         - this menu");
    Serial.println("list all        - list current schedule");
    Serial.println("add sch         - add to schedule");
    Serial.println("clear all       - clear schedule");
    Serial.println("status all      - stats... open/close, time, temp");

}

void command_save(int arg_ctn, char **args){
  Serial.println("Saving Schedule...");
  schedule1.save(1);
}
void command_clear(int arg_cnt, char **args){
  Serial.println("Clearing Schedule...");
  schedule1.clearAll();
}

void command_list(int arg_cnt, char **args){
  schedule1.list(args);
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
  }

  //keypad->poll();
  //sensorB->poll();
  //delay(100);
}
void command_settime(int arg_cnt, char **args){
  RtcDateTime settime = RtcDateTime(args[1], args[2]);
  Serial.println("Setting Date Time");
  Rtc.SetDateTime(settime);

}

void command_status(int arg_cnt, char **args){
  RtcTemperature temp = Rtc.GetTemperature();
  RtcDateTime now = Rtc.GetDateTime();
  schedule1.status();


  Serial.print("Tempature of controller and RTC is currently ");
  Serial.print(temp.AsFloat());
  Serial.println("C ");
  Serial.print("Current Time: ");
  printDateTime(now);
  Serial.println(" ");
  Serial.println(" ");

}

void command_add(int arg_cnt, char **args)
{
  if(strcmp(args[1],"schedule")){
    if(arg_cnt < 14){
      Serial.println("Not a valid add command, Usage: ");
      Serial.println(">> add sch <i> <rg> <yr> <mnth> <day> <dow> <oH> <oM> <cH> <cM> <lr>");
      Serial.println("");
      Serial.println("");
      return;
    }
    return schedule1.add(args);

  }else{
    Serial.print("unknown ");
    Serial.println(args[1]);

  }

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


void cmd_parse(char *cmd)
{
    uint8_t argc, i = 0;
    char *argv[30];
    char buf[50];
    cmd_t *cmd_entry;
    // parse the command line statement and break it up into space-delimited
    // strings. the array of strings will be saved in the argv array.
    argv[i] = strtok(cmd, " ");
    do
    {
        argv[++i] = strtok(NULL, " ");
    } while ((i < 30) && (argv[i] != NULL));

    // save off the number of arguments for the particular command.
    argc = i;

    // parse the command table for valid command. used argv[0] which is the
    // actual command name typed in at the prompt
    for (cmd_entry = cmd_tbl; cmd_entry != NULL; cmd_entry = cmd_entry->next)
    {
        if (strcmp(argv[0], cmd_entry->cmd) == 0)
        {
            cmd_entry->func(argc, argv);
            cmd_display();
            return;
        }
    }
    Serial.println("Unknown Command");
    cmd_display();
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