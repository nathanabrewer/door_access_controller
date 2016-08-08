
#include <Arduino.h>
#include <Streaming.h>
#include <Wire.h>  // must be incuded here so that Arduino library object file references work

#include "Scheduler.h"
#include "Users.h"
// #include "MD5.h"
#include "pins_arduino.h"
#include "config.h"


static uint8_t msg[80]; // command line message buffer and pointer
static uint8_t *msg_ptr;

unsigned long keypadBuffer = 0;

long authorized_timestamp;
int authorized_user = -1;

#define countof(a) (sizeof(a) / sizeof(a[0]))

//SoftwareSerial Serial_TTY(11, 12);
#if (DS3231 == 1)
  RtcDS3231 Rtc;
#else
  RtcDS1307 Rtc;
#endif

#if(BOARD_TYPE == MEGA_BOARD)
  #include <RCSwitch.h>
  RCSwitch mySwitch = RCSwitch();
  int last_rf_value;
  long last_rf_time;
#endif

int loopCount=0;
int rules_count = 0;

Scheduler schedule1;
Users users;
//MD5 md5;


volatile long readerBits = 0;
volatile int readerBitCount = 0;

int freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
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

void keypad_buzzer_off(){
  digitalWrite(KEYPAD_BUZZER, HIGH);
}
void keypad_buzzer_on(){
  digitalWrite(KEYPAD_BUZZER, LOW);
}
void keypad_led_off(){
  digitalWrite(KEYPAD_LED, HIGH);
}
void keypad_led_on(){
  digitalWrite(KEYPAD_LED, LOW);
}

void command_status(){
  #if DS3231
    RtcTemperature temp = Rtc.GetTemperature();
    Serial.print(F("Tempature of controller and RTC is currently "));
    Serial.print(temp.AsFloat());
    Serial.println("C ");
  #endif

  RtcDateTime now = Rtc.GetDateTime();
  schedule1.status();

  Serial.print(F("Current Time: "));
  printDateTime(now);
  Serial.println(" ");
  Serial.print(F("free SRAM: "));
  Serial.println(freeRam());
  Serial.println(" ");

}

void command_help()
{
    Serial.println(F("help\tthis menu"));
    Serial.println(F("list schedule\tlist current schedule"));
    Serial.println(F("list users\tlist current schedule"));
    Serial.println(F("add schedule\tadd to schedule"));
    Serial.println(F("add user\tadd a user"));
    Serial.println(F("clear schedule\tclear schedule"));
    Serial.println(F("clear users\tclear schedule"));
    Serial.println(F("status\tstats... open/close, time, temp"));
    Serial.println(F("set time <SecondsSince2000>\tSet the Time, Duh"));
    //JS: Math.floor((b.getTime() - a.getTime())/1000)-(7*60*60);
}

void command_save(){
  Serial.println(F("Saving Schedule..."));
  schedule1.save(1);
  users.save(300);
}



void showCode(unsigned long code, bool rfid)
{
  if(rfid){
    Serial.print(F("RX RFID >> "));
    authorized_user = users.lookupRFID(code);
  }else{
    Serial.print(F("RX KEYPAD >> "));
    authorized_user = users.lookupPIN(code);
  }
  Serial.println(code);

  //see if we got a user we know.
  if( authorized_user > -1){
      keypad_led_on();
      authorized_timestamp = millis();
      schedule1.guestEntraceRequest();
      // Success on User AUTH, Turn keypad GREEN
  }

}



void keypadOk(){
  digitalWrite(KEYPAD_LED, LOW);
  digitalWrite(KEYPAD_BUZZER, LOW);
  delay(100);
  digitalWrite(KEYPAD_BUZZER, HIGH);
  delay(600);

}

void keypadErr(){
  int8_t i;
  for(i=1; i<3; i++){
    digitalWrite(KEYPAD_LED, LOW);
    digitalWrite(KEYPAD_BUZZER, LOW);
    delay(100);
    digitalWrite(KEYPAD_LED, HIGH);
    digitalWrite(KEYPAD_BUZZER, HIGH);
    delay(200);
  }
}

