#include <RtcDS3231.h>

#define DOOR_SCHEDULE_STATE_UNLOCKED 0
#define DOOR_SCHEDULE_STATE_LOCKED 1

#define MAX_SCHEDULE_SIZE 10



class ScheduleType {
  public:
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t dow;
    uint8_t open_hour;
    uint8_t open_min;
    uint8_t close_hour;
    uint8_t close_min;
    uint8_t door1_relay;
    uint8_t door1_sensor;
    uint8_t door2_relay;
    uint8_t door2_sensor;
    uint8_t door3_relay;
    uint8_t door3_sensor;
    uint8_t door4_relay;
    uint8_t door4_sensor;
    uint8_t env_flag;
    uint8_t rule_flag;
    uint8_t metric;
    bool match;
};


class Scheduler{
private:
    RtcDS3231 _RTC;
    uint8_t doorScheduleState;
    uint8_t rules_count;
    uint8_t resolveDayStateOfSchedule(RtcDateTime dt, ScheduleType rule);
    ScheduleType schedule[MAX_SCHEDULE_SIZE];
    int minutes_till_open;
    int minutes_till_close;
    uint8_t resolveRelayState(char key);
    uint8_t resolveSensorState(char key);
public:
    void poll(RtcDateTime dt);
    void sort();
    uint8_t getState(RtcDateTime dt);
    void add(char **args);
    void list();
    void save(uint8_t memConfigStart);
    void loadFromMemory(uint8_t memConfigStart);
    void clearAll();
    void status();
};
