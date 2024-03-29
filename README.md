# Supported Devices
1. Arduino uno
2. Arduino Mega 2560
3. Numato Programmable 4 channel Relay
4. Wemos mini D1 - ESP8266

| Device       | Doors | RTC                | Keypad       | 433mhz Receiver  |
|--------------|-------|--------------------|--------------|------------------|
| Arduino Uno  |  4    | DS3231 External    | Pin 2,3,4,5  | n/a              |
| Arduino MEGA |  8    | DS3231 External    | Pin 2,3,4,5  | Pin 18 (Int 5)   |
| Numato       |  4    | Internal           | Pin 4,5,12,6 | n/a              |
| Wemos        |  x    | x                  | x            |                  |

# Door and Security MCU

v1.0 MCU will support any number of doors. Rather than just Locked, and Unlocked states for a given relay,  we will implement a few other options for flexibility. The schedule will include flags for each door relay state, each door sensor/monitor state, and site environment state. Flags such as “Inherited”, “No State Applied”, and “Final Rule” will facilitate the ability of making complex rule combinations.

Order of Operation will attempt to match in the following order:
1. Exact date (year, month, day)
2. Month
3. Day of Month
4. Day of Week
5. Non-specific date rules

More than one Exact date, Month, Day of Month, or Day of Week rule would be invalid. However, there may be multiple Non-specific date rules applied. These will be applied using a simple span of time in minutes as a metric. I.E. Any rule that covers a span of 8 hours will be applied before a rules that spans that of only 60 minutes. Making a simple closed during lunch hour rules a simple assumption to apply last in the order of operations, as it should be able to override the fact that the site is generally open.

A rule flag of F - [Final] will be rarely used, but will facilitate the ability to apply a global rule that forces no other rules to be applied. Allowing one to quickly and permanently apply a closed all day rule to a specific day.

### Door Relay State
- U - Unlocked
- L - Locked
- F - Force Lock (overcome manual override)
- I - Inherit State from other rules
- X - No Door State Applied by this Rule

### Door Sensor State
- U - Unarmed
- C - Chime
- A - Armed (Will not grant door access to locked door)
- I - Inherit State from Previous Rule
- X - No Sensor State Applied by this Rule

### Site Environment State
- I - Inherit State Applied by Previous Rule
- X - No Site State Applied by this Rule
- U - Unarmed Site, Guest Access Permitted
- A - Armed State, No Guest Presence Permitted (Motion Sensors ON)
- S - Stay Mode, No Perimeter Access Allowed
- F - Force Armed Mode - Overriding Manual State
- O - Force Disarmed State - Overriding Manual Arm State

### Rule Flag
- X - No Rule Flag
- F - Final Rule. Process no other rules

Closed all Day Rule should be anything with a time 00:00 - 23:59
That marks all doors as Locked or Armed, with a F rule flag (final)

Default Door Lock State: *Locked*

Default Door Sensor State: *Unarmed*

Default Site State: *Unarmed*

| YEAR | MONTH|DAY   |DOW   |Time       | Door1|Door2 | Door3| Door4| SITE | RULE |
|------|------|------|------|-----------|------|------|------|------|------|------|
| 2016 | 05   |  16  |      |           |      |      |      |      | U    | X    |
| 2016 | 05   |  17  |      |00:00-23:59|  LU  | LU   | LU   | LU   | U    | X    |
| 2016 | 05   |  18  |      |08:00-20:00|      |      |      |      | U    | X    |
| 2016 | 05   |  19  |      |08:00-20:00|      |      |      |      | U    | X    |
|      |      |      | 6    |           |  LU  | LU   | LU   | LA   | U    | F    |
|      |      |      | 5    |           |  LU  | LU   | LU   | LA   | U    | F    |
|      |      |      |      |08:00-20:00|  UU  | UU   | LU   | LA   | U    | X    |
|      |      |      |      |12:00-13:00|  LU  | LU   | LU   | LA   | U    | X    |



## in the works

thoughts on schedule entries
- incourage the use of (* * * * 00 00 23 59) default type rules for global env state
- use * instead of -1 or 255 to emulate wildcard/crontab syntax
- span of day contributes to metric weight
- year, month, day, dow, each contribute to metric weight
- replace RULE value with int8_t metric value, allowing user to manually apply additional metric weight

+SCH 2016 05 * * 01 00 05 00 LU LU LU LU O 10
+SCH * * * * 00 00 23 59 LA LA LA LA A 100

## User entries
| UID | keycode | cardcode | kc_req  | cc_req  | Door1 | Door2 | Door3 | Door4 | Site |
|-----|---------|----------|---------|---------|-------|-------|-------|-------|------|
| 10  | 6321055 |  16444   |   N     |   Y     |       |       |       |       |      |
| 11  | 6322055 |  17322   |   Y     |   Y     | G     | G     | A     | S     | S    |

### Door Level Access
- G - Guest
- S - Staff
- A - Admin
