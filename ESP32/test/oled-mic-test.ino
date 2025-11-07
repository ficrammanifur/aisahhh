#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <driver/i2s.h>
#include <math.h>

// ==== OLED 0.92" ====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ==== I2S MIC ====
#define SAMPLE_BUFFER_SIZE 512
#define SAMPLE_RATE 16000 // Bisa 8000 juga
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_33  // BCLK
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_35 // LRCLK / WS
#define I2S_MIC_SERIAL_DATA GPIO_NUM_32   // DOUT

i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};

i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA
};

int32_t raw_samples[SAMPLE_BUFFER_SIZE];

void setup() {
  Serial.begin(115200);

  // Inisialisasi OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Inisialisasi I2S
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &i2s_mic_pins);
}

float calculateRMS(int32_t *samples, int num_samples) {
  double sum = 0;
  for (int i = 0; i < num_samples; i++) {
    float val = (float)samples[i] / 2147483648.0; // normalize 32-bit signed
    sum += val * val;
  }
  return sqrt(sum / num_samples);
}

void loop() {
  size_t bytes_read = 0;
  i2s_read(I2S_NUM_0, raw_samples, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
  int samples_read = bytes_read / sizeof(int32_t);

  float rms = calculateRMS(raw_samples, samples_read);
  
  // ==== Update OLED ====
  display.clearDisplay();

  // Tampilkan RMS
  display.setCursor(0,0);
  display.printf("RMS: %.3f", rms);

  // Tampilkan gelombang statis
  int16_t center = SCREEN_HEIGHT / 2;
  int16_t max_height = SCREEN_HEIGHT / 2 - 5; // margin 5 px
  int16_t wave_height = (int16_t)(rms * max_height * 10); // dikali 10 agar terlihat lebih besar

  if(wave_height > max_height) wave_height = max_height;

  int16_t top = center - wave_height;
  int16_t bottom = center + wave_height;

  display.drawLine(0, top, SCREEN_WIDTH, top, SSD1306_WHITE);
  display.drawLine(0, bottom, SCREEN_WIDTH, bottom, SSD1306_WHITE);

  // Garis tengah
  display.drawLine(0, center, SCREEN_WIDTH, center, SSD1306_WHITE);

  display.display();

  delay(50); // update tiap 50ms
}
