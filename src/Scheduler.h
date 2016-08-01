#include "door.h"
#include "config.h"


class ScheduleDoor {
public:
  uint8_t relay;
  uint8_t sensor;
};

class SiteState{
  public:
    uint8_t matching_rules;
    ScheduleDoor doors[NUM_OF_DOORS];
    uint8_t env_flag;
};

class ScheduleMetricType{
  public:
    uint8_t metric;
    bool match;
    int minutes_since;
    int minutes_until;
};



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
    ScheduleDoor doors[NUM_OF_DOORS];
    uint8_t env_flag;
    uint8_t rule_flag;
};


class Scheduler{
private:
  #if (DS3231 == 1)
    RtcDS3231 _RTC;
  #else
    RtcDS1307 _RTC;
  #endif
    uint8_t doorScheduleState;
    uint8_t rules_count;
    DoorSensor doors[NUM_OF_DOORS];
    uint8_t door_count;
    ScheduleType schedule[MAX_SCHEDULE_SIZE];
    ScheduleMetricType schedule_metric[MAX_SCHEDULE_SIZE];
    uint8_t SortIndex[MAX_SCHEDULE_SIZE];
    uint8_t SortMetric[MAX_SCHEDULE_SIZE];
    SiteState resolved_state;
    SiteState current_state;
    int minutes_till_open;
    int minutes_till_close;
    uint8_t resolveRelayState(char key);
    uint8_t resolveSensorState(char key);
public:
    void poll(RtcDateTime dt);
    void init();
    void setDoorState(uint8_t door, uint8_t state );
    void sort();
    void evalState(RtcDateTime dt);
    void add(char **args);
    void list();
    void save(uint8_t memConfigStart);
    void loadFromMemory(uint8_t memConfigStart);
    void clearAll();
    void status();
    void resetSortIndex();
    void remove(uint8_t index);
    void guestEntraceRequest();
};
