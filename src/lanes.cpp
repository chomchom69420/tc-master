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

    BL,     // Blinker (BL) mode for Amber
    BL_REXT // Red extension for Blinker
} state;

typedef struct slave
{
    int slave_id;
    bool skipped;
    enum SlaveStates state;
    int r, g, amb;
    int blink_f;
};

std::vector<slave> slaves(7); // 7 slaves at max

static bool comm_done = 0; // flag to store if latest state has been communicated or not

static void lanes_setSlaveState()
{
    // Set state of the slave -- currently doing for 4 slaves
    int n = env_getNumSlaves();
    switch (state)
    {
    case States::s_IDLE:
        for (int i = 0; i < n; i++)
        {
            slaves[i].state = SlaveStates::IDLE;
        }
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
            if (i == 0 && !slaves[0].skipped)
                slaves[i].state = SlaveStates::GREEN;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_AMB1:
        for (int i = 0; i < n; i++)
        {
            if (slaves[i].skipped)
            {
                slaves[i].state = SlaveStates::RED;
                continue;
            }
            if (i == 0 && !slaves[0].skipped)
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
            if (i == 1 && !slaves[1].skipped)
                slaves[i].state = SlaveStates::GREEN;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_AMB2:
        for (int i = 0; i < n; i++)
        {
            if (i == 1 && !slaves[1].skipped)
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
            if (i == 2 && !slaves[2].skipped) 
                slaves[i].state = SlaveStates::GREEN;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_AMB3:
        for (int i = 0; i < n; i++)
        {
            if (i == 2 && !slaves[2].skipped)
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
            if (i == 3 && !slaves[3].skipped)
                slaves[i].state = SlaveStates::GREEN;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_AMB4:
        for (int i = 0; i < n; i++)
        {
            if (i == 3 && !slaves[3].skipped)
                slaves[i].state = SlaveStates::AMBER;
            else
                slaves[i].state = SlaveStates::RED;
        }
        break;

    case States::MD_REXT4:
        for (int i = 0; i < n; i++)
            slaves[i].state = SlaveStates::RED;
        break;

    case States::BL:
        for (int i = 0; i < n; i++)
        {
            if(slaves[i].skipped)
            {
                slaves[i].state = SlaveStates::RED;
            }
            slaves[i].state = SlaveStates::BLINKER;
        }
        break;

    case States::BL_REXT:
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
    obj["mode"] = mode;
    JsonObject &json_slaves = obj.createNestedObject("slaves");

    for (int i = 0; i < n; i++)
    {
        char s[5];
        sprintf(s, "%d", i + 1);
        JsonObject &json_slave = json_slaves.createNestedObject(s);
        json_slave["en"] = env_getSlaveEnableStatus(i + 1); // slave id is i+1
        json_slave["state"] = slaves[i].state;
        json_slave["red"] = slaves[i].r;
        json_slave["amb"] = slaves[i].amb;
        json_slave["green"] = slaves[i].g;
        if (slaves[i].state == SlaveStates::BLINKER)
        {
            // give them a blink frequency
            json_slave["blink_f"] = slaves[i].blink_f;
        }
    }

    obj.printTo(payload);
    mqtt_publish("/traffic/signals", payload);
}

void lanes_setSlaveParams()
{
    int n = env_getNumSlaves();
    int mode = env_getMode();
    bool p_en = env_getPedEnable();
    bool r_en = env_getRedExtEnable();
    int p_t = env_getPedTimer();
    int r_ext_t = env_getRedExtTimer();
    int *arr;
    arr = env_getParams(mode);

    int g[4], amb[4];

    // These need to be stored before computation
    for (int i = 0; i < n; i++)
    {
        slaves[i].slave_id = i + 1;
        slaves[i].skipped = !env_getSlaveEnableStatus(slaves[i].slave_id); //! because skipped is true when slave enable is false
    }

    for (int i = 0; i < n; i++)
    {
        if (mode == MODE_BL)
        {
            slaves[i].blink_f = *(env_getParams(mode));
        }

        else if (mode == MODE_MD)
        {
            for (int j = 0; j < 4; j++)
            {
                g[j] = arr[j];
                amb[j] = arr[j + 4];
            }

            slaves[i].g = g[i];
            slaves[i].amb = amb[i];

            /*
            Calculate number of red extensions to be executed with slave skipping
            No. of red ext = n - total slaves skipped
            */
            int n_r_ext = n;
            for (int j = 0; i < n; i++)
            {
                n += slaves[j].skipped ? -1 : 0;   
            }

            slaves[i].r = p_t * p_en + n_r_ext * r_ext_t * r_en;
            
            for (int j = 0; j < n; j++)
            {
                if (i == j || 
                    slaves[j].skipped)          //Redundant because: if j is skipped, then g[j] = amb[j] = 0 already
                    continue;
                slaves[i].r += g[j] + amb[j];
            }
        }

        else if (mode == MODE_SO)
        {
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
                slaves[i].r += g[j] + amb[j];
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
}

static void lanes_performState()
{
    // Set the states of each slave
    lanes_setSlaveState();

    // Publish the command with slave states, parameters on MQTT topic /traffic/signals
    // comm_done is used as we only want to transmit once for every state
    if (!comm_done)
    {
        lanes_publishSignal();
        comm_done = 1;
    }
}

static void lanes_moveToState(enum States s)
{
    // Set the state
    state = s;

    // Reset comm_done
    comm_done = false;

    /* Timers should be started during transition */

    int n = env_getNumSlaves();
    int mode = env_getMode();

    // Start timers
    /*
    There are two steps involved in starting timers
    1. Get the timer paramters from the environment api and store them
    2. Switch state and set the required timer by calling the delay api
    */

    // Getting the timer data
    int p_t = env_getPedTimer();
    int r_ext_t = env_getRedExtTimer();

    int *arr;
    arr = env_getParams(mode);
    int g[4], amb[4];
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
    // Don't need to set any timer for MODE_BL as it is infinite

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
        // delay_set(0, t_blink);   //No need to set any timer
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
    int slave_en[7];

    for(int i=0;i<n;i++)
        slave_en[i]= env_getSlaveEnableStatus(i+1);

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

        // If no transition, now execute the state
        lanes_performState();
        break;

        /*SO FSM part*/

    case States::SO_G1:
        if (delay_is_done(0) && mode == MODE_SO)
            lanes_moveToState(States::SO_AMB1);
        else if (mode == MODE_MD)
            lanes_moveToState(States::MD_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);

        lanes_performState();
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
        lanes_performState();
        break;

    case States::SO_REXT1:
        if (delay_is_done(0))
            lanes_moveToState(States::SO_G2);

        lanes_performState();
        break;

    case States::SO_G2:
        if (delay_is_done(0) && mode == MODE_SO)
            lanes_moveToState(States::SO_AMB2);
        else if (mode == MODE_MD)
            lanes_moveToState(States::MD_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);

        lanes_performState();
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

        lanes_performState();
        break;

    case States::SO_REXT2:
        if (delay_is_done(0) && p_en)
            lanes_moveToState(States::PED);
        else if (delay_is_done(0) && !p_en)
            lanes_moveToState(States::SO_G1);

        lanes_performState();
        break;

        /*SO FSM part end*/

        /*MD FSM part*/

    case States::MD_G1:
        // Check if skipped
        if (!slave_en[0])
            lanes_moveToState(States::MD_G2);

        if (delay_is_done(0) && mode == MODE_MD)
            lanes_moveToState(States::MD_AMB1);
        else if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);

        lanes_performState();
        break;

    case States::MD_AMB1:
        if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);

        if (delay_is_done(0))
        {
            if (r_ext_en)
                lanes_moveToState(States::MD_REXT1);
            else
                lanes_moveToState(States::MD_G2);
        }

        lanes_performState();
        break;

    case States::MD_REXT1:
        if (delay_is_done(0))
            lanes_moveToState(States::MD_G2);

        lanes_performState();
        break;

    case States::MD_G2:
        if (!slave_en[1])
            lanes_moveToState(States::MD_G3);

        if (delay_is_done(0) && mode == MODE_MD)
            lanes_moveToState(States::MD_AMB2);
        else if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);

        lanes_performState();
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

        lanes_performState();
        break;

    case States::MD_REXT2:
        if (delay_is_done(0))
            lanes_moveToState(States::MD_G3);

        lanes_performState();
        break;

    case States::MD_G3:
        if (!slave_en[2])
            lanes_moveToState(States::MD_G4);

        if (delay_is_done(0) && mode == MODE_MD)
            lanes_moveToState(States::MD_AMB4);
        else if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);

        lanes_performState();
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

        lanes_performState();
        break;

    case States::MD_REXT3:
        if (delay_is_done(0))
            lanes_moveToState(States::MD_G4);

        lanes_performState();
        break;

    case States::MD_G4:
        if (!slave_en[0])
        {
            if (p_en)
                lanes_moveToState(States::PED);
            else
                lanes_moveToState(States::MD_G3);
        }

        if (mode == MODE_SO)
            lanes_moveToState(States::SO_G1);
        else if (mode == MODE_BL)
            lanes_moveToState(States::BL);

        if (delay_is_done(0) && mode == MODE_MD)
            lanes_moveToState(States::MD_AMB4);

        lanes_performState();
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

        lanes_performState();
        break;

    case States::MD_REXT4:
        if (delay_is_done(0) && p_en)
            lanes_moveToState(States::PED);
        else if (delay_is_done(0) && !p_en)
            lanes_moveToState(States::MD_G1);

        lanes_performState();
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

        lanes_performState();
        break;

    case States::BL:
        // Stay here as long as mode==BL
        if (mode != MODE_BL)
            lanes_moveToState(States::BL_REXT);

        lanes_performState();
        break;

    case States::BL_REXT:
        if (!delay_is_done(0))
            break;
        else if (delay_is_done(0) && mode == MODE_BL)
            lanes_moveToState(States::BL);
        else if (delay_is_done(0) && mode == MODE_MD)
            lanes_moveToState(States::MD_G1);
        else if (delay_is_done(0) && mode == MODE_SO)
            lanes_moveToState(States::SO_G1);

        lanes_performState();
        break;

    default:
        break;
    }
}
