#include "mqtt.h"
#include "credentials.h"
#include "control.h"
#include "lanes.h"
#include "environment.h"
#include "slots.h"
#include <WiFi.h>
#include <ArduinoJson.h>

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

String slave_updates_topic = SLAVE_UPDATES_TOPIC;
String master_updates_topc = MASTER_UPDATES_TOPIC;
String signal_publish_topic = SIGNAL_PUBLISH_TOPIC;

// connects to the WiFi access point
static void connectToWiFi()
{
  Serial.print("Connecting to ");
  WiFi.begin(WIFI_SSID, PWD);
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("Connected.");
}

static void parse_mqtt_updates(byte *payload)
{
  Serial.print("Starting to parse updates on master...\n");
  const int capacity = JSON_OBJECT_SIZE(9);
  DynamicJsonBuffer jb(capacity);               // Memory pool
  JsonObject &parsed = jb.parseObject(payload); // Parse message
  if (!parsed.success())
  {
    Serial.println("Parsing failed");
    delay(5000);
    return;
  }

  env_set(parsed);
}

static void mqtt_callback(char *topic, byte *payload, unsigned int length)
{

  Serial.print("Callback occured\n");
  Serial.print("Topic: ");
  Serial.print(topic);
  Serial.print("\n");

  String control_topic = "/traffic/control";
  String slots_topic = "/traffic/slots";

  if (strcmp(topic, master_updates_topc.c_str()) == 0)
    parse_mqtt_updates(payload);

  else if (strcmp(topic, slave_updates_topic.c_str()) == 0)
    Serial.println("Update from slave rcvd.");

  else if (strcmp(topic, control_topic.c_str()) == 0)
  {
    const int capacity = JSON_OBJECT_SIZE(6);
    StaticJsonBuffer<capacity> jb;
    JsonObject &parsed = jb.parseObject(payload);
    setControlMode(parsed);
  }

  // if in slots topic
  else if (strcmp(topic, slots_topic.c_str()) == 0)
    parseSlots(payload);

}

// used to setup MQTT server and callback function
void mqtt_setup()
{
  connectToWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqtt_callback);
  mqttClient.setKeepAlive(1);
}

void mqtt_reconnect()
{
  Serial.println("Reconnecting to MQTT Broker..");
  String clientId = "ESP32Master";
  String username = "traffic-controller";
  String password = "f3JuzTfh3utBpIow";

  String lastwilltopic = "/status/master";
  String lastwillmessage = "{\"status\":\"Offline\"}";
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected())
  {
    if (mqttClient.connect(clientId.c_str(), username.c_str(), password.c_str(), lastwilltopic.c_str(), 1, true, lastwillmessage.c_str()))
    {
      Serial.println("Connected.");

      //Subscription list
      mqttClient.subscribe("/traffic/commands");
      mqttClient.subscribe("/traffic/slave_feedback");
      mqttClient.subscribe("/traffic/updates");
      mqttClient.subscribe("/traffic/slots");
      mqttClient.subscribe("/traffic/control");

      // Master subscribes to each client's status topics
      for (int i = 1; i <= 7; i++)
      {
        char s[50];
        sprintf(s, "/status/slave-%d", i);
        mqttClient.subscribe(s, 1);
      }

      while (!mqttClient.subscribe(slave_updates_topic.c_str(), 1))
      {
        Serial.println("Subsribe failed");
      }
    }
  }
  mqttClient.publish(lastwilltopic.c_str(), "{\"status\":\"Online\"}", true);
}

void mqtt_publish_signal()
{
  int n = env_getNumSlaves();

  char payload[500];
  const int capacity = JSON_OBJECT_SIZE(50);
  StaticJsonBuffer<capacity> jb;
  JsonObject &obj = jb.createObject();
  obj["n"] = n;
  for (int i = 1; i <= n; i++)
  {
    char s[10];
    sprintf(s, "S%d", i);
    obj[s] = env_getSlaveState(i);
  }

  for (int i = 1; i <= n; i++)
  {
    char s[10];
    sprintf(s, "t%d", i);
    JsonObject &t_n = obj.createNestedObject(s);
    t_n["red"] = env_getSlaveTimer(i, SlaveStates::RED);
    t_n["green"] = env_getSlaveTimer(i, SlaveStates::RED);
  }

  obj["mode"] = env_getMode();

  // Serializing into payload
  obj.printTo(payload);
  mqttClient.publish(SIGNAL_PUBLISH_TOPIC, payload);
}

bool mqtt_pubsubloop()
{
  return mqttClient.loop();
}

void mqtt_log(String log_message)
{
  char payload[1000];

  // Remove the species checking code based on context. When uploading code for master, slave, lcp
  const int capacity = JSON_OBJECT_SIZE(6); // Required is 5 --> take one more
  StaticJsonBuffer<capacity> jb;
  JsonObject &obj = jb.createObject();
  obj["species"] = "master";
  obj["log"] = log_message;

  // Serializing into payload
  obj.printTo(payload);
  mqttClient.publish("/status/logs", payload);
}

static void parseSlots(byte *payload)
{
  const int capacity = JSON_OBJECT_SIZE(9);
  DynamicJsonBuffer jb(capacity);               // Memory pool
  JsonObject &parsed = jb.parseObject(payload); // Parse message

  //Clear the slots and re-store 
  slots_clear();

  //Re-store
  slots_set(parsed);
}