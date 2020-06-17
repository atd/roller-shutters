struct Uptime {
    // d, h, m, s and ms record the current uptime.
    int d;                      // Days (0-)
    int h;                      // Hours (0-23)
    int m;                      // Minutes (0-59)
    int s;                      // Seconds (0-59)
    int ms;                     // Milliseconds (0-999)

    // The value of millis() the last the the above was updated.
    // Note: this value will wrap after slightly less than 50 days.
    // In contrast, the above values won't wrap for more than 5
    // million years.
    unsigned long last_millis;
};
struct Uptime uptime;

void uptimeSetup() {
  uptime.d = 0;
  uptime.h = 0;
  uptime.m = 0;
  uptime.s = 0;
}


// Update the uptime information.
//
// As long as you call this once every 20 days or so, or more often,
// it should handle the wrap-around in millis() just fine.
void uptimeLoop() {
  unsigned long now = millis();
  unsigned long delta = now - uptime.last_millis;
  uptime.last_millis = now;

  uptime.ms += delta;

  // Avoid expensive floating point arithmetic if it isn't needed.
  if (uptime.ms < 1000)
  return;

  uptime.s += uptime.ms / 1000;
  uptime.ms %= 1000;

  // Avoid expensive floating point arithmetic if it isn't needed.
  if (uptime.s < 60)
  return;

  uptime.m += uptime.s / 60;
  uptime.s %= 60;

  // We could do an early return here (and before the update of d)
  // as well, but what if the entire loop runs too slowly when we
  // need to update update.d?  Beter to run all the code at least
  // once a minute, so that performance problems have a chance of
  // beeing seen regularly, and not just once per day.

  uptime.h += uptime.m / 60;
  uptime.m %= 60;
  uptime.d += uptime.h / 24;
  uptime.h %= 24;
}

void publishUptime() {
  StaticJsonDocument<100> json;

  json["d"] = uptime.d;
  json["h"] = uptime.h;
  json["m"] = uptime.m;
  json["s"] = uptime.s;

  char payload[100];
  serializeJson(json, payload);

  client.publish(mqttUptimeTopic, payload);
}
