//This header file is used to maintain the real-time clock of the rtc

#include "ESP32Time.h"
#include "ArduinoJson.h"

/*
Parses the jsonobject and stores the RTC time
Format:
{
    "year": ,       //(0-365) + 1990
    "month":    ,   //0-11
    "day":  ,       //days since Sunday (0-6) ==> Sunday is 0
    "hr":   ,       //0-23
    "min":  ,       //0-59
    "sec":          //0-59
}
*/
void slots_setTime(JsonObject& parsed);

/*
Initializes datetime to 1st Jan, 2023 00:00:00
*/
void slots_initTime();

/*
Resets the datetime to 1st Jan, 2023 00:00:00
*/
void slots_resetTime();

/*
Function to parse slots
*/
void slots_set(JsonObject& parsed);

/*
Clears all slots for all days
*/
void slots_clear();

/*
Updates the current slot by looking at the current time from the inbuilt RTC of the ESP32 
*/
void slots_updateCurrent();

//Now, add functionality for changing modes using the slots library, maybe add it in updateCurrent()