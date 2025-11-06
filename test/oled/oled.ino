#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Konfigurasi OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1  // Tidak digunakan
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA, SCL

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("OLED tidak terdeteksi! Periksa kabel SDA/SCL & alamat I2C."));
    while (true);
  }

  Serial.println("OLED terdeteksi!");
  display.clearDisplay();

  // Tes tampilan teks
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("Halo Ficram!");
  display.setCursor(10, 25);
  display.println("OLED 0.92\" Siap :)");
  display.display();
}

void loop() {
  // Animasi sederhana (garis bergerak)
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    display.clearDisplay();
    display.drawLine(0, 63, i, 0, SSD1306_WHITE);
    display.display();
    delay(10);
  }
}
