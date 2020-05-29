class Shutter {
public:
  String label;
  unsigned int position;
  unsigned int steps;
  
  Shutter(String label, unsigned int pinUp, unsigned int pinDown) {
    this->label = label;
    position = 0;
    steps = 0;
    _pinUp = pinUp;
    _pinDown = pinDown;
  }

  void setup() {
    setupPins();
    stop();
    setupPosition();  
  }

  void subscribe() {
    char topic[50];
    sprintf(topic,"%s/%s", mqttCmndTopic, label.c_str());
    client.subscribe(topic);
  }

  void setPosition(int newPosition) {
    Serial.print("setPosition ");
    Serial.print(label + ": ");
    Serial.println(String(newPosition));
    
    if (relayStopped()) {
      if (newPosition >= 0 && newPosition <= 100) {
        if (newPosition > position) {
          digitalWrite(_pinUp, LOW);
  
          steps = newPosition - position;
        } else {
          digitalWrite(_pinDown, LOW);
  
          steps = position - newPosition;
        }
        position = newPosition;
      }
    }
    //EEPROM.put(0, shutterPosition);
    //EEPROM.commit();
  }

  void afterStep() {
    if (steps == 0) {
      return;
    }
    
    steps -= 1;
    
    if (steps == 0) {
      stop();
    }
  }

  void stop() {
    digitalWrite(_pinUp, HIGH);
    digitalWrite(_pinDown, HIGH);
  }

  void publish() {
    Serial.println(label + " position: " + String(position));
  }

private:
  unsigned int _pinUp;
  unsigned int _pinDown;

  bool relayStopped() {
    return digitalRead(_pinUp) == HIGH && digitalRead(_pinDown) == HIGH;
  }

  void setupPins() {
    pinMode(_pinUp, OUTPUT);
    pinMode(_pinDown, OUTPUT);
  }

  void setupPosition() {
    //EEPROM.get(0, position);   // Carga de la EEPROM la posición de la persiana

    if (position > 100) {
      position = 0;
      //EEPROM.put(0, position);
      //EEPROM.commit();
    }
  }
};
