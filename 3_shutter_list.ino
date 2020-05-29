const int stepDelay = shutterRoundTime * 10;

class ShutterList {
public:
  Shutter *list[shuttersSize];
  
  ShutterList() {
    list[0] = new Shutter("left", RELAY_LEFT_UP, RELAY_LEFT_DOWN);
    list[1] = new Shutter("middle", RELAY_MIDDLE_UP, RELAY_MIDDLE_DOWN);
    list[2] = new Shutter("right", RELAY_RIGHT_UP, RELAY_RIGHT_DOWN);
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

  void subscribe() {
    for (int i = 0; i < shuttersSize; i++) {
      list[i]->subscribe();
    }
  }

  void setPosition(char * label, int newPosition) {
    find(label)->setPosition(newPosition);
  }

  void publish() {
    for (int i = 0; i < shuttersSize; i++) {
      list[i]->publish();
    }
  }

private:
  bool noSteps() {
    for (int i = 0; i < shuttersSize; i++) {
      if (list[i]->steps != 0) {
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