void welcomeKeypad(){
  pinMode(KEYPAD_BUZZER, OUTPUT);
  pinMode(KEYPAD_LED, OUTPUT);
  int8_t i;
  for(i=1; i<5; i++){
    keypadOk();
  }
  digitalWrite(KEYPAD_LED, HIGH);
  digitalWrite(KEYPAD_BUZZER, HIGH);
}


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

  if (memcmp(argv[0], "list", 4) == 0){

    if (memcmp(argv[1], "users", 5) == 0){
      users.list();
      return;
    }

      schedule1.list();
      return;
  }
  if (memcmp(argv[0], "add", 3) == 0){

    if (memcmp(argv[1], "user", 4) == 0){
      //Users::add(int user_id, long pin, long rfid, uint8_t access_level, bool admin)
      //md5.make_hash(
      users.add(atoi(argv[2]), atol(argv[3]), atol(argv[4]), atoi(argv[5]), atoi(argv[6]) );
      users.list();
      return;
    }
    if (memcmp(argv[1], "schedule", 8) == 0){
      Serial.println("Try +SCH instead");
      return;
    }

  }
  if (memcmp(argv[0], "clear", 5) == 0){
    if (memcmp(argv[1], "users", 5) == 0){
      Serial << "Clearing Users..." << endl;
      users.clearAll();
      return;
    }
    if (memcmp(argv[1], "schedule", 8) == 0){
      Serial << "Clearing Schedule..." << endl;
      schedule1.clearAll();
      return;
    }
  }

  if (memcmp(argv[0], "status", 6) == 0)
    return command_status();

  if (memcmp(argv[0], "kpOK", 4) == 0){
    keypadOk();
    return;
  }
  if (memcmp(argv[0], "kpERR", 5) == 0){
    keypadErr();
    return;
  }
  if (memcmp(argv[0], "remove", 6) == 0){
    if (memcmp(argv[1], "schedule", 8) == 0){
      schedule1.remove( atoi(argv[2]) );
      return;
    }
  }

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
    int8_t c = NUM_OF_DOORS+10;
    if(argv[c][0] != 'X' && argv[c][0] != 'F'){
      Serial.println(F("General Error with formatting +SCH command."));
      Serial.println(F("Example:"));
      Serial.println(F("+SCH 2016 05 255 255 01 00 05 00 LU LU LU LU O X"));
      Serial.println(F("+SCH 255 255 255 255 00 00 23 59 LA LA LA LA A X"));
//+SCH 255 255 255 255 00 00 23 59 LA LA LA LA LA LA LA LA A X
//+SCH 255 255 255 255 00 00 23 00 LA LA LA LA LA LA LA LA A X
//+SCH 16 7 18 255 20 22 23 00 LA LA LA LA LA LA LA LA A F
//+SCH 16 7 255 255 20 22 23 00 LA LA LA LA LA LA LA LA A F

//+SCH 16 255 255 255 00 00 23 59 UC UC UC UC UU UU UU UU U X
//+SCH 16 255 255 255 00 00 23 59 UC UC UC UC UU UU UU UU U X
//+SCH 16 255 255 255 00 00 19 59 XX XC FA UA IC UI II II U X

//+SCH year month day dow bh bm eh em d0 d1
      return;
    }
    schedule1.add(argv);
    return;
  }

  Serial.println(F("Unknown Command (try help)"));
  Serial.println(F("---------------------------------"));

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




void RTCSetup(){
  //--------RTC SETUP ------------
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.println("BUILD DATE TIME:");
  Serial.println(__DATE__);
  Serial.println(__TIME__);

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
      Serial.println(F("RTC is older than compile time!  (Updating DateTime)"));
      Rtc.SetDateTime(compiled);
  }
  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state

  //Rtc.Enable32kHzPin(false);
  //Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
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
    Serial.println();
    Serial.println(F("READY."));
}


//WIEGAND... read bits, called by interrupts
void readerOne(void) {
  if(digitalRead(WIEGAND_DATA1_PIN) == LOW){
   readerBitCount++;
   readerBits = readerBits << 1;  //Move the bits ...
   readerBits |= 1;  // ... add a bit to '1 'in the least significant bit
  }
}

void readerZero(void) {
  if(digitalRead(WIEGAND_DATA0_PIN) == LOW){
   readerBitCount++;
   readerBits = readerBits << 1;  //Move the bits ...
  }
}


#if(BOARD_TYPE == UNO_BOARD || BOARD_TYPE == NUMATO_BOARD)

