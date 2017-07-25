#ifndef NETWORK_SETTINGS_H
#define NETWORK_SETTINGS_H

//To enter your wifi settings edit the following file and save it as wifi_settings.h

//WIFI SETTINGS
const char* SSID = "Wireless_Network_SSID";
const char* PSWD = "Password_123";

//MQTT SETTINGS
const char* mqtt_server = "{MQTT-SERVER}";
const char* mqtt_username = "{MQTT-USERNAME}";
const char* mqtt_password = "{MQTT-PASSWORD}";
//Port not needed if you are using your own, you can change this if you are using a service like cloudmqtt
const int mqtt_port = 1883;


#endif
