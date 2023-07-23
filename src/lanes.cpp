#include "lanes.h"
#include "delay.h"
#include "Arduino.h"
#include "mqtt.h"
#include "environment.h"
#include "configurations.h"
#include <vector>
#include "ArduinoJson.h"

enum States
{
    s_IDLE, // idle state - all lamps off

    SO_G1,    // straight only (SO) 1,3 --> green
    SO_AMB1,  // straight only (SO) 1,3 --> amber
    SO_REXT1, // straight only (SO) --> red extension 1 (for r_ext_t time)
    SO_G2,    // straight only (SO) 2,4 --> green
    SO_AMB2,  // straight only (SO) 2,4 --> amber
    SO_REXT2, // straight only (SO) --> red extension 2 (for r_ext_t time)

    MD_G1,    // multidirection (MD) 1 --> green
    MD_AMB1,  // multidirection (MD) 1 --> amber
    MD_REXT1, // multidirection (MD) --> red extension 1
    MD_G2,
    MD_AMB2,
    MD_REXT2,
    MD_G3,
    MD_AMB3,
    MD_REXT3,
    MD_G4,
    MD_AMB4,
    MD_REXT4,

    PED, // Pedestrian lights on (for p time)

    BL,         // Blinker (BL) mode for Amber
    BL_REXT     //Red extension for Blinker
} state;

typedef struct slave
{
    int slave_id;
    enum SlaveStates state;
    float r, g, amb;
};

std::vector<slave> slaves(7); // 7 slaves at max

// static int comm_done = 0; // flag to store if latest state has been communicated or not

