#include <Arduino.h>
#include <Streaming.h>
#include "Users.h"
// #include "MD5.h"

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


void Users::add(int user_id, long pin, long rfid, uint8_t access_level, bool admin){
  Serial.print(F("Saving USER Entry to slot: "));
  Serial.println(users_count);

  users[users_count].user_id = user_id;
  users[users_count].pin = pin;
  users[users_count].rfid = rfid;
  users[users_count].access_level = access_level;
  users[users_count].admin = admin;

  users_count++;

};

int Users::lookupRFID(long rfid){
  for(uint8_t i =0; i < users_count; i++){

    // Serial.print("Compare");
    // Serial.print(users[i].rfid);
    // Serial.print(" to ");
    // Serial.println(rfid);

    if(users[i].rfid == rfid){
//      Serial.println(">> USER MATCH");
      return users[i].user_id;
    }
  }
  return -1;
}

int Users::lookupPIN(long pin){
  for(uint8_t i =0; i < users_count; i++){
    if(users[i].pin == pin) return users[i].user_id;
  }
  return -1;
}

void Users::list(){

  Serial.println(F("User List"));
  Serial.println(F("Index\tUser ID\tPin\tRFID\tAccess\tAdmin"));

  for(uint8_t i =0; i < users_count; i++){
    Serial.print(i);
    Serial.print("\t");
    Serial.print(users[i].user_id);

    Serial.print("\t");
    Serial.print(users[i].pin);
    Serial.print("\t");
    Serial.print(users[i].rfid);
    Serial.print("\t");
    Serial.print(users[i].access_level);
    Serial.print("\t");
    Serial.println(users[i].admin);
  }


};
