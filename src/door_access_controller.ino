
#include <Arduino.h>
#include <Streaming.h>
#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDateTime.h>
//#include <RtcDS1307.h>

#include <RtcDS3231.h>
//#include <RtcTemperature.h>
#include <RtcUtility.h>

#include "Scheduler.h"
#include <SoftwareSerial.h>

#include "pins_arduino.h"
#include <avr/pgmspace.h>

#define KEYPAD_BUZZER 11
#define KEYPAD_LED 12

static uint8_t msg[60]; // command line message buffer and pointer
static uint8_t *msg_ptr;

static uint8_t wgmsg[10];
static uint8_t *wg_msg_ptr;

#define countof(a) (sizeof(a) / sizeof(a[0]))

//SoftwareSerial Serial_TTY(11, 12);
RtcDS3231 Rtc;
//RtcDS1307 Rtc;

int loopCount=0;
int rules_count = 0;

// DoorSensor door1;
// DoorSensor door2;
// DoorSensor door3;
// DoorSensor door4;

Scheduler schedule1;

//Keypad keypad;


const int WiegandData1 = A6;
const int WiegandData0 = A7;
volatile long readerBits = 0;
volatile int readerBitCount = 0;


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
  handle_keypad_buffer();

  if (Serial.available())
  {
      cmd_handler();
  }
  // if(Serial_TTY.available())
  // {
  //   tty_handler();
  // }

  loopCount++;
  if(loopCount == 25500){
    if (!Rtc.IsDateTimeValid()){
        //Serial.println("RTC lost confidence in the DateTime!");
    }
    loopCount=0;
    RtcDateTime now = Rtc.GetDateTime();

    schedule1.poll(now);

    // door1.poll();
    // door2.poll();
    // door3.poll();
    // door4.poll();

    //keypad.poll();
  }
}



