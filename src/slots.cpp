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
    
    //Environment variables
    int mode_select;
    int global_fsm_timers[8]; //max states = 7 (max slaves) + 1
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
            JsonObject& start_json = slot.value["start"];
            JsonObject& end_json = slot.value["end"];
            JsonObject& mode = slot.value["mode"];
            struct tm start, end;
            start = returnStruct(start_json);
            end = returnStruct(end_json);
            
            struct slot temp;
            temp.start = start;
            temp.end = end;
            temp.mode_select = mode["mode"];
            for(int i=0;i<env_getNumSlaves()+1;i++)
            {
                char timer_idx[10];
                sprintf(timer_idx, "t%d", i);
                temp.global_fsm_timers[i]=mode[timer_idx];
            }

            days[day].push_back(temp);
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

    //Now that we have the current slot, update the environment with the right parameters
    env_setSequenceMode(current_slot.mode_select);
    env_setGlobalTimers(current_slot.global_fsm_timers);
}