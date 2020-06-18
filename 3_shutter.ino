class Shutter {
public:
  uint8_t index;
  String label;
  uint8_t position;
  uint8_t newPosition;
  char statTopic[50];
  char cmndTopic[50];
  
  Shutter(unsigned int index, String label, unsigned int pinUp, unsigned int pinDown) {
    this->index = index;
    this->label = label;

    sprintf(statTopic,"%s/%s", mqttStatTopic, label.c_str());
    sprintf(cmndTopic,"%s/%s", mqttCmndTopic, label.c_str());
    
    position = 0;
    newPosition = 0;
    
    _pinUp = pinUp;
    _pinDown = pinDown;
  }

  void setup() {
    setupPins();
    stop();
    setupPosition();  
  }

  void subscribe() {
    client.subscribe(cmndTopic);
  }

  int steps() {
    return newPosition - position;
  }

  void setPosition(String updatePosition) {
    Serial.print("setPosition ");
    Serial.print(label + ": ");
    Serial.println(updatePosition);

    if (updatePosition == "STOP") {
      newPosition = position;
      stop();
    } else {
      newPosition = updatePosition.toInt();

      updateRelays();  
    }
  }
  

  void afterStep() {
    if (steps() == 0) {
      return;
    }

    position += (steps() > 0 ? 1 : -1);

    publishMqtt();
    
    if (steps() == 0) {
      savePosition();
      stop();
    }
  }

  void savePosition() {
    Serial.println("Saving " + String(index) + " " + String(position)); 
    EEPROM.put(index, position);
    EEPROM.commit();
  }
  
  void stop() {
    digitalWrite(_pinUp, HIGH);
    digitalWrite(_pinDown, HIGH);
  }

  void publishSerial() {
    Serial.println(label + " position: " + String(position));
  }
  
  void publishMqtt() {
    char positionChar[3];
    sprintf(positionChar, "%i", position);
    client.publish(statTopic, positionChar, true);
  }

private:
  unsigned int _pinUp;
  unsigned int _pinDown;

  void updateRelays() {
    if (steps() > 0) {
      if (movingDown()) {
        stop();
      }

      digitalWrite(_pinUp, LOW);
    } else if (steps() < 0) {
      if (movingUp()) {
        stop();
      }
      
      digitalWrite(_pinDown, LOW);
    } else {
      stop();
    }
    
  }

  bool movingUp() {
    return digitalRead(_pinUp) == LOW;
  }

  bool movingDown() {
    return digitalRead(_pinDown) == LOW;
  }

  bool relayStopped() {
    return digitalRead(_pinUp) == HIGH && digitalRead(_pinDown) == HIGH;
  }

  void setupPins() {
    pinMode(_pinUp, OUTPUT);
    pinMode(_pinDown, OUTPUT);
  }

  void setupPosition() {
    EEPROM.get(index, position);
    newPosition = position;

    Serial.println("Loading " + String(index) + " " + String(position)); 
  }
};
