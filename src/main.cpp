#include <Arduino.h>
#include "delay.h"
#include "mqtt.h"
#include "lanes.h"
#include "control.h"
#include "environment.h"
#include "slots.h"

void setup(){
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  delay_init();
  mqtt_setup();
  mqtt_reconnect();
  slots_initTime();
  slots_clear();

  //Setup the Slave states
  env_setNumSlaves(4);
  env_setMode(MODE_MULTIDIRECTION);

  int global_timers[5] = {20,10,15,5,3};

  env_setGlobalTimers(global_timers);
  env_calcSetSlaveTimers();

  //Start fsm
  lanes_start_signals();
}

void loop(){
  if(!mqtt_pubsubloop())  mqtt_reconnect(); 
  
  //Update lanes FSM only if control mode is auto
  if(getControlMode() == ControlMode::AUTO)
    lanes_update();

  slots_updateCurrent();
}
