#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <driver/i2s.h>

#define I2S_WS 35
#define I2S_SD 32
#define I2S_SCK 33
#define I2S_BCLK 27
#define I2S_LRC 26
#define I2S_DOUT 25

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Konfigurasi I2S untuk MIC INMP441
void i2s_install() {
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX), 
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
}

// Pin mapping MIC
void i2s_setpin_mic() {
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

// Pin mapping SPEAKER
void i2s_setpin_spk() {
  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = -1
  };
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal ditemukan!");
    while(true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 25);
  display.println("Tes Mic + Speaker + OLED");
  display.display();
  delay(2000);

  i2s_install();
  i2s_setpin_mic();
  i2s_start(I2S_NUM_0);
}

void loop() {
  const int BUFFER_SIZE = 1024;
  int32_t buffer[BUFFER_SIZE];
  size_t bytes_read;

  // Baca data dari MIC
  i2s_read(I2S_NUM_0, (void*)buffer, BUFFER_SIZE * sizeof(int32_t), &bytes_read, portMAX_DELAY);
  int samples_read = bytes_read / sizeof(int32_t);

  // Hitung RMS untuk OLED
  double sum = 0;
  for (int i = 0; i < samples_read; i++) {
    sum += (buffer[i] / 2147483648.0) * (buffer[i] / 2147483648.0);
  }
  double rms = sqrt(sum / samples_read);

  // Tampilkan di OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("🎤 Level Suara:");
  int bar = map(rms * 1000, 0, 50, 0, 128);
  display.fillRect(0, 20, bar, 20, SSD1306_WHITE);
  display.display();

  // Kirim ke Speaker
  i2s_setpin_spk();
  size_t bytes_written;
  i2s_write(I2S_NUM_0, buffer, bytes_read, &bytes_written, portMAX_DELAY);
  i2s_setpin_mic();
}
