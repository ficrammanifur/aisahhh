#include <ESP32Servo.h>

Servo servo;  // Buat objek servo

const int servoPin = 19;

void setup() {
  Serial.begin(115200);
  Serial.println("Tes Servo SG90 Dimulai...");
  
  // Attach servo ke pin 19 dengan range 500-2400 microsecond (default untuk SG90)
  servo.attach(servoPin, 500, 2400);
  
  // Posisi awal (0 derajat)
  servo.write(0);
  Serial.println("Servo di posisi 0 derajat");
  delay(1000);
}

void loop() {
  // Gerak dari 0° ke 180°
  for (int pos = 0; pos <= 180; pos += 10) {
    servo.write(pos);
    Serial.print("Gerak ke ");
    Serial.print(pos);
    Serial.println(" derajat");
    delay(200);
  }

  // Kembali dari 180° ke 0°
  for (int pos = 180; pos >= 0; pos -= 10) {
    servo.write(pos);
    Serial.print("Kembali ke ");
    Serial.print(pos);
    Serial.println(" derajat");
    delay(200);
  }
}
