#include "slots.h"
#include "environment.h"
#include <vector>

float offset = 5.5 * 3600;
ESP32Time rtc((int)offset);

//add environment struct

struct slot
{
    struct tm start;
    struct tm end;
    
    //Environment variable
    Environment env;
};

typedef std::vector<struct slot> slots;

std::vector<slots> days(7);
struct slot current_slot;

struct tm slots_returnStruct(JsonObject& parsed)
{
    struct tm t = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    t.tm_year = 123;
    t.tm_mon = 0;
    t.tm_wday = parsed["wday"];
    t.tm_hour = parsed["hr"];
    t.tm_min = parsed["min"];
    t.tm_sec = parsed["sec"];
    return t;
}

static void initTimeStruct(struct tm &t)
{
    t.tm_year = 123;
    t.tm_mon = 0;
    t.tm_wday = 0; //init to Sunday (0-6)
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
}

void slots_init()
{
    //Clear the slots
    slots_clearDays();
}

void slots_setTime(JsonObject& parsed)
{
    rtc.setTimeStruct(slots_returnStruct(parsed));
}

void slots_initTime()
{
    struct tm t = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    t.tm_year = 123;
    t.tm_mon = 0;
    t.tm_wday = 0;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;

    rtc.setTimeStruct(t);
}

void slots_resetTime()
{
    struct tm t = {123, 0, 0, 0, 0, 0, 0, 0, 0};
    t.tm_year = 123;
    t.tm_mon = 0;
    t.tm_wday = 0;
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
            JsonObject& start_json = slot.value["start"];
            JsonObject& end_json = slot.value["end"];
            JsonObject& env_json = slot.value["env"];
            struct tm start, end;
            start = slots_returnStruct(start_json);
            end = slots_returnStruct(end_json);
            
            struct slot temp;
            temp.start = start;
            temp.end = end;
            temp.env = env_returnStruct(env_json);

            days[day].push_back(temp);
        }
    }
}

void slots_clearDays()
{
    for(int i=0;i<7;i++)
        days[i].clear();
}

void slots_updateCurrent()
{
    int day = rtc.getDayofWeek();
    int hr = rtc.getHour();
    int min = rtc.getMinute();
    if( strcmp(rtc.getAmPm(false).c_str(), "PM") ==0)
        hr+=12;

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

    //Now that we have the current slot, update the environment with the right parameters
    env_set(current_slot.env);
}