#include <LinkedList.h>
#include <RtcDS3231.h>

#define DOOR_SCHEDULE_STATE_UNLOCKED 0
#define DOOR_SCHEDULE_STATE_LOCKED 1

class ScheduleType {
  public:
    int relaygroup;
    int index;
    int year;
    int month;
    int day;
    int dayofweek;
    int closed_all_day;
    int open_hour;
    int open_min;
    int close_hour;
    int close_min;
    int last;
};


class Scheduler{
private:
    RtcDS3231 _RTC;
    int doorScheduleState;
    int rules_count;
    ScheduleType schedule[20];
public:
    void poll();
    int getState();
    void add(char **args);
    void list(char **args);
};
