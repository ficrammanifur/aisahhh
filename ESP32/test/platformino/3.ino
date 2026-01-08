#include <WiFi.h>
#include <PubSubClient.h>
#include "AudioTools.h"

#define I2S_BCLK 26
#define I2S_LRC  25
#define I2S_DOUT 27

// ===== WIFI =====
const char* ssid = "FRISS";
const char* password = "mamahfris";

// ===== MQTT =====
const char* mqtt_server = "192.168.1.17";
const int   mqtt_port   = 1883;
const char* topic_audio = "esp32/audio";

// ===== OBJECT =====
WiFiClient espClient;
PubSubClient mqtt(espClient);

I2SStream i2s;
AudioInfo audio_info(16000, 1, 16); // 16kHz, mono, 16bit

// ===== MQTT CALLBACK =====
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // kirim data audio langsung ke I2S
  i2s.write(payload, length);
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  // I2S CONFIG
  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.sample_rate = audio_info.sample_rate;
  cfg.bits_per_sample = audio_info.bits_per_sample;
  cfg.channels = audio_info.channels;

  cfg.pin_bck  = I2S_BCLK;
  cfg.pin_ws   = I2S_LRC;
  cfg.pin_data = I2S_DOUT;

  i2s.begin(cfg);

  // MQTT
  mqtt.setServer(mqtt_server, mqtt_port);
  mqtt.setCallback(mqttCallback);

  while (!mqtt.connected()) {
    Serial.print("Connecting MQTT...");
    if (mqtt.connect("ESP32-AUDIO")) {
      Serial.println("OK");
      mqtt.subscribe(topic_audio);
    } else {
      Serial.println("FAIL");
      delay(2000);
    }
  }

  Serial.println("🎧 ESP32 READY PLAY AUDIO");
}

// ===== LOOP =====
void loop() {
  mqtt.loop();
}
