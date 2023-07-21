#include <Arduino.h>
#include "delay.h"
#include "mqtt.h"
#include "lanes.h"
#include "control.h"
#include "environment.h"

void setup(){
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  delay_init();
  mqtt_setup();
  reconnect();

  //Setup the Slave states
  set_number_of_slaves(4);
  env_setSequenceMode(MODE_MULTIDIRECTION);

  int global_timers[5] = {20,10,15,5,3};

  env_setGlobalTimers(global_timers, 4);
  env_calcSetSlaveTimers();

  //Start fsm
  lanes_start_signals();
}

void loop(){
  if(!pubsubloop())  reconnect(); 

  //Update lanes FSM only if control mode is auto
  if(getControlMode() == ControlMode::AUTO)
    lanes_update();
}