void command_status(){
  //RtcTemperature temp = Rtc.GetTemperature();
  RtcDateTime now = Rtc.GetDateTime();
  schedule1.status();

  //Serial.print(F("Tempature of controller and RTC is currently "));
  //Serial.print(temp.AsFloat());
  //Serial.println("C ");

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

  //Rtc.Enable32kHzPin(false);
  //Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
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
    if (memcmp(argv[1], "lock", 4) == 0){
      int8_t d = atol(argv[2]);
      //RELAY_STATE_LOCKED
      schedule1.setDoorState(d, 2);
      return;
    }
    if (memcmp(argv[1], "unlock", 6) == 0){
      int8_t d = atol(argv[2]);
      //RELAY_STATE_UNLOCKED
      schedule1.setDoorState(d, 1);
      return;
    }
    if (memcmp(argv[1], "time", 3) == 0){
          int32_t t = atol(argv[2]);

          RtcDateTime settime = RtcDateTime(t);
          Serial.print("Setting Date Time: ");
          Serial.println(t);
          Rtc.SetDateTime(settime);
          return;
    }
    if (memcmp(argv[1], "compiled", 8) == 0){
      RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
      Rtc.SetDateTime(compiled);
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

// void tty_handler()
// {
//   char c = Serial_TTY.read();
//   switch (c)
//   {
//   case '\n':
//       *msg_ptr = '\0';
//       cmd_parse((char *)msg);
//       msg_ptr = msg;
//       break;
//   default:
//       *msg_ptr++ = c;
//       break;
//   }
// }

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













//***************** vv Code for managing interruptions vv ********************
  /*
    All terminals can generate interrupts on ATmega168 transition.
    The bit corresponding to the terminal to be the source of the event should
    be enabled in the registry correspond PCInt and a service routine (ISR).
    Because the registration PCInt operates port, not terminal, the routine ISR
    must use some algorithm to implement an interrupt service routine
    by terminal
    Correspondenicas terminal and to interrupt masking registers:
      D0-D7 => 16-23 = PCIR2 PCInt = PD = PCIE2 = pcmsk2
      D8-D13 => 0-5 = PCIR0 PCInt = PB = PCIE0 = pcmsk0
      A0-A5 (D14-D19) => 8-13 = PCIR1 PCInt = PC = PCIE1 = pcmsk1
   */

  volatile uint8_t *port_to_pcmask[] = { &PCMSK0, &PCMSK1, &PCMSK2 };

  typedef void (*voidFuncPtr)(void);
  volatile static voidFuncPtr PCintFunc[24] = { NULL };
  volatile static uint8_t PCintLast[3];

  void PCattachInterrupt(uint8_t pin, void (*userFunc)(void), int mode) {
      uint8_t bit = digitalPinToBitMask(pin);
      uint8_t port = digitalPinToPort(pin);
      uint8_t slot;
      volatile uint8_t *pcmask;

      if (mode != CHANGE) {
      return;
      }
      // Modify the registry ("Interrupt Control Register") ICR
      // As requested terminal, validating that is between 0 and 13


      if (port == NOT_A_PORT) {
        return;
      }else{
        port -= 2;
        pcmask = port_to_pcmask[port];
      }

      slot = port * 8 + (pin % 8);
      PCintFunc[slot] = userFunc;

      // Set the interrupt mask
      *pcmask |= bit;

      // Interrupt Enable
      PCICR |= 0x01 << port;
  }

  static void PCint(uint8_t port) {
      uint8_t bit;
      uint8_t curr;
      uint8_t mask;
      uint8_t pin;

      // get the pin states for the indicated port.
      curr = *portInputRegister(port+2);
      mask = curr ^ PCintLast[port];
      PCintLast[port] = curr;
      // mask is pins that have changed. screen out non pcint pins.
      if ((mask &= *port_to_pcmask[port]) == 0) {
      return;
      }
      // mask is pcint pins that have changed.
      for (uint8_t i=0; i < 8; i++) {
      bit = 0x01 << i;
      if (bit & mask) {
        pin = port * 8 + i;
        if (PCintFunc[pin] != NULL) {
        PCintFunc[pin]();
        }
      }
      }
  }

  SIGNAL(PCINT0_vect) {
    PCint(0);
  }
  SIGNAL(PCINT1_vect) {
    PCint(1);
  }
  SIGNAL(PCINT2_vect) {
    PCint(2);
  }

  //***************** ^^ Code for managing interruptions ^^ ********************
  //*********** vv Code for counting and storing bits vv **********
  void readerOne(void) {
    if(digitalRead(WiegandData1) == LOW){
     readerBitCount++;
     readerBits = readerBits << 1;  //Move the bits ...
     readerBits |= 1;  // ... add a bit to '1 'in the least significant bit
    }

  }

  void readerZero(void) {
    if(digitalRead(WiegandData0) == LOW){
     readerBitCount++;
     readerBits = readerBits << 1;  //Move the bits ...

    }

  }


  //*********** ^^ Code for counting and storing bits ^^ **********




  void prepWiegand(){
      for(int i = WiegandData1; i<=WiegandData0; i++){
         pinMode(i, OUTPUT);
         digitalWrite(i, HIGH); // enable internal pull up causing a one
         digitalWrite(i, LOW);  // disable internal pull up causing zero and thus an interrupt
         pinMode(i, INPUT);
         digitalWrite(i, HIGH); // enable internal pull up
      }
      delay(10);
  }

  unsigned long cardID;
  unsigned long _lastWiegand = 0;


  void handle_keypad_buffer(){
    unsigned long sysTick = millis();

  	if ((sysTick - _lastWiegand) > 300){
      if ((readerBitCount==26) || (readerBitCount==4)) {

        if(readerBitCount == 26){
          cardID = (readerBits & 0x1FFFFFE) >>1;
          showCode(cardID);
          readerBits = 0;
          readerBitCount = 0;

          *wg_msg_ptr = '\0';
          wg_msg_ptr = wgmsg;

          //prepWiegand();
          return;
        }

        if(readerBitCount == 4){
          int data = (int) readerBits & 0x0000000F;

          switch (data)
          {
            case 11:
                *wg_msg_ptr = '\0';
                showCode(keypadToLong((char *)wgmsg));
                wg_msg_ptr = wgmsg;
                break;

            case 10:
                // escape... clear buffer
                *wg_msg_ptr = '\0';
                wg_msg_ptr = wgmsg;
            break;

            default:
                // normal character entered. add it to the buffer
                *wg_msg_ptr++ = data;
                break;
          }

          readerBits = 0;
          readerBitCount = 0;
          //prepWiegand();
          return;
        }

      }else{

        // if(readerBitCount > 2){
        //   Serial.print("Size: ");
        //   Serial.println(readerBitCount, DEC);
        //   unsigned long data = (readerBits & 0x1FFFFFE) >>1;
        //   Serial.print(data, DEC);
        //   Serial.print("-");
        //   Serial.println(data, BIN);
        // }
        readerBits = 0;
        readerBitCount = 0;
        //prepWiegand();
      }
        _lastWiegand=sysTick;
  }

}



void welcomeKeypad(){
  pinMode(KEYPAD_BUZZER, OUTPUT);
  pinMode(KEYPAD_LED, OUTPUT);
  int8_t i;
  for(i=1; i<5; i++){
    digitalWrite(KEYPAD_LED, LOW);
    digitalWrite(KEYPAD_BUZZER, LOW);
    delay(100);
    digitalWrite(KEYPAD_LED, HIGH);
    digitalWrite(KEYPAD_BUZZER, HIGH);
    delay(200);
  }
  digitalWrite(KEYPAD_LED, HIGH);
  digitalWrite(KEYPAD_BUZZER, HIGH);
}

void setup()
{
  welcomeKeypad();

  Serial.begin(57600);

  msg_ptr = msg;
  wg_msg_ptr = wgmsg;

  RTCSetup();
  //keypad =  Keypad();
  // door1.setPin(A0, 2);
  // door2.setPin(A1, 3);
  // door3.setPin(A2, 9);
  // door4.setPin(A3, 7);

  schedule1.loadFromMemory(1);
  schedule1.init();


  cmd_display();

  //KEYPAD
  PCattachInterrupt(WiegandData1, readerOne, CHANGE);
  PCattachInterrupt(WiegandData0, readerZero, CHANGE);
  delay(10);

  // put the reader input variables to zero
  readerBits=0;
  readerBitCount = 0;
  prepWiegand();

}


unsigned long keypadToLong(char *cmd)
{
  //YUCK, this is UGLY! ...but it works for now
  //having issues with null terminated \0 ...it is truncating a keypad code at zero
  int8_t i, v, l;
  unsigned long longv;
  l = strlen(cmd);
  char temp[10];
  char t[2];
  for(i=0; i< l;i++){
    v = cmd[i];
    if(v > 47) v = v-48; //bring back to actual value
    cmd[i] = v;
    sprintf(t,"%d",v);
    temp[i] = t[0];
    //Serial.println(v);
  }
  i++;
  temp[i]='\0';
  longv = atol (temp);
  return longv;
}

void showCode(unsigned long code)
{

  if(code == 6321032){
    Serial.println(F("Welcome User XYZ"));
  }
  if(code == 8818){
    Serial.println(F("Hey Nate!"));
  }

  Serial.print(F("RX KEY/CARD >> "));
  char tmp[12];
  ultoa(code, tmp, 10);
  Serial.println(code);
}
