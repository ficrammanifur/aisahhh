#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// --- OLED CONFIG ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- SERVO CONFIG ---
Servo servo;
const int servoPin = 19;

// --- SETUP ---
void setup() {
  Serial.begin(115200);
  Serial.println("Mulai Tes Servo + OLED...");

  // Inisialisasi OLED
  Wire.begin(21, 22); // SDA, SCL
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("❌ OLED tidak terdeteksi!"));
    while (true);
  }
  Serial.println("✅ OLED terdeteksi!");

  // Inisialisasi servo
  servo.attach(servoPin, 500, 2400);
  servo.write(0);
  Serial.println("✅ Servo siap di pin 19");

  // Tampilan awal di OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("Tes Servo + OLED");
  display.setCursor(20, 30);
  display.println("Mulai...");
  display.display();
  delay(1500);
}

// --- LOOP ---
void loop() {
  // Gerak 0° → 180°
  for (int pos = 0; pos <= 180; pos += 10) {
    servo.write(pos);
    updateOLED(pos);
    delay(200);
  }

  // Gerak balik 180° → 0°
  for (int pos = 180; pos >= 0; pos -= 10) {
    servo.write(pos);
    updateOLED(pos);
    delay(200);
  }
}

// --- FUNGSI UPDATE OLED ---
void updateOLED(int angle) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 10);
  display.println("Servo SG90 @ GPIO19");
  display.setCursor(5, 30);
  display.print("Sudut: ");
  display.print(angle);
  display.println(" derajat");

  // Visual indikator posisi
  int barLen = map(angle, 0, 180, 0, 120);
  display.drawRect(4, 50, 120, 10, SSD1306_WHITE);
  display.fillRect(4, 50, barLen, 10, SSD1306_WHITE);
  display.display();

  Serial.print("Servo ke ");
  Serial.print(angle);
  Serial.println(" derajat");
}