static void lanes_setSlaveState()
{
    // Set state of the slave -- currently doing for 4 slaves
    int n = env_getNumSlaves();
    switch (state)
    {
    case States::s_IDLE:
        for (int i = 0; i < n; i++)
            slaves[i].state = SlaveStates::IDLE;
        break;

    case States::SO_G1:
        for (int i = 0; i < n; i++)
        {
            if (i == 0 || i == 0 + n / 2)
                slaves[i].state = SlaveStates::GREEN;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::SO_AMB1:
        for (int i = 0; i < n; i++)
        {
            if (i == 0 || i == 0 + n / 2)
                slaves[i].state = SlaveStates::AMBER;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::SO_REXT1:
        for (int i = 0; i < n; i++)
            slaves[i].state = SlaveStates::RED;
        break;

    case States::SO_G2:
        for (int i = 0; i < n; i++)
        {
            if (i == 1 || i == 1 + n / 2)
                slaves[i].state = SlaveStates::GREEN;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::SO_AMB2:
        for (int i = 0; i < n; i++)
        {
            if (i == 1 || i == 1 + n / 2)
                slaves[i].state = SlaveStates::AMBER;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::SO_REXT2:
        for (int i = 0; i < n; i++)
            slaves[i].state = SlaveStates::RED;
        break;

    case States::PED:
        for (int i = 0; i < n; i++)
            slaves[i].state = SlaveStates::RED;
        break;

    case States::MD_G1:
        for (int i = 0; i < n; i++)
        {
            if (i == 0)
                slaves[i].state = SlaveStates::GREEN;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_AMB1:
        for (int i = 0; i < n; i++)
        {
            if (i == 0)
                slaves[i].state = SlaveStates::AMBER;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_REXT1:
        for (int i = 0; i < n; i++)
            slaves[i].state = SlaveStates::RED;
        break;

    case States::MD_G2:
        for (int i = 0; i < n; i++)
        {
            if (i == 1)
                slaves[i].state = SlaveStates::GREEN;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_AMB2:
        for (int i = 0; i < n; i++)
        {
            if (i == 1)
                slaves[i].state = SlaveStates::AMBER;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_REXT2:
        for (int i = 0; i < n; i++)
            slaves[i].state = SlaveStates::RED;
        break;

    case States::MD_G3:
        for (int i = 0; i < n; i++)
        {
            if (i == 2)
                slaves[i].state = SlaveStates::GREEN;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_AMB3:
        for (int i = 0; i < n; i++)
        {
            if (i == 2)
                slaves[i].state = SlaveStates::AMBER;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_REXT3:
        for (int i = 0; i < n; i++)
            slaves[i].state = SlaveStates::RED;
        break;

    case States::MD_G4:
        for (int i = 0; i < n; i++)
        {
            if (i == 3)
                slaves[i].state = SlaveStates::GREEN;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_AMB4:
        for (int i = 0; i < n; i++)
        {
            if (i == 3)
                slaves[i].state = SlaveStates::AMBER;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_REXT4:
        for (int i = 0; i < n; i++)
            slaves[i].state = SlaveStates::RED;
        break;

    default:
        break;
    }
}

void lanes_init()
{
    state = States::s_IDLE;
    lanes_setSlaveState();
    lanes_publishSignal();
}

void lanes_publishSignal()
{
    char payload[1000];
    int n = env_getNumSlaves();
    int mode = env_getMode();

    const int capacity = JSON_OBJECT_SIZE(50);
    StaticJsonBuffer<capacity> jb;
    JsonObject &obj = jb.createObject();

    obj["n"] = n;
    obj["mode"]=mode;
    JsonObject &json_slaves = obj.createNestedObject("slaves");

    for (int i = 0; i < n; i++)
    {
        char s[5];
        sprintf(s, "%d", i + 1);
        JsonObject &json_slave = json_slaves.createNestedObject(s);
        json_slave["en"] = env_getSlaveEnableStatus(i+1);           //slave id is i+1
        json_slave["state"] = slaves[i].state;
        json_slave["red"] = slaves[i].r;
        json_slave["amb"] = slaves[i].amb;
        json_slave["green"] = slaves[i].g;
    }

    obj.printTo(payload);
    mqtt_publish("/traffic/signals", payload);
}

void lanes_setSlaveTimers()
{
    // Set timers
    int n = env_getNumSlaves();
    int mode = env_getMode();
    for (int i = 0; i < n; i++)
    {
        if (mode == MODE_BL)
        {
            float f = *(env_getParams(mode));
            slaves[i].r = slaves[i].g = (1 / f) / 2; // T/2 is the green and red time for blink mode
            slaves[i].amb = 0;
        }

        else if (mode == MODE_MD)
        {
            bool p_en = env_getPedEnable();
            bool r_en = env_getRedExtEnable();
            int p_t = env_getPedTimer();
            int r_ext_t = env_getRedExtTimer();

            int *arr;
            arr = env_getParams(mode);
            int g[4], amb[4];
            for (int j = 0; j < 4; j++)
            {
                g[j] = arr[j];
                amb[j] = arr[j + 4];
            }

            slaves[i].g = g[i];
            slaves[i].amb = amb[i];
            slaves[i].r = p_t * p_en + n * r_ext_t * r_en;
            for (int j = 0; j < n; j++)
            {
                if (i == j)
                    continue;
                slaves[i].r += g[i] + amb[i];
            }
        }

        else if (mode == MODE_SO)
        {
            bool p_en = env_getPedEnable();
            bool r_en = env_getRedExtEnable();
            int p_t = env_getPedTimer();
            int r_ext_t = env_getRedExtTimer();

            int *arr;
            arr = env_getParams(mode);
            int g[4], amb[4];
            for (int j = 0; j < 2; i++)
            {
                g[j] = arr[j];
                amb[j] = arr[j + 2];
            }

            slaves[i].g = g[i];
            slaves[i].amb = amb[i];
            slaves[i].r = p_t * p_en + (n / 2) * r_ext_t * r_en;
            for (int j = 0; j < n / 2; j++)
            {
                if (i == j || i - (n / 2) == j)
                    continue;
                slaves[i].r += g[i] + amb[i];
            }
        }
    }
}

void lanes_start_signals()
{
    int mode = env_getMode();
    if (mode == MODE_MD)
        state = States::MD_G1;
    else if (mode == MODE_SO)
        state = States::SO_G1;
    else if (mode == MODE_BL)
        state = States::BL;
    else
        state = States::s_IDLE;

    lanes_setSlaveState();
    lanes_publishSignal();
}

static void lanes_moveToState(enum States s)
{
    // Set the state
    state = s;
    lanes_setSlaveState();

    // Publish on MQTT
    lanes_publishSignal();

    int n = env_getNumSlaves();
    int mode = env_getMode();

    // Start timers
    /*
    There are two steps involved in starting timers
    1. Get the timer paramters from the environment api and store them aptly
    2. switch over state and set the required timer by calling the delay api
    */

    // Getting the timer data
    int p_t = env_getPedTimer();
    int r_ext_t = env_getRedExtTimer();

    int *arr;
    arr = env_getParams(mode);
    int g[4], amb[4];
    int t_blink;
    if (mode == MODE_MD)
    {
        for (int j = 0; j < 4; j++)
        {
            g[j] = arr[j];
            amb[j] = arr[j + 4];
        }
    }
    else if (mode == MODE_SO)
    {
        for (int j = 0; j < 2; j++)
        {
            g[j] = arr[j];
            amb[j] = arr[j + 2];
        }
    }
    else if (mode == MODE_BL)
    {
        t_blink = 1 / (*arr);
    }

    switch (state)
    {
    case States::s_IDLE:
        break;

    case States::SO_G1:
        delay_set(0, g[0]);
        break;

    case States::SO_AMB1:
        delay_set(0, amb[0]);
        break;

    case States::SO_REXT1:
        delay_set(0, r_ext_t);
        break;

    case States::SO_G2:
        delay_set(0, g[1]);
        break;

    case States::SO_AMB2:
        delay_set(0, amb[1]);
        break;

    case States::SO_REXT2:
        delay_set(0, r_ext_t);
        break;

    case States::PED:
        delay_set(0, p_t);
        break;

    case States::MD_G1:
        delay_set(0, g[0]);
        break;

    case States::MD_AMB1:
        delay_set(0, amb[0]);
        break;

    case States::MD_REXT1:
        delay_set(0, r_ext_t);
        break;

    case States::MD_G2:
        delay_set(0, g[1]);
        break;

    case States::MD_AMB2:
        delay_set(0, amb[1]);
        break;

    case States::MD_REXT2:
        delay_set(0, r_ext_t);
        break;

    case States::MD_G3:
        delay_set(0, g[2]);
        break;

    case States::MD_AMB3:
        delay_set(0, amb[2]);
        break;

    case States::MD_REXT3:
        delay_set(0, r_ext_t);
        break;

    case States::MD_G4:
        delay_set(0, g[3]);
        break;

    case States::MD_AMB4:
        delay_set(0, amb[3]);
        break;

    case States::MD_REXT4:
        delay_set(0, r_ext_t);
        break;

    case States::BL:
        delay_set(0, t_blink);
        break;

    case States::BL_REXT:
        delay_set(0, r_ext_t);
        break;
    
    default:
        break;
    }
}

int lanes_getState()
{
    return state;
}

void lanes_update()
{
    int n = env_getNumSlaves();
    int mode = env_getMode();
    int p_en = env_getPedEnable();
    int r_ext_en = env_getRedExtEnable();
    int slave_en[4] = {1,1,1,1};

    switch (state)
    {
    case States::s_IDLE:
        // Transition
        if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);

        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);

        else if (mode == MODE_MD)
            lanes_moveToState(States::MD_G1);
        break;

    /*SO FSM part*/

    case States::SO_G1:
        if (delay_is_done(0) && mode == MODE_SO)
            lanes_moveToState(States::SO_AMB1);
        else if (mode == MODE_MD)
            lanes_moveToState(States::MD_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);
        break;

    case States::SO_AMB1:
        if (delay_is_done(0) && mode == MODE_SO && r_ext_en)
            lanes_moveToState(States::SO_REXT1);
        else if (delay_is_done(0) && mode == MODE_SO && !r_ext_en)
            lanes_moveToState(States::SO_G2);
        else if (mode == MODE_MD)
            lanes_moveToState(States::MD_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);
        break;

    case States::SO_REXT1:
        if (delay_is_done(0))
            lanes_moveToState(States::SO_G2);
        break;

    case States::SO_G2:
        if (delay_is_done(0) && mode == MODE_SO)
            lanes_moveToState(States::SO_AMB2);
        else if (mode == MODE_MD)
            lanes_moveToState(States::MD_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);
        break;

    case States::SO_AMB2:
        if (delay_is_done(0) && mode == MODE_SO && r_ext_en)
            lanes_moveToState(States::SO_REXT2);
        else if (delay_is_done(0) && mode == MODE_SO && !r_ext_en && p_en)
            lanes_moveToState(States::PED);
        else if (delay_is_done(0) && mode == MODE_SO && !r_ext_en && !p_en)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_MD)
            lanes_moveToState(States::MD_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);
        break;

    case States::SO_REXT2:
        if (delay_is_done(0) && p_en)
            lanes_moveToState(States::PED);
        else if (delay_is_done(0) && !p_en)
            lanes_moveToState(States::SO_G1);
        break;

    /*SO FSM part end*/

    /*MD FSM part*/

    case States::MD_G1:
        //Check if skipped 
        if(!slave_en[0])
            lanes_moveToState(States::MD_G2);
        
        if (delay_is_done(0) && mode == MODE_MD)
            lanes_moveToState(States::MD_AMB1);
        else if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);
        break;

    case States::MD_AMB1:
        if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);

        if(delay_is_done(0))
        {
            if(r_ext_en)
                lanes_moveToState(States::MD_REXT1);
            else
                lanes_moveToState(States::MD_G2);
        }
        
        break;

    case States::MD_REXT1:
        if (delay_is_done(0))
            lanes_moveToState(States::MD_G2);
        break;

    case States::MD_G2:
        if (delay_is_done(0) && mode == MODE_MD)
            lanes_moveToState(States::MD_AMB2);
        else if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);
        break;

    case States::MD_AMB2:
        if (delay_is_done(0) && mode == MODE_MD && r_ext_en)
            lanes_moveToState(States::MD_REXT2);
        else if (delay_is_done(0) && mode == MODE_MD && !r_ext_en)
            lanes_moveToState(States::MD_G3);
        else if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);
        break;

    case States::MD_REXT2:
        if (delay_is_done(0))
            lanes_moveToState(States::MD_G3);
        break;

    case States::MD_G3:
        if (delay_is_done(0) && mode == MODE_MD)
            lanes_moveToState(States::MD_AMB4);
        else if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);
        break;

    case States::MD_AMB3:
        if (delay_is_done(0) && mode == MODE_MD && r_ext_en)
            lanes_moveToState(States::MD_REXT3);
        else if (delay_is_done(0) && mode == MODE_MD && !r_ext_en)
            lanes_moveToState(States::MD_G4);
        else if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);
        break;

    case States::MD_REXT3:
        if (delay_is_done(0))
            lanes_moveToState(States::MD_G4);
        break;

    case States::MD_G4:
        if (delay_is_done(0) && mode == MODE_MD)
            lanes_moveToState(States::MD_AMB4);
        else if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);
        break;

    case States::MD_AMB4:
        if (delay_is_done(0) && mode == MODE_MD && r_ext_en)
            lanes_moveToState(States::MD_REXT4);
        else if (delay_is_done(0) && mode == MODE_MD && !r_ext_en && p_en)
            lanes_moveToState(States::PED);
        else if (delay_is_done(0) && mode == MODE_MD && !r_ext_en && !p_en)
            lanes_moveToState(States::MD_G1);
        else if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);
        break;

    case States::MD_REXT4:
        if (delay_is_done(0) && p_en)
            lanes_moveToState(States::PED);
        else if (delay_is_done(0) && !p_en)
            lanes_moveToState(States::MD_G1);
        break;

    /*MD fsm part end*/

    case States::PED:
        if (delay_is_done(0))
        {
            if (mode == MODE_MD)
                lanes_moveToState(States::s_IDLE);
            else
                lanes_moveToState(States::s_IDLE);
        }
        break;

    case States::BL:
        //Stay here as long as mode==BL
        if(mode!=MODE_BL)
            lanes_moveToState(States::BL_REXT);
        break;

    case States::BL_REXT:
        if(!delay_is_done(0))
            break;
        else if(delay_is_done(0) && mode == MODE_BL)
            lanes_moveToState(States::BL);
        else if(delay_is_done(0) && mode == MODE_MD)
            lanes_moveToState(States::MD_G1);
        else if(delay_is_done(0) && mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else
            break;
        break;
        
    default:
        break;
    }
}
