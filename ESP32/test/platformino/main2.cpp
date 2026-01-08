#include <WiFi.h>
#include <PubSubClient.h>
#include <driver/i2s.h>

const char* ssid = "FRISS";
const char* password = "mamahfris";

const char* mqtt_server = "broker.hivemq.com";
const char* audio_topic = "package/audio";

WiFiClient espClient;
PubSubClient client(espClient);

// ===== INMP441 PINS =====
#define I2S_WS   25
#define I2S_SD   33
#define I2S_SCK  26

#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000
#define BUFFER_SAMPLES 256   // samples
#define BUFFER_BYTES (BUFFER_SAMPLES * 2)

int32_t i2sBuffer[BUFFER_SAMPLES];
int16_t pcmBuffer[BUFFER_SAMPLES];

void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // 🔥 INMP441
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 256,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
}

void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect("ESP32-INMP441")) {
      Serial.println("✅ MQTT Connected");
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("✅ WiFi Connected");

  client.setServer(mqtt_server, 1883);
  setupI2S();
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  size_t bytesRead;
  i2s_read(I2S_PORT, i2sBuffer, sizeof(i2sBuffer), &bytesRead, portMAX_DELAY);

  int samples = bytesRead / 4;

  for (int i = 0; i < samples; i++) {
    pcmBuffer[i] = (int16_t)(i2sBuffer[i] >> 8); // 🔥 penting
  }

  client.publish(audio_topic, (uint8_t*)pcmBuffer, samples * 2);
}
