void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message = "";

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message = message + (char)payload[i];
  }
  Serial.print("\n");

  char label[10];
  sprintf(label, "%s", topic + strlen(mqttCmndTopic) + 1);

  shutters.setPosition(label, message.toInt());
}

void mqttSetup() {
  client.setClient(espClient);
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(mqttClient, mqttUser, mqttPassword)) {
      Serial.println("connected");

      shutters.subscribe(client);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}

void mqttLoop() {
  if (!client.connected()) {
    mqttReconnect();
  }

  client.loop();
}
