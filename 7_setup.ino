void setup() {
  Serial.begin(115200);

  delay(4000);
  
  uptimeSetup();

  eepromSetup();

  shutters.setup();
  
  wifiSetup();
  mqttSetup();
}
