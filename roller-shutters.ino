//
//  Original file from https://recretronica.wordpress.com/2018/11/05/openhab-19-controla-el-motor-de-tus-persianas/
//

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

// ***************
// Configuration *
// ***************

// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#define  RELAY_LEFT_UP    5       // GPIO5 Pin D1 in Wemos D1
#define  RELAY_LEFT_DOWN  4       // GPIO4 Pin D2 in Wemos D1
#define  RELAY_MIDDLE_UP    0       // GPIO0 Pin D3 in Wemos D1
#define  RELAY_MIDDLE_DOWN  14      // GPIO14 Pin D5 in Wemos D1
#define  RELAY_RIGHT_UP    12      // GPIO12 Pin D6 in Wemos D1
#define  RELAY_RIGHT_DOWN  13      // GPIO13 Pin D7 in Wemos D1
#define  shuttersSize 3

#define wifiSSID "Wifi SSID"
#define wifiPassword "Wifi password"

#define mqttServer "MQTT server IP"
#define mqttPort 1883
#define mqttClient "Roller shutters"
#define mqttUser ""
#define mqttPassword ""

#define mqttStatTopic "ew/stat"
#define mqttCmndTopic "ew/cmnd"
#define mqttUptimeTopic "ew/uptime"

#define shutterRoundTime 24     // Time in seconds it takes to open or close the roller shutter

//*****************************************************************************

WiFiClient espClient;
PubSubClient client(espClient);
