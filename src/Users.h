#include "config.h"

class UserObject {
public:
  bool admin;
  uint8_t access_level;
  char pin_hash[33];
  long rfid;
};

class Users{
private:
    UserObject users[MAX_USERS];
    uint8_t users_count;
public:
    void lookupPIN_HASH(char hash[33]);
    void lookupRFID(long rfid);
    void add(char pin_hash[33], long rfid, uint8_t access_level, bool admin);
    void clearAll();
    void save(uint8_t memConfigStart);
    void loadFromMemory(uint8_t memConfigStart);
};
