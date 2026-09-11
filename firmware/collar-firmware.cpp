/**
 * GPS Dog Collar Firmware
 * ESP32-based with GPS tracking and MQTT communication
 * 
 * Features:
 * - GPS location tracking
 * - MQTT communication
 * - Power management
 * - OTA updates
 * - Geofencing
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <TinyGPS++.h>
#include <ArduinoJson.h>

void publishLocation(); // defined below; used before its definition

// ============== CONFIGURATION ==============
// WiFi
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// MQTT
const char* MQTT_SERVER = "YOUR_MQTT_BROKER";
const int MQTT_PORT = 1883;
const char* MQTT_USER = "mqtt-user";
const char* MQTT_PASSWORD = "mqtt-password";
const char* MQTT_DEVICE_TOPIC = "dogs/collar-001";

// Device ID (unique for each collar)
const char* DEVICE_ID = "collar-001";

// Hardware Pins
const int GPS_TX_PIN = 16;
const int GPS_RX_PIN = 17;
const int LED_PIN = 2;
const int BATTERY_PIN = 34;
const int BUTTON_PIN = 0;

// ============== POWER SETTINGS ==============
const int TRACKING_INTERVAL_MOVING = 10;    // seconds when moving
const int TRACKING_INTERVAL_IDLE = 60;       // seconds when idle
const int TRACKING_INTERVAL_SLEEP = 300;     // seconds in sleep
const int BATTERY_CRITICAL = 20;            // percentage

// ============== GLOBALS ==============
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
TinyGPSPlus gps;

unsigned long lastPublish = 0;
int currentInterval = TRACKING_INTERVAL_IDLE;
bool isMoving = false;
float lastLat = 0;
float lastLng = 0;

// ============== GPS ==============
void initGPS() {
  Serial1.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS initialized");
}

bool getGPSLocation(float &lat, float &lng, float &accuracy) {
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }
  
  if (gps.location.isUpdated()) {
    lat = gps.location.lat();
    lng = gps.location.lng();
    accuracy = gps.hdop.hdop();
    return true;
  }
  return false;
}

// ============== POWER MANAGEMENT ==============
int readBatteryLevel() {
  int raw = analogRead(BATTERY_PIN);
  // Map 0-4095 to 0-100%
  // Adjust based on your voltage divider
  return map(raw, 0, 4095, 0, 100);
}

void goToSleep(int seconds) {
  Serial.printf("Going to sleep for %d seconds\n", seconds);
  ESP.deepSleep(seconds * 1000000);
}

// ============== WIFI ==============
void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi failed!");
  }
}

// ============== MQTT ==============
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);
  
  // Parse command
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);
  
  if (!error) {
    const char* cmd = doc["command"];
    if (strcmp(cmd, "get_location") == 0) {
      publishLocation();
    } else if (strcmp(cmd, "set_interval") == 0) {
      currentInterval = doc["interval"];
    } else if (strcmp(cmd, "reboot") == 0) {
      ESP.restart();
    }
  }
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "DogCollar-";
    clientId += DEVICE_ID;
    
    if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected!");
      
      // Subscribe to commands
      String commandTopic = String(MQTT_DEVICE_TOPIC) + "/command";
      mqttClient.subscribe(commandTopic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}

void publishLocation() {
  float lat, lng, accuracy;
  
  if (getGPSLocation(lat, lng, accuracy)) {
    int battery = readBatteryLevel();
    
    StaticJsonDocument<512> doc;
    doc["deviceId"] = DEVICE_ID;
    doc["latitude"] = lat;
    doc["longitude"] = lng;
    doc["accuracy"] = accuracy;
    doc["altitude"] = gps.altitude.meters();
    doc["speed"] = gps.speed.kmph();
    doc["heading"] = gps.course.deg();
    doc["battery"] = battery;
    doc["timestamp"] = millis();
    doc["wifiRssi"] = WiFi.RSSI();
    
    String json;
    serializeJson(doc, json);
    
    String topic = String(MQTT_DEVICE_TOPIC) + "/location";
    mqttClient.publish(topic.c_str(), json.c_str());
    
    Serial.println(json);
    
    // Detect movement
    if (lastLat != 0 && lastLng != 0) {
      float distance = TinyGPSPlus::distanceBetween(lat, lng, lastLat, lastLng);
      isMoving = distance > 5; // 5 meters threshold
      currentInterval = isMoving ? TRACKING_INTERVAL_MOVING : TRACKING_INTERVAL_IDLE;
    }
    
    lastLat = lat;
    lastLng = lng;
  }
}

// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  initGPS();
  connectWiFi();
  
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  connectMQTT();
  
  // Blink LED to indicate startup
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  
  Serial.println("Dog Collar initialized!");
}

// ============== LOOP ==============
void loop() {
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  
  mqttClient.loop();
  
  // Publish location at intervals
  if (millis() - lastPublish > currentInterval * 1000) {
    publishLocation();
    lastPublish = millis();
  }
  
  // Battery critical - go to deep sleep
  if (readBatteryLevel() < BATTERY_CRITICAL) {
    Serial.println("Battery critical!");
    goToSleep(TRACKING_INTERVAL_SLEEP);
  }
  
  delay(100);
}
