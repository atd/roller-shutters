void setup() {
  uptimeSetup();

  //EEPROM.begin(512);
  Serial.begin(115200);

  shutters.setup();
  
  wifiSetup();
  mqttSetup();
}
