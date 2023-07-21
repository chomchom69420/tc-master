#include "lanes.h"
#include "delay.h"
#include "Arduino.h"
#include "mqtt.h"
#include "environment.h"
#include "configurations.h"

enum States{
    IDLE,           //idle state - all lamps off

    SO_G1,          //straight only (SO) 1,3 --> green
    SO_AMB1,        //straight only (SO) 1,3 --> amber
    SO_REX1,        //straight only (SO) --> red extension 1 (for r_ext_t time)
    SO_G2,          //straight only (SO) 2,4 --> green
    SO_AMB2,        //straight only (SO) 2,4 --> amber
    SO_REXT2,       //straight only (SO) --> red extension 2 (for r_ext_t time)

    MD_G1,          //multidirection (MD) 1 --> green
    MD_AMB1,        //multidirection (MD) 1 --> amber 
    MA_REXT1,       //multidirection (MD) --> red extension 1
    MD_G2,          
    MD_AMB2,                 
    MA_REXT2,       
    MD_G3, 
    MD_AMB3,
    MA_REXT3,
    MD_G4, 
    MD_AMB4,
    MA_REXT4,
    
    PED,            //Pedestrian lights on (for p time)

    BL              //Blinker (BL) mode for Amber
};

int state;
int comm_done = 0; // flag to store if latest value has been communicated or not

void lanes_start_signals()
{
    // Start with making everything red
    state = 0;
    for(int i=1;i<=env_getNumSlaves();i++) 
        env_setSlaveState(i,0);

    //Communicate to slaves
    mqtt_publish_signal();

    //Start delay
    delay_set(0,env_getGlobalTimer(0));
}

void lanes_set_state(int state)
{
    // Set the state of the fsm
    state = state;
}

int lanes_get_state() {
    return state;
}

void lanes_updateCombined()
{

}

void lanes_update()
{
    int n = env_getNumSlaves();
    int mode = env_getMode();

    if (mode == MODE_MULTIDIRECTION)
    {
        switch (state)
        {
        case 0:
            //Lane lights off, pedestrian lights ON
            if(delay_is_done(0))
            {
                state=1;
                comm_done =0;
                delay_set(0,env_getGlobalTimer(1));
            }
            if(!comm_done)
            {
                //set all lanes to red
                for(int i=1;i<=n;i++) 
                    env_setSlaveState(i, 0);

                mqtt_publish_signal();
            }

        case 1:
            if (delay_is_done(0))
            {
                // if the delay is done move to the next lane
                state = 2;
                comm_done = 0;
                delay_set(0, env_getGlobalTimer(2));
            }

            if (!comm_done)
            {
                // Change slave state
                // slave_states.states[state - 1] = 1; // set the current lane to green
                env_setSlaveState(state, 1);

                // set other lanes to red
                for (int i = 1; i<=n; i++)
                {
                    if (i == state)
                        continue;
                    env_setSlaveState(i, 0);
                }

                // Send MQTT message
                mqtt_publish_signal();
                // Set comm_done
                comm_done = 1;
            }

            break;
        case 2:
            if (delay_is_done(0))
            {
                // move to next state
                state = 3;
                comm_done = 0;
                delay_set(0, env_getGlobalTimer(3));
            }

            if (!comm_done)
            {
                // Change slave state
                env_setSlaveState(state, 1);

                // set other lanes to red
                for (int i = 1; i<=n; i++)
                {
                    if (i == state)
                        continue;
                    env_setSlaveState(i, 0);
                }

                // Send MQTT message
                mqtt_publish_signal();
                // Set comm_done
                comm_done = 1;
            }
            break;

        case 3:
            if (delay_is_done(0))
            {
                // move to next state
                state = 4;
                comm_done = 0;
                delay_set(0, env_getGlobalTimer(4));
            }
            if (!comm_done)
            {
                // Change slave state
                env_setSlaveState(state, 1);

                // set other lanes to red
                for (int i = 1; i<=n; i++)
                {
                    if (i == state)
                        continue;
                    env_setSlaveState(i, 0);
                }

                // Send MQTT message
                mqtt_publish_signal();
                // Set comm_done
                comm_done = 1;
            }

        case 4:
            if (delay_is_done(0))
            {
                // move to next state
                state = 0;
                comm_done = 0;
                delay_set(0, env_getGlobalTimer(0));
            }
            if (!comm_done)
            {
                // Change slave state
                env_setSlaveState(state, 1);

                // set other lanes to red
                for (int i = 1; i<=n; i++)
                {
                    if (i == state)
                        continue;
                    env_setSlaveState(i, 0);
                }

                // Send MQTT message
                mqtt_publish_signal();
                // Set comm_done
                comm_done = 1;
            }

        default:
            break;
        }
    }

    else if(mode == MODE_STRAIGHT_ONLY)
    {

        //Straights are allowed only 
        //Two sets of slaves are turned GREEN at once
        switch (state)
        {

        case 0:
            //All are red, pedestrian lights are ON
            if(delay_is_done(0))
            {
                state=1;
                comm_done=0;
                delay_set(0, env_getGlobalTimer(1));
            }
            if(!comm_done)
            {
                // Change slave states
                //All slaves are turned red 
                for(int i=1;i<=n;i++) env_setSlaveState(i, 0);
            }
        case 1:
            //Slave 1, 3 are turned green
            if (delay_is_done(0))
            {
                // if the delay is done move to the next lane
                state = 2;
                comm_done = 0;
                delay_set(0, env_getGlobalTimer(2));
            }

            if (!comm_done)
            {
                // Change slave states
                //Slave 1 and 3 are turned GREEN, all others are RED
                env_setSlaveState(1, 1);
                env_setSlaveState(3, 1);
                env_setSlaveState(2, 0);
                env_setSlaveState(4, 0); 

                // Send MQTT message
                mqtt_publish_signal();
                // Set comm_done
                comm_done = 1;
            }

            break;
        case 2:
            //Slave 2, 4 are turned green
            if (delay_is_done(0))
            {
                // move to next state
                state = 0;
                comm_done = 0;
                delay_set(0, env_getGlobalTimer(0));
            }

            if (!comm_done)
            {
                // Change slave state
                //Slave 2 and 4 are turned GREEN, all others are RED
                env_setSlaveState(1, 0);
                env_setSlaveState(3, 0);
                env_setSlaveState(2, 1);
                env_setSlaveState(4, 1);


                // Send MQTT message
                mqtt_publish_signal();
                // Set comm_done
                comm_done = 1;
            }
            break;

        default:
            break;
        }
    }
}