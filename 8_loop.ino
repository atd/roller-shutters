long lastMsg = 0;

void loop() {
  uptimeLoop();

  mqttLoop();
  
  shutters.loop();

  // Check operation
  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    
    publishUptime();
    shutters.publishSerial();
    Serial.println("[EEPROM] " + String(EEPROM.percentUsed()) + "% used");
  }
}
