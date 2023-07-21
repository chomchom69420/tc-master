#include "ArduinoJson.h"

typedef struct {
    int n_slaves;
    int mode;

    bool p_en;                   //Pedestrian enable 
    bool r_ext_en;               //Red extension enable

    int p_t;                    //Pedestrian timer
    int r_ext_t;                //Red extension timer

    int so_g[2];                //green timers for straight only (g1, g2)
    int so_amb[2];              //amber timers for straight only (a1, a2)

    int md_g[4];                //green timers for multidirection (g1, g2, g3, g4)
    int md_amb[4];              //amber timers for multidirection (amb1, amb2, amb3, amb4)

    int bl_freq;                //blink frequency for blink mode  
} Environment;

void env_set(JsonObject& parsed);

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
void env_setMode(int mode);

/*
Return the number of slaves in the environment
*/
int env_getNumSlaves();

/*
Return the sequence mode of the environment
*/
int env_getMode();

/*
Set the pedestrian enable flag
*/
void env_setPedEnable(bool p);

/*
Set the red extension enable flag
*/
void  env_setRedExtEnable(bool r);

/*
Get the pedestrian enable flag
*/
int env_getPedEnable();

/*
Get the red extension enable flag
*/
int  env_getRedExtEnable();

/*
Used to set the parameters for a specific mode
Mention the mode and send in a pointer to an array containing all the parameters in sequence
Modes:
MODE_SO
MODE_MD
MODE_BL
*/
void env_setParams(int mode, int* param_array);

/*
Returns a pointer to an array containing params for a specific mode
Modes:
MODE_SO 
MODE_MD
MODE_BL
*/
int* env_getParams(int mode);

void env_setRedExtTimer(int t);

void env_setPedTimer(int t);

int env_getRedExtTimer();

int env_getPedTimer();

