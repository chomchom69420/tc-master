#include "lanes.h"
#include "delay.h"
#include "Arduino.h"
#include "mqtt.h"
#include "environment.h"
#include "configurations.h"

int signal_state;

int comm_done = 0; // flag to store if latest value has been communicated or not

void lanes_start_signals()
{
    // Start with making everything red
    signal_state = 0;
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
    signal_state = state;
}

int lanes_get_state() {
    return signal_state;
}

void lanes_update()
{
    int n = env_getNumSlaves();
    int mode_select = env_getSequenceMode();

    if (mode_select == MODE_MULTIDIRECTION)
    {
        switch (signal_state)
        {
        case 0:
            //Lane lights off, pedestrian lights ON
            if(delay_is_done(0))
            {
                signal_state=1;
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
                signal_state = 2;
                comm_done = 0;
                delay_set(0, env_getGlobalTimer(2));
            }

            if (!comm_done)
            {
                // Change slave state
                // slave_states.states[signal_state - 1] = 1; // set the current lane to green
                env_setSlaveState(signal_state, 1);

                // set other lanes to red
                for (int i = 1; i<=n; i++)
                {
                    if (i == signal_state)
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
                signal_state = 3;
                comm_done = 0;
                delay_set(0, env_getGlobalTimer(3));
            }

            if (!comm_done)
            {
                // Change slave state
                env_setSlaveState(signal_state, 1);

                // set other lanes to red
                for (int i = 1; i<=n; i++)
                {
                    if (i == signal_state)
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
                signal_state = 4;
                comm_done = 0;
                delay_set(0, env_getGlobalTimer(4));
            }
            if (!comm_done)
            {
                // Change slave state
                env_setSlaveState(signal_state, 1);

                // set other lanes to red
                for (int i = 1; i<=n; i++)
                {
                    if (i == signal_state)
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
                signal_state = 0;
                comm_done = 0;
                delay_set(0, env_getGlobalTimer(0));
            }
            if (!comm_done)
            {
                // Change slave state
                env_setSlaveState(signal_state, 1);

                // set other lanes to red
                for (int i = 1; i<=n; i++)
                {
                    if (i == signal_state)
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

    else if(mode_select == MODE_STRAIGHT_ONLY)
    {

        //Straights are allowed only 
        //Two sets of slaves are turned GREEN at once
        switch (signal_state)
        {

        case 0:
            //All are red, pedestrian lights are ON
            if(delay_is_done(0))
            {
                signal_state=1;
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
                signal_state = 2;
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
                signal_state = 0;
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