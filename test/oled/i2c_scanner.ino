#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Serial.println("\n🔍 Memulai I2C Scanner...");
  Wire.begin(21, 22); // SDA, SCL

  byte error, address;
  int nDevices = 0;

  Serial.println("Mencari perangkat I2C...\n");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("✅ Ditemukan perangkat I2C di alamat 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    } else if (error == 4) {
      Serial.print("⚠️ Kesalahan tak diketahui di alamat 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0) {
    Serial.println("❌ Tidak ada perangkat I2C ditemukan.\nPeriksa kabel SDA/SCL dan power (VCC, GND).");
  } else {
    Serial.println("\n✅ Pemindaian selesai.");
  }
}

void loop() {
  // Tidak ada yang dilakukan di loop
}
