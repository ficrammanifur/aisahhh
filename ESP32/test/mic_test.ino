/* mic_inmp441_oled_debug.ino
   Improved INMP441 I2S read + RMS + OLED debug
   Wiring (per user):
   INMP441:
     VCC -> 3.3V
     GND -> GND
     L/R -> GND (left)
     WS  -> GPIO35
     SCK -> GPIO33
     SD  -> GPIO32

   OLED:
     SDA -> 21
     SCL -> 22
     VCC -> 3.3V, GND->GND
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "driver/i2s.h"
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// I2S config
#define I2S_NUM       I2S_NUM_0
#define SAMPLE_RATE   16000
#define BUF_SAMPLES   512   // number of 32-bit samples per read

// smoothing
float level_smooth = 0.0;
const float ALPHA = 0.85; // smoothing factor (higher = slower, lower = faster)

// baseline calibration
float baseline = 0.0;
const int BASELINE_MS = 2000; // measure ambient for 2s

void setup_i2s_rx() {
  // RX config: read 32-bit words left-justified (we'll shift >>8 to get 24-bit)
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // only left channel
    .communication_format = I2S_COMM_FORMAT_STAND_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 4,
    .dma_buf_len = 512,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = 33,   // SCK / BCLK
    .ws_io_num = 35,    // WS / LRCLK
    .data_out_num = -1,
    .data_in_num = 32   // SD from mic
  };

  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);
  // set clock if needed (usually fine)
}

float calc_rms_from_buf(int32_t *buf, size_t samples, int32_t &minv, int32_t &maxv) {
  // Convert 32-bit left-justified to signed 24-bit: shift right 8
  // Compute DC-removed RMS
  double sum = 0.0;
  double sum_x = 0.0;
  minv = INT32_MAX;
  maxv = INT32_MIN;
  for (size_t i = 0; i < samples; i++) {
    int32_t v32 = buf[i];
    int32_t v24 = v32 >> 8; // signed 24-bit value
    if (v24 < minv) minv = v24;
    if (v24 > maxv) maxv = v24;
    sum_x += (double)v24;
  }
  double mean = sum_x / (double)samples;
  for (size_t i = 0; i < samples; i++) {
    int32_t v24 = (buf[i] >> 8);
    double d = (double)v24 - mean;
    sum += d * d;
  }
  double mean_sq = sum / (double)samples;
  double rms = sqrt(mean_sq);
  // Normalize to 24-bit full scale (max ~ 2^23-1 = 8388607)
  double norm = rms / 8388607.0;
  return (float)norm;
}

void draw_bar(float value) {
  // value 0..1
  int barMax = 110;
  int w = (int)(constrain(value, 0.0, 1.0) * barMax);
  display.drawRect(8, 40, barMax, 12, SSD1306_WHITE);
  if (w > 0) display.fillRect(8, 40, w, 12, SSD1306_WHITE);
}

void calibrate_baseline() {
  unsigned long start = millis();
  unsigned long cnt = 0;
  double sum = 0.0;
  int32_t tmpBuf[BUF_SAMPLES];
  size_t bytes = 0;
  Serial.println("Kalibrasi baseline ambient...");
  while (millis() - start < BASELINE_MS) {
    i2s_read(I2S_NUM, (void*)tmpBuf, BUF_SAMPLES * sizeof(int32_t), &bytes, portMAX_DELAY);
    size_t samples = bytes / sizeof(int32_t);
    if (samples == 0) continue;
    int32_t minv, maxv;
    float n = calc_rms_from_buf(tmpBuf, samples, minv, maxv);
    sum += n;
    cnt++;
  }
  if (cnt > 0) baseline = (float)(sum / (double)cnt);
  else baseline = 0.0;
  Serial.print("Baseline calibrated = ");
  Serial.println(baseline, 6);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("INMP441 I2S test init...");
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED fail");
    while (1) delay(1000);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("INMP441 Test");
  display.display();

  setup_i2s_rx();
  delay(100);
  calibrate_baseline();
  level_smooth = baseline;
}

void loop() {
  static int32_t readBuf[BUF_SAMPLES];
  size_t bytes_read = 0;
  i2s_read(I2S_NUM, (void*)readBuf, BUF_SAMPLES * sizeof(int32_t), &bytes_read, portMAX_DELAY);
  size_t samples = bytes_read / sizeof(int32_t);
  if (samples == 0) return;

  int32_t minv, maxv;
  float norm = calc_rms_from_buf(readBuf, samples, minv, maxv);

  // subtract baseline, clamp
  float level = norm - baseline;
  if (level < 0) level = 0;
  // smoothing EMA
  level_smooth = ALPHA * level_smooth + (1.0 - ALPHA) * level;

  // Display
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("RMS(norm): ");
  display.println(norm, 6);
  display.print("Level: ");
  display.println(level_smooth, 6);
  display.print("Min/Max: ");
  display.print(minv);
  display.print(" / ");
  display.println(maxv);
  draw_bar(level_smooth * 3.0); // scale for visibility
  display.display();

  Serial.print("norm:"); Serial.print(norm,6);
  Serial.print(" level:"); Serial.print(level_smooth,6);
  Serial.print(" min:"); Serial.print(minv);
  Serial.print(" max:"); Serial.println(maxv);

  delay(40);
}
