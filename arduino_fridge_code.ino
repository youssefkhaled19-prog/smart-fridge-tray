// ============================================================
//  Smart Fridge Tray Monitor — ESP32 Firmware
//  Hardware: ESP32 + HX711 + Load Cell (5kg)
//  Protocol: MQTT over WiFi (WAN)
// ============================================================

#include <WiFi.h>
#include <PubSubClient.h>
#include <HX711.h>
#include <ArduinoJson.h>

// ── WiFi Credentials ─────────────────────────
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ── MQTT Broker ──────────────────────────────
const char* MQTT_BROKER = "smart-fridge-tray-production.up.railway.app";
const int   MQTT_PORT   = 1883;
const char* MQTT_CLIENT = "esp32_fridge_tray";
const char* MQTT_TOPIC  = "fridge/tray/count";
const char* MQTT_CMD    = "fridge/tray/cmd";

// ── HX711 Pin Wiring ─────────────────────────
#define HX711_DT   32
#define HX711_SCK  26

// ── Calibration & Item Settings ──────────────
float CALIBRATION_FACTOR = -7050.0;
float SINGLE_ITEM_WEIGHT = 150.0;
int   ALERT_THRESHOLD    = 2;

// ── Timing ───────────────────────────────────
const unsigned long PUBLISH_INTERVAL = 5000;
unsigned long lastPublish = 0;

// ── Objects ──────────────────────────────────
HX711 scale;
WiFiClient espClient;
PubSubClient mqtt(espClient);

void connectWiFi() {
  Serial.print("[WiFi] Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connected! IP: " + WiFi.localIP().toString());
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.println("[MQTT] Command received: " + msg);
  if (msg.indexOf("tare") >= 0) {
    scale.tare();
    Serial.println("[Scale] Tared");
  }
  if (msg.indexOf("threshold") >= 0) {
    StaticJsonDocument<128> doc;
    deserializeJson(doc, msg);
    if (doc.containsKey("value")) {
      ALERT_THRESHOLD = doc["value"].as<int>();
      Serial.println("[Config] New threshold: " + String(ALERT_THRESHOLD));
    }
  }
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("[MQTT] Connecting to broker...");
    if (mqtt.connect(MQTT_CLIENT)) {
      Serial.println(" connected!");
      mqtt.subscribe(MQTT_CMD);
    } else {
      Serial.print(" failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" — retry in 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  scale.begin(HX711_DT, HX711_SCK);
  scale.set_scale(CALIBRATION_FACTOR);
  Serial.println("[Scale] Taring... keep tray empty.");
  delay(2000);
  scale.tare();
  Serial.println("[Scale] Ready.");

  connectWiFi();

  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] Reconnecting...");
    connectWiFi();
  }

  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  unsigned long now = millis();
  if (now - lastPublish >= PUBLISH_INTERVAL) {
    lastPublish = now;

    float weight = scale.get_units(10);
    if (weight < 0) weight = 0;

    int count = (int)(weight / SINGLE_ITEM_WEIGHT + 0.5);
    bool lowStock = (count <= ALERT_THRESHOLD);

    StaticJsonDocument<200> doc;
    doc["count"]     = count;
    doc["weight_g"]  = (int)weight;
    doc["low_stock"] = lowStock;
    doc["threshold"] = ALERT_THRESHOLD;
    doc["timestamp"] = millis();

    char payload[200];
    serializeJson(doc, payload);

    mqtt.publish(MQTT_TOPIC, payload);

    Serial.printf("[Publish] count=%d | weight=%.1fg | low=%s\n",
                  count, weight, lowStock ? "YES" : "no");
  }
}
