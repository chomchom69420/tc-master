#include <Arduino.h>
#include "delay.h"
#include "mqtt.h"
#include "lanes.h"
#include "control.h"
#include "environment.h"
#include "slots.h"

void setup()
{
  Serial.begin(9600);
  delay_init();
  mqtt_setup();
  mqtt_reconnect();
  slots_init();
  slots_initTime();
  env_init();
  lanes_init();

  mqtt_publish("test/message", "Hello world from ESP32!");
}

void loop()
{
  if (!mqtt_pubsubloop())
    mqtt_reconnect();

  lanes_update();

  slots_updateCurrent();
}
