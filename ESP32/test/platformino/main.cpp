#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <driver/i2s.h>

// ================= WIFI & MQTT =================
#define WIFI_SSID "FRISS"
#define WIFI_PASS "mamahfris"

#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_SUB_TOPIC "package/chat"
#define MQTT_PUB_TOPIC "package/status"

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ================= SERVO =================
#define SERVO_PIN 19

// ================= I2S =================
#define I2S_WS 25
#define I2S_BCK 26
#define I2S_MIC_SD 33
#define I2S_SPK_DIN 27

#define I2S_MIC_PORT I2S_NUM_0
#define I2S_SPK_PORT I2S_NUM_1

// ================= OBJECT =================
WiFiClient espClient;
PubSubClient mqtt(espClient);
Servo lockServo;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= STATE =================
bool boxOpen = false;
unsigned long openUntil = 0;

// ================= OLED =================
void showText(String a, String b = "", String c = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(8, 20);
  display.println(a);
  display.setCursor(8, 32);
  display.println(b);
  display.setCursor(8, 44);
  display.println(c);

  display.display();
}

// ================= SERVO =================
void servoOpen() {
  lockServo.write(0);
  Serial.println("✅ SERVO OPEN");
}

void servoClose() {
  lockServo.write(180);
  Serial.println("🔒 SERVO CLOSED");
}

// ================= WIFI =================
void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected");
}

// ================= MQTT CALLBACK =================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (uint8_t i = 0; i < length; i++) msg += (char)payload[i];
  msg.toLowerCase();

  Serial.println("📥 MQTT: " + msg);

  if (msg.startsWith("name:") && !boxOpen) {
    servoOpen();
    boxOpen = true;
    openUntil = millis() + 15000;

    mqtt.publish(MQTT_PUB_TOPIC, "opened");
    showText("Selamat!", "Paket diterima");
  }

  if (msg == "close_box") {
    servoClose();
    boxOpen = false;
    mqtt.publish(MQTT_PUB_TOPIC, "closed");
    showText("Kotak ditutup");
  }
}

// ================= MQTT =================
void setupMQTT() {
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  while (!mqtt.connected()) {
    Serial.print("Connecting MQTT...");
    if (mqtt.connect("SmartBoxESP32")) {
      Serial.println(" OK");
      mqtt.subscribe(MQTT_SUB_TOPIC);
    } else {
      Serial.println(" FAIL");
      delay(3000);
    }
  }
}

// ================= I2S MIC =================
void setupI2SMic() {
  i2s_config_t mic_cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 256,
    .use_apll = false
  };

  i2s_pin_config_t mic_pin = {
    .bck_io_num = I2S_BCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_MIC_SD
  };

  i2s_driver_install(I2S_MIC_PORT, &mic_cfg, 0, NULL);
  i2s_set_pin(I2S_MIC_PORT, &mic_pin);
  Serial.println("🎤 INMP441 READY");
}

// ================= I2S SPEAKER =================
void setupI2SSpeaker() {
  i2s_config_t spk_cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 256,
    .use_apll = false
  };

  i2s_pin_config_t spk_pin = {
    .bck_io_num = I2S_BCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_SPK_DIN,
    .data_in_num = -1
  };

  i2s_driver_install(I2S_SPK_PORT, &spk_cfg, 0, NULL);
  i2s_set_pin(I2S_SPK_PORT, &spk_pin);
  Serial.println("🔊 MAX98357A READY");
}

// ================= AUDIO LOOPBACK =================
int32_t micBuf[256];
int16_t spkBuf[256];

void audioLoopback() {
  size_t bytesRead, bytesWritten;

  i2s_read(I2S_MIC_PORT, micBuf, sizeof(micBuf), &bytesRead, portMAX_DELAY);
  int samples = bytesRead / 4;

  for (int i = 0; i < samples; i++) {
    spkBuf[i] = micBuf[i] >> 14;
  }

  i2s_write(I2S_SPK_PORT, spkBuf, samples * 2, &bytesWritten, portMAX_DELAY);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  lockServo.attach(SERVO_PIN);
  servoClose();

  setupWiFi();
  setupMQTT();

  setupI2SMic();
  setupI2SSpeaker();

  showText("Smart Package", "Audio Ready");
}

// ================= LOOP =================
void loop() {
  if (!mqtt.connected()) setupMQTT();
  mqtt.loop();

  audioLoopback();

  if (boxOpen && millis() > openUntil) {
    servoClose();
    boxOpen = false;
    mqtt.publish(MQTT_PUB_TOPIC, "closed");
    showText("Timeout", "Kotak ditutup");
  }
}
