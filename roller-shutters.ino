//
//  Original file from https://recretronica.wordpress.com/2018/11/05/openhab-19-controla-el-motor-de-tus-persianas/
//

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

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

#define wifiSSID "Wifi SSID"
#define wifiPassword "Wifi password"

#define mqttServer "MQTT server IP"
#define mqttPort 1883
#define mqttClient "Roller shutters"
#define mqttUser ""
#define mqttPassword ""

#define mqttTopic "mqtt_topic"

#define shutterRoundTime 24     // Time in seconds it takes to open or close the roller shutter

//*****************************************************************************

WiFiClient espClient;
PubSubClient client(espClient);

int shutterStep = shutterRoundTime * 10;
int shutterDelay = 0;
unsigned int shutterPosition = 0;
unsigned int newPosition;
long lastMsg = 0;

bool relayStopped() {
  if (digitalRead (RELAY_LEFT_UP) == HIGH && digitalRead (RELAY_LEFT_DOWN) == HIGH){
    return true;
  } else {
    return false;
  }
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

  if (relayStopped()) {
    newPosition = message.toInt();

    if (newPosition >= 0 && newPosition <= 100) {
      if (newPosition > shutterPosition) {
        digitalWrite(RELAY_LEFT_UP, LOW);

        shutterDelay = newPosition - shutterPosition;
      } else {
        digitalWrite(RELAY_LEFT_DOWN, LOW);

        shutterDelay = shutterPosition - newPosition;
      }
      shutterPosition = newPosition;
    }
  }
  EEPROM.put(0, shutterPosition);
  EEPROM.commit();
}

void reconnect() {
  // Bucle hasta conseguir la reconexión
  while (!client.connected()) {
    // Intento de conexión
    Serial.print("Attempting MQTT connection...");

    if (client.connect(mqttClient, mqttUser, mqttPassword)) {  // Cambiar el nombre si ya hay dispositivos conectados
      Serial.println("connected");
      client.subscribe(mqttTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      // Espera 5 segundos para un nuevo intento
      delay(5000);
    }
  }
}

void setup() {
  EEPROM.begin(512);
  Serial.begin(115200);

  pinMode(RELAY_LEFT_UP, OUTPUT);
  pinMode(RELAY_LEFT_DOWN, OUTPUT);
  digitalWrite(RELAY_LEFT_UP, HIGH);
  digitalWrite(RELAY_LEFT_DOWN, HIGH);

  setupWifi();

  client.setClient(espClient);
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);

  EEPROM.get(0, shutterPosition);   // Carga de la EEPROM la posición de la persiana

  if (shutterPosition > 100) {
    shutterPosition = 0;
    EEPROM.put(0, shutterPosition);
    EEPROM.commit();
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if (shutterDelay != 0) {
    delay(shutterStep);
    shutterDelay -= 1;
  } else {
    digitalWrite(RELAY_LEFT_UP, HIGH);
    digitalWrite(RELAY_LEFT_DOWN, HIGH);
  }

  // Las siguientes líneas son sólamente para comprobar el funcionamiento
  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    Serial.print("Position: ");
    Serial.println(shutterPosition);
  }
}
