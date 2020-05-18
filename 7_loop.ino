long lastMsg = 0;

void loop() {
  uptimeLoop();

  mqttLoop();
  
  shutters.loop();

  // Las siguientes líneas son sólamente para comprobar el funcionamiento
  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    
    publishUptime();
    shutters.publish();
  }
}
