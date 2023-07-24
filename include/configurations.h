//Slaves states 
enum SlaveStates {
    IDLE=0, 
    RED=4,       //These values need to match the DICTATED_<colour> values in the SignalState enum in tc-slave/src/signals.h
    AMBER=5, 
    GREEN=6,
    BLINKER=7
};

// Mode select
#define MODE_STRAIGHT_ONLY 1
#define MODE_MULTIDIRECTION 2
#define MODE_BLINKER 3

#define MODE_SO MODE_STRAIGHT_ONLY
#define MODE_MD MODE_MULTIDIRECTION
#define MODE_BL MODE_BLINKER

//Local Control Panel (LCP) ID
#define PANEL_ID 20
