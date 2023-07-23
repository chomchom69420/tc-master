/*
Publishes the signal over MQTT in a JSON format:
{
    "n":    ,
    "mode": ,
    "slaves": {
        "1" : {
            "en": 1     ,
            "state":    ,
            "red":      ,
            "amber":    ,
            "green":    
        },
        "2" : {
            "en": 0     ,
            "state":    ,
            "red":      ,
            "amber":    ,
            "green":    
        },
        ...
    }
}
*/
void lanes_publishSignal();

/*
Initializes the slaves to IDLE and publishes
*/
void lanes_init();

/*
Calculates and updates the slave timers and publishes
Environment variables (global parameters) need to be set before calling this function
*/
void lanes_setSlaveTimers();

/*
Updates the lanes fsm
*/
void lanes_update();

/*
Returns the state of the lanes fsm
States:
    IDLE, // idle state - all lamps off

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

    BL // Blinker (BL) mode for Amber
    BL_REXT
*/
int lanes_getState();

