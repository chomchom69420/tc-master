typedef struct {
    int n_slaves;
    int states[7];
    int mode_select;
    int global_fsm_timers[8]; //max is n_slaves + 1
    int slaves_green_timers[7];
    int slaves_red_timers[7];
} Environment;

/*
Set the state of the slave corresponding to the slave_id 
slave_id => (1-7)
state => SlaveStates::RED, SlaveStates::GREEN, SlaveStates::AMBER, SlaveStates::OFF
*/
void env_setSlaveState(int slave_id, int state);

/*
Set the number of slaves. min = 1, max = 7
This function needs to be called before calling 
set_global_timers(), set_slaves_green_timers(), set_slaves_red_timers()
*/
void env_setNumSlaves(int n);

/*
Set the sequence mode. Sequence modes are stored in enum SeqModes in configurations.h
Modes:
SeqModes::MODE_STRAIGHT_ONLY 
SeqModes::MODE_MULTIDIRECTION
*/
void env_setSequenceMode(int mode);

/*
Set global timers by passing a pointer to an array containing the timer values in seconds
This function can be called only after the number
of states has been updated by calling the set_number_of_slaves() function
*/
void env_setGlobalTimers(int *global_timers_array);

/*
Set the green_timers of the slaves by passing a pointer to an array containing the slave timer values in seconds
This function is called only if we already have the slave timings and want to just store them
In this case the calc_set_slave_timers() function need not be called 
*/
void env_setSlavesGreenTimers(int *slave_timers_array);

/*
Set the red timers of the slaves by passing a pointer to an array containg the slave timer values in seconds
This function is called only if we already have the slave timings and want to just store them
In this case the calc_set_slave_timers() function need not be called 
*/
void env_setSlavesRedTimers(int *slave_timers_array);

/*
Calculate the slave timings and set them, no argument needed
Called only if set_slave_red_timers() and set_slaves_green_timers() are not called 
*/
void env_calcSetSlaveTimers();

/*
Returns the state of the slave referred to by passing the slave id
*/
int env_getSlaveState(int slave_id);

/*
Returns the global timer value of the state referred to by passing the slave id
*/
int env_getGlobalTimer(int state_id);

/*
Returns the timer associated with the slave id and state
*/
int env_getSlaveTimer(int slave_id, int slave_state);

/*
Return the number of slaves in the environment
*/
int env_getNumSlaves();

/*
Return the sequence mode of the environment
*/
int env_getSequenceMode();

