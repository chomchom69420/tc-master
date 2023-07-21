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
  env_init();
  lanes_init(); 
}

void loop(){
  if(!mqtt_pubsubloop())  
    mqtt_reconnect(); 
  
  //Update lanes FSM only if control mode is auto
  if(getControlMode() == ControlMode::AUTO)
    lanes_update();

  slots_updateCurrent();
}
