/**
 * GPS Dog Collar — firmware on smart-pet-device-sdk.
 *
 * The SDK owns Wi-Fi + SoftAP provisioning, NTP, MQTT on
 * kennel/{kennelId}/gps/{deviceId}/*, LWT, OTA, command/ack and the offline
 * journal. This file is just the collar: read the GPS, publish `location`,
 * adapt the interval to movement, watch the battery.
 *
 * Topic migration: the pre-SDK firmware used `dogs/collar-001/location`. The SDK
 * publishes `kennel/{k}/gps/{id}/location` (spd_topics.h already knows the `gps`
 * device type). No `dogs/...` topics remain.
 *
 * The pre-SDK single-file firmware is kept as collar-firmware.legacy.cpp.
 *
 * Wiring (diagram.json): NEO-6M GPS on UART1 (RX GPIO16 / TX GPIO17),
 * battery divider on GPIO34, status LED GPIO2, button GPIO0.
 */
#include <SmartPetDevice.h>
#include <TinyGPSPlus.h>

constexpr int GPS_RX_PIN = 16;   // ESP32 RX  <- GPS TX
constexpr int GPS_TX_PIN = 17;   // ESP32 TX  -> GPS RX
constexpr int LED_PIN     = 2;
constexpr int BATTERY_PIN = 34;
constexpr int BUTTON_PIN  = 0;

constexpr uint32_t INTERVAL_MOVING_MS = 10000;
constexpr uint32_t INTERVAL_IDLE_MS   = 60000;
constexpr float    MOVE_THRESHOLD_M   = 5.0f;
constexpr int      BATTERY_CRITICAL   = 20;   // percent
constexpr uint32_t SLEEP_ON_CRITICAL_S = 300;

spd::SmartPetDevice dev("gps");
TinyGPSPlus gps;

static double lastLat = 0, lastLng = 0;
static bool   moving = false;
static uint32_t lastFixPub = 0;

static int batteryPct() {
  // ponytail: linear map of the raw ADC. Calibrate the voltage divider on a
  // real board (measure full and empty pack voltage).
  int raw = analogRead(BATTERY_PIN);
  return constrain(map(raw, 0, 4095, 0, 100), 0, 100);
}

static void pumpGps() {
  while (Serial1.available() > 0) gps.encode(Serial1.read());
}

static void publishFix() {
  if (!gps.location.isValid()) {
    Serial.println("[gps] no fix yet");
    return;
  }
  double lat = gps.location.lat();
  double lng = gps.location.lng();
  int batt = batteryPct();

  if (lastLat != 0 || lastLng != 0) {
    double moved = TinyGPSPlus::distanceBetween(lat, lng, lastLat, lastLng);
    moving = moved > MOVE_THRESHOLD_M;
    dev.setStatusInterval(moving ? INTERVAL_MOVING_MS : INTERVAL_IDLE_MS);
  }
  lastLat = lat;
  lastLng = lng;

  dev.publishLocation([&](JsonObject& o) {
    o["lat"] = lat;
    o["lng"] = lng;
    o["accuracyM"] = gps.hdop.isValid() ? gps.hdop.hdop() : 0.0;
    o["altitudeM"] = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
    o["speedKmh"] = gps.speed.isValid() ? gps.speed.kmph() : 0.0;
    o["headingDeg"] = gps.course.isValid() ? gps.course.deg() : 0.0;
    o["battery"] = batt;
    o["sats"] = gps.satellites.isValid() ? (int)gps.satellites.value() : 0;
    o["moving"] = moving;
  });
  lastFixPub = millis();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial1.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  dev.begin();

  // "Where is the dog" — publish a fix right now.
  dev.onCommand("get_location", [](JsonObjectConst, const String&) {
    pumpGps();
    publishFix();
    return true;
  });

  // Extra fields on the retained status message.
  dev.onStatusFill([](JsonObject& s) {
    s["battery"] = batteryPct();
    s["hasFix"]  = gps.location.isValid();
    s["moving"]  = moving;
  });

  dev.identifyFn_ = [](int secs) {
    for (int i = 0; i < secs * 2; ++i) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      delay(250);
    }
    digitalWrite(LED_PIN, LOW);
  };
}

void loop() {
  dev.loop();
  pumpGps();

  // Manual "publish now" button.
  static int lastBtn = HIGH;
  int btn = digitalRead(BUTTON_PIN);
  if (lastBtn == HIGH && btn == LOW) publishFix();
  lastBtn = btn;

  // Location stream, cadence = the SDK status interval (adapts to movement).
  uint32_t due = moving ? INTERVAL_MOVING_MS : INTERVAL_IDLE_MS;
  if (dev.isOnline() && millis() - lastFixPub > due) publishFix();

  // Status LED: fast blink = provisioning/offline, slow = no fix, on = tracking.
  if (dev.inProvisioning() || !dev.isOnline()) {
    digitalWrite(LED_PIN, (millis() / 300) % 2);
  } else if (!gps.location.isValid()) {
    digitalWrite(LED_PIN, (millis() / 1000) % 2);
  } else {
    digitalWrite(LED_PIN, HIGH);
  }

  // Battery critical → deep sleep; the SDK republishes on wake.
  if (batteryPct() < BATTERY_CRITICAL) {
    Serial.println("[collar] battery critical — deep sleep");
    dev.publishEvent("battery_critical", [](JsonObject& d) { d["battery"] = batteryPct(); });
    delay(200);
    ESP.deepSleep((uint64_t)SLEEP_ON_CRITICAL_S * 1000000ULL);
  }

  delay(10);
}