/*
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


#endif


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
      }
    i++;
    temp[i]='\0';
    longv = atol (temp);
    return longv;
  }


  void prepWiegand(){
      for(int i = WIEGAND_DATA1_PIN; i<=WIEGAND_DATA0_PIN; i++){
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
          showCode(cardID, true);
          readerBits = 0;
          readerBitCount = 0;

          keypadBuffer = 0;


          //prepWiegand();
          return;
        }

        if(readerBitCount == 4){
          int data = (int) readerBits & 0x0000000F;

          if(authorized_user != -1){

              //authorized user is interacting with keypad... extend the timeout
              authorized_timestamp = millis();


              Serial.print("Authorized User >> ");
              Serial.print(authorized_user);
              Serial.print(" >> entered: ");
              Serial.print(data);
              Serial.println();

              //reset bits
              readerBits = 0;
              readerBitCount = 0;
              return;
          }

          switch (data)
          {
            case 0:
              keypadBuffer = keypadBuffer*10;
            break;
            case 11:
              showCode(keypadBuffer, false);
              keypadBuffer = 0;
            break;
            case 10:
                // escape... clear buffer
                keypadBuffer = 0;
            break;

            default:
                keypadBuffer = (keypadBuffer*10)+data;
                // normal character entered. add it to the buffer
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



void setup()
{

  Serial.begin(BAUD_RATE);

  Serial.print(F("BOARD_TYPE: "));
  Serial.println(BOARD_TYPE);

  Serial.print(F("NUM_OF_DOORS: <"));
  Serial.print(NUM_OF_DOORS);
  Serial.println(">");

  Serial.println(F("Int Keypad"));
  welcomeKeypad();

  #if(BOARD_TYPE == MEGA_BOARD)
    #include <RCSwitch.h>
    Serial.print("Int Radio rx");
    mySwitch.enableReceive(5);  // pin 18
    //	2	3	21	20	19	18
    //0, 1, 2,   3,  4,  5
  #endif

  msg_ptr = msg;
  RTCSetup();

  schedule1.loadFromMemory(1);
  schedule1.init();
  users.loadFromMemory(300);


  cmd_display();

  //KEYPAD

  #if(BOARD_TYPE == UNO_BOARD || BOARD_TYPE == NUMATO_BOARD)
    PCattachInterrupt(WIEGAND_DATA1_PIN, readerOne, CHANGE);
    PCattachInterrupt(WIEGAND_DATA0_PIN, readerZero, CHANGE);
  #else
    attachInterrupt(digitalPinToInterrupt(WIEGAND_DATA1_PIN), readerOne, CHANGE);
    attachInterrupt(digitalPinToInterrupt(WIEGAND_DATA0_PIN), readerZero, CHANGE);
  #endif
  delay(10);

  // put the reader input variables to zero
  readerBits=0;
  readerBitCount = 0;
  prepWiegand();

}
void loop()
{
  handle_keypad_buffer();

  if (Serial.available())
  {
      cmd_handler();
  }

  #if(BOARD_TYPE == MEGA_BOARD)
      if (mySwitch.available()) {
        int value = mySwitch.getReceivedValue();
        if (value == 0) {
          Serial.print("Unknown encoding");
        } else {
          if(last_rf_value != value || last_rf_value == value && (millis() - last_rf_time > 1000) ){
            last_rf_value = value;
            last_rf_time = millis();
             Serial.print("RF <");
             Serial.print( mySwitch.getReceivedValue() );
             Serial.print("> <");
             Serial.print( mySwitch.getReceivedBitlength() );
             Serial.print("> <");
             Serial.print( mySwitch.getReceivedProtocol() );
             Serial.println(">");

          }
        }
        mySwitch.resetAvailable();
      }
  #endif

  loopCount++;
  if(loopCount == 25500){

      //make sure buzzer is off
      keypad_buzzer_off();

    if (!Rtc.IsDateTimeValid()){
        //Serial.println("RTC lost confidence in the DateTime!");
    }
    loopCount=0;
    RtcDateTime now = Rtc.GetDateTime();
    schedule1.poll(now);
  }

  if(authorized_timestamp > 0){
    if(millis() - authorized_timestamp < 8000){

      keypad_led_on();

    }else{
      authorized_timestamp = 0;
      authorized_user = -1;
      keypad_led_off();
    }
  }else{
    //not authorized .. or timedout
    keypad_led_off();
  }

}
