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

#define mqttPositionTopic "ew/position"
#define mqttUptimeTopic "ew/uptime"

#define shutterRoundTime 24     // Time in seconds it takes to open or close the roller shutter

//*****************************************************************************

struct Uptime {
    // d, h, m, s and ms record the current uptime.
    int d;                      // Days (0-)
    int h;                      // Hours (0-23)
    int m;                      // Minutes (0-59)
    int s;                      // Seconds (0-59)
    int ms;                     // Milliseconds (0-999)

    // The value of millis() the last the the above was updated.
    // Note: this value will wrap after slightly less than 50 days.
    // In contrast, the above values won't wrap for more than 5
    // million years.
    unsigned long last_millis;
};
struct Uptime uptime;

WiFiClient espClient;
PubSubClient client(espClient);

int shutterStep = shutterRoundTime * 10;
int shutterDelay = 0;
long lastMsg = 0;

class Shutter {
public:
  String label;
  unsigned int position;
  
  Shutter(String label, unsigned int pinUp, unsigned int pinDown) {
    this->label = label;
    position = 0;
    _pinUp = pinUp;
    _pinDown = pinDown;
    _delay = 0;
  }

  void setup() {
    setupPins();
    setupPosition();  
  }

  void subscribe(PubSubClient client) {
    char topic[50];
    sprintf(topic,"%s/%s", mqttPositionTopic, label.c_str());
    client.subscribe(topic);
  }

  void setPosition(int newPosition) {
    Serial.print("setPosition ");
    Serial.print(label + ": ");
    Serial.println(String(newPosition));
    if (relayStopped()) {   
      if (newPosition >= 0 && newPosition <= 100) {
        if (newPosition > position) {
          digitalWrite(_pinUp, LOW);
  
          _delay = newPosition - position;
        } else {
          digitalWrite(_pinDown, LOW);
  
          _delay = position - newPosition;
        }
        position = newPosition;
      }
    }
    //EEPROM.put(0, shutterPosition);
    //EEPROM.commit();
  }
  
private:
  unsigned int _pinUp;
  unsigned int _pinDown;
  unsigned int _delay;

  bool relayStopped() {
    return digitalRead(_pinUp) == HIGH && digitalRead(_pinDown) == HIGH;
  }

  void setupPins() {
    pinMode(_pinUp, OUTPUT);
    pinMode(_pinDown, OUTPUT);
    digitalWrite(_pinUp, HIGH);
    digitalWrite(_pinDown, HIGH);
  }

  void setupPosition() {
    //EEPROM.get(0, position);   // Carga de la EEPROM la posición de la persiana

    if (position > 100) {
      position = 0;
      //EEPROM.put(0, position);
      //EEPROM.commit();
    }
  }
};

class ShutterList {
public:
  Shutter *list[shuttersSize];
  
  ShutterList() {
    list[0] = new Shutter("left", RELAY_LEFT_UP, RELAY_LEFT_DOWN);
    list[1] = new Shutter("middle", RELAY_MIDDLE_UP, RELAY_MIDDLE_DOWN);
    list[2] = new Shutter("right", RELAY_RIGHT_UP, RELAY_RIGHT_DOWN);
  }

  Shutter find(char* label) {
    for (int i = 0; i < shuttersSize; i++) {
      if (list[i]->label == String(label)) {
        return *list[i];
      }
    }
    Serial.print("Invalid shutter label: ");
    Serial.println(label);
  }

  void setup() {
    for (int i = 0; i < shuttersSize; i++) {
      list[i]->setup();
    }
  }

  void subscribe(PubSubClient client) {
    for (int i = 0; i < shuttersSize; i++) {
      list[i]->subscribe(client);
    }
  }

  void setPosition(char * label, int newPosition) {
    find(label).setPosition(newPosition);
  }
};

ShutterList shutters;

void setupUptime() {
  uptime.d = 0;
  uptime.h = 0;
  uptime.m = 0;
  uptime.s = 0;
}

void setupWifi() {
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifiSSID);

  WiFi.begin(wifiSSID, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message = "";

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message = message + (char)payload[i];
  }
  Serial.println("\n");

  char label[10];
  sprintf(label, "%s", topic + strlen(mqttPositionTopic) + 1);

  shutters.setPosition(label, message.toInt());
}

void setupMqtt() {
  client.setClient(espClient);
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
}

void setup() {
  setupUptime();

  //EEPROM.begin(512);
  Serial.begin(115200);

  shutters.setup();
  
  setupWifi();
  setupMqtt();

}

// Update the uptime information.
//
// As long as you call this once every 20 days or so, or more often,
// it should handle the wrap-around in millis() just fine.
void uptimeLoop() {
  unsigned long now = millis();
  unsigned long delta = now - uptime.last_millis;
  uptime.last_millis = now;

  uptime.ms += delta;

  // Avoid expensive floating point arithmetic if it isn't needed.
  if (uptime.ms < 1000)
  return;

  uptime.s += uptime.ms / 1000;
  uptime.ms %= 1000;

  // Avoid expensive floating point arithmetic if it isn't needed.
  if (uptime.s < 60)
  return;

  uptime.m += uptime.s / 60;
  uptime.s %= 60;

  // We could do an early return here (and before the update of d)
  // as well, but what if the entire loop runs too slowly when we
  // need to update update.d?  Beter to run all the code at least
  // once a minute, so that performance problems have a chance of
  // beeing seen regularly, and not just once per day.

  uptime.h += uptime.m / 60;
  uptime.m %= 60;
  uptime.d += uptime.h / 24;
  uptime.h %= 24;
}

void publishUptime() {
  StaticJsonDocument<100> json;

  json["d"] = uptime.d;
  json["h"] = uptime.h;
  json["m"] = uptime.m;
  json["s"] = uptime.s;

  char payload[100];
  serializeJson(json, payload);

  client.publish(mqttUptimeTopic, payload);
}

void reconnect() {
  // Bucle hasta conseguir la reconexión
  while (!client.connected()) {
    // Intento de conexión
    Serial.print("Attempting MQTT connection...");

    if (client.connect(mqttClient, mqttUser, mqttPassword)) {  // Cambiar el nombre si ya hay dispositivos conectados
      Serial.println("connected");

      shutters.subscribe(client);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      // Espera 5 segundos para un nuevo intento
      delay(5000);
    }
  }
}

void loop() {
  uptimeLoop();

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

//  if (shutterDelay != 0) {
//    delay(shutterStep);
//    shutterDelay -= 1;
//  } else {
//    digitalWrite(RELAY_LEFT_UP, HIGH);
//    digitalWrite(RELAY_LEFT_DOWN, HIGH);
//  }

  // Las siguientes líneas son sólamente para comprobar el funcionamiento
  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    
    publishUptime();

//    Serial.print("Position: ");
//    Serial.println(shutterPosition);
  }
}
