const int stepDelay = shutterRoundTime * 10;

class ShutterList {
public:
  Shutter *list[shuttersSize];
  
  ShutterList() {
    list[0] = new Shutter(0, "left", RELAY_LEFT_UP, RELAY_LEFT_DOWN);
    list[1] = new Shutter(1, "middle", RELAY_MIDDLE_UP, RELAY_MIDDLE_DOWN);
    list[2] = new Shutter(2, "right", RELAY_RIGHT_UP, RELAY_RIGHT_DOWN);
  }

  Shutter *find(char* label) {
    for (int i = 0; i < shuttersSize; i++) {
      if (list[i]->label == String(label)) {
        return list[i];
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

  void loop() {
    if (noSteps()) {
      return;
    }
    
    delay(stepDelay);
    
    afterStep();
  }

  void onMqttConnected() {
    subscribe();
    publishMqtt();
  }

  void subscribe() {
    for (int i = 0; i < shuttersSize; i++) {
      list[i]->subscribe();
    }
  }

  void setPosition(char * label, String newPosition) {
    find(label)->setPosition(newPosition);
  }

  void publishSerial() {
    for (int i = 0; i < shuttersSize; i++) {
      list[i]->publishSerial();
    }
  }

  void publishMqtt() {
    for (int i = 0; i < shuttersSize; i++) {
      list[i]->publishMqtt();
    }
  }
private:
  bool noSteps() {
    for (int i = 0; i < shuttersSize; i++) {
      if (list[i]->steps() != 0) {
        return false;
      }
    }    
    return true;
  }

  void afterStep() {
    for (int i = 0; i < shuttersSize; i++) {
      list[i]->afterStep();
    }
  }
};

ShutterList shutters;
