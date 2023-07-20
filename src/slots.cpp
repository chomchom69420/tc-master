#include "slots.h"
#include <vector>

float offset = 5.5 * 3600;
ESP32Time rtc((int)offset);

//add environment struct

struct slot
{
    struct tm start;
    struct tm end;

    //TODO: add environment struct instance 
};

typedef std::vector<struct slot> slot_array;

std::vector<slot_array> days(7);
struct slot current_slot;

static struct tm returnStruct(JsonObject& parsed)
{
    struct tm t = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    t.tm_year = parsed["year"];
    t.tm_year -= 1990;          // This is year-1900, so 123 = 2023
    
    t.tm_mon = parsed["month"];
    t.tm_mon -= 1;

    t.tm_wday = parsed["day"];
    t.tm_hour = parsed["hr"];
    t.tm_min = parsed["min"];
    t.tm_sec = parsed["sec"];
    return t;
}

void slots_setTime(JsonObject& parsed)
{
    rtc.setTimeStruct(returnStruct(parsed));
}

void slots_initTime()
{
    struct tm t = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    t.tm_year = 123;
    t.tm_mon = 0;
    t.tm_mday = 1;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;

    rtc.setTimeStruct(t);
}

void slots_resetTime()
{
    struct tm t = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    t.tm_year = 123;
    t.tm_mon = 0;
    t.tm_mday = 1;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;

    rtc.setTimeStruct(t);
}

void slots_set(JsonObject &parsed)
{
    for (int day = 0; day < 7;day++)
    {
        JsonObject& slots = parsed[(String)day];
        for(JsonPair slot : slots)
        {
            JsonObject& times = slot.value;
            JsonObject& start_json = times["start"];
            JsonObject& end_json = times["end"];
            struct tm start, end;
            start = returnStruct(start_json);
            end = returnStruct(end_json);

            days[day].push_back({start, end});
        }
    }
}

void slots_clear()
{
    for(int i=0;i<7;i++)
        days[i].clear();
}

void slots_updateCurrent()
{
    int day = rtc.getDayofWeek();
    int hr = rtc.getHour();
    if( strcmp(rtc.getAmPm(false).c_str(), "PM") ==0)
    {
        hr+=12;
    }
    int min = rtc.getMinute();

    for(auto& slot : days[day])
    {
        if(hr<slot.start.tm_hour)
            break;
        else if(hr>=slot.end.tm_hour)
            continue;
        else if(hr>slot.start.tm_hour && hr<slot.end.tm_hour)
        {
            //this is current
            current_slot = slot;
            break;  //no need to check more
        }
        else //only once case is left --> hr == slot.start.tm_hour
        {
            if(min < slot.start.tm_min)
                break;
            else if(min >= slot.end.tm_min)
                continue;
            else if(min >= slot.start.tm_min && min < slot.end.tm_min)
            {
                current_slot = slot;
                break; //no need to check more 
            }
        }
    }

    //TODO: Update the environment
}