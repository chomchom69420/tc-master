//Slaves states 
enum SlaveStates {
    RED, AMBER, GREEN, OFF
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
