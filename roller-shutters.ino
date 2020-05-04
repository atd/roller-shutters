// Hardware usado -> Wemos D1 Pro

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

// ***************************************************************************
// Introducir los siguientes 11 valores para que funcione en vuestro entorno *
// ***************************************************************************

#define  RELAY1  14      // GPIO14 Pin D5 en Wemos D1 //Conectar al cable de subir persiana
#define  RELAY2  16      // GPIO16 Pin D0 en Wemos D1 //Conectar al cable de bajar persiana

#define wifi_ssid "Nombre de tu WiFi"
#define wifi_password "Password de tu WiFi"

#define mqtt_server "IP de tu Raspberry"
#define mqtt_port 1883  // Por defecto es 1883 pero puedes cambiarlo
#define mqtt_client "Nombre de cliente"  // Cualquier nombre que no se repita
#define mqtt_user "Usuario"
#define mqtt_password "Contraseña"     

#define intopic "topic por el que recibe los mensajes"  

#define segundos 24     // Introduce el tiempo en segundos que tarda la persiana en subir

//*****************************************************************************

int paso = segundos * 10;
int retardo = 0;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
unsigned int posicion = 0;
unsigned int lectura;

bool relay_stopped(){
  if (digitalRead (RELAY1) == HIGH && digitalRead (RELAY2) == HIGH){
    return true;
  } else {
    return false;
  }
}

void setup_wifi() {
    //WiFi.softAPdisconnect();
    //WiFi.disconnect();
    //WiFi.mode(WIFI_STA);
    //delay(100);
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);

    WiFi.begin(wifi_ssid, wifi_password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    String stringOne = "";
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        stringOne = stringOne + (char)payload[i];
    }
    Serial.println("\n");
  if (relay_stopped()){
    if (stringOne == "DECREASE"){
          if ((posicion + 20) > 100){
            retardo = 100 - posicion;
            posicion = 100;
          } else {
            posicion = posicion + 20;
            retardo = 20;
          }
          if (retardo > 0){
            digitalWrite(RELAY1, LOW);
          }
    } else if (stringOne == "INCREASE"){
          if (posicion < 20){
            retardo = posicion;
            posicion = 0;
          } else {
            posicion = posicion - 20;
            retardo = 20;
          }
          if (retardo > 0){
            digitalWrite(RELAY2, LOW);
          }
    } else {
        lectura = stringOne.toInt();
        if (lectura >= 0 && lectura < 101) {
            if (lectura > posicion) {
                digitalWrite(RELAY1, LOW);
                retardo = lectura-posicion;
            } else {
                digitalWrite(RELAY2, LOW);
                retardo = posicion-lectura;      
            }
        posicion = lectura;
        }
    }
    EEPROM.put(0, posicion);
    EEPROM.commit();
  }  
}

void reconnect() {
    // Bucle hasta conseguir la reconexión
    while (!client.connected()) {
        // Intento de conexión
        Serial.print("Attempting MQTT connection...");
        
        if (client.connect(mqtt_client, mqtt_user, mqtt_password)) {  // Cambiar el nombre si ya hay dispositivos conectados
            Serial.println("connected");
            client.subscribe(intopic);
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
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);
    setup_wifi();
    client.setClient(espClient);
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    EEPROM.get(0, posicion);   // Carga de la EEPROM la posición de la persiana
    if (posicion > 100) {
        posicion = 0;
        EEPROM.put(0, posicion);
        EEPROM.commit();
    }
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    if (retardo != 0) {
        delay(paso);
        retardo -= 1;
    } else {
        digitalWrite(RELAY1, HIGH);
        digitalWrite(RELAY2, HIGH);
    }

    // Las siguientes líneas son sólamente para comprobar el funcionamiento
    long now = millis();
    if (now - lastMsg > 10000) {
        lastMsg = now;
        Serial.print("Posición - ");
        Serial.println(posicion);
    }
}
