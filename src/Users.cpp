#include <Arduino.h>
#include <Streaming.h>
#include "Users.h"


void Users::clearAll(){
  users_count = 0;
}

void Users::loadFromMemory(uint8_t memConfigStart){
  users_count = EEPROM.read(memConfigStart);
  if(users_count > MAX_USERS){
    Serial.println(F("Saved Userlist larger than MAX_USER. Error"));
    users_count = 0;
  }
  for (unsigned int t=0; t<sizeof(users); t++)
  *((char*)&users + t) = EEPROM.read(memConfigStart + 1 + t);
}

void Users::save(uint8_t memConfigStart){

  EEPROM.write(memConfigStart, users_count);
  char *s = (char *) &users;

  for (unsigned int t=0; t<sizeof(users); t++)
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
  Serial.println(" Done.");

  Serial.print("Saved ");
  Serial.print(sizeof(users)+1);
  Serial.println(" bytes of data");

}


void Users::add(char pin_hash[33], long rfid, uint8_t access_level, bool admin){
  Serial.print(F("Saving USER Entry to slot: "));
  Serial.println(users_count);
  //users[users_count].pin_hash = pin_hash;
  users[users_count].rfid = rfid;
  users[users_count].access_level = access_level;
  users[users_count].admin = admin;

  users_count++;

};
