/* speaker_test.ino
   Play a sine tone via I2S to MAX98357A
   Pins: DIN=25 (data), BCLK=27, LRC=26
*/

#include <Arduino.h>
#include <driver/i2s.h>

#define I2S_BCLK 27
#define I2S_LRC  26
#define I2S_DOUT 25

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Test MAX98357A: safe version");

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = 0,
    .dma_buf_count = 6,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("I2S install failed: %d\n", err);
    while (true) delay(100);
  }

  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, 16000, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);
}

void loop() {
  // Output tone 440 Hz
  static float phase = 0;
  float freq = 440;
  int16_t sample[2];

  for (int i = 0; i < 160; i++) { // 10ms worth
    float val = sin(phase) * 3000;  // kecilin amplitudo dulu
    sample[0] = sample[1] = (int16_t)val;
    size_t bytes_written;
    i2s_write(I2S_NUM_0, (const char*)sample, sizeof(sample), &bytes_written, portMAX_DELAY);
    phase += 2 * PI * freq / 16000.0;
    if (phase >= 2 * PI) phase -= 2 * PI;
  }

  delay(100); // kecilkan beban CPU
}
