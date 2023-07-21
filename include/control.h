/*
* This file is used to set the control mode for the ESP32 
*/

#include <ArduinoJson.h>

enum ControlMode
{
    AUTO, MANUAL
};

/*
Parses JsonObject to set the control mode 
Control mode: ControlMode::AUTO / ControlMode::MANUAL
Format:
{
    "panel_id": ,
    "mode":
}
If the panel_id does not match, then it sends an mqtt log
*/
void setControlMode(JsonObject &parsed);


/*
Returns the current control mode 
Modes:
*/
int getControlMode();