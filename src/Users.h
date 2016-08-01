#include "config.h"

class UserObject {
public:
  bool admin;
  int user_id;
  uint8_t access_level;
  long pin;
  long rfid;
};

class Users{
private:
    UserObject users[MAX_USERS];
    uint8_t users_count;
public:
    int lookupPIN(long pin);
    int lookupRFID(long rfid);
    void add(int user_id, long pin, long rfid, uint8_t access_level, bool admin);
    void clearAll();
    void save(uint8_t memConfigStart);
    void loadFromMemory(uint8_t memConfigStart);
    void list();

};
