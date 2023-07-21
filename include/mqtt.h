#include <PubSubClient.h>

#define SLAVE_UPDATES_TOPIC "/traffic/slave_feedback"
#define MASTER_UPDATES_TOPIC "/traffic/updates"
#define SIGNAL_PUBLISH_TOPIC "/traffic/signals"

//used to setup MQTT server and callback function
void mqtt_setup();

//for connecting to MQTT broker
void mqtt_reconnect();

void mqtt_publish(char* topic, char* payload);

void mqtt_publish_signal();

bool mqtt_pubsubloop();

/*
* This function publishes to the /status/logs topic on MQTT
* Species: master, slave, GUI, app, lcp   (lcp --> local control panel)
* slave_id : (only required for slave)
* panel_id : (only required for panel)
* log_message: message that is to be logged  (TODO: set a character limit on this)
* Payload format:
{
“species”: “”
(if slave) “slave_id”: ,
(if panel) “panel_id”: ,
“timestamp”: “”,
“log” : “”
}
* TODO: Implement the timestamp feature in logging 
*/
void mqtt_log(String log_message);