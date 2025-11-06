#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Serial.println("\n🌟 Halo! Mari kita jelajahi dunia I2C bareng yuk! 🔍");
  Wire.begin(21, 22); // SDA, SCL (untuk ESP32)
  delay(1000); // Kasih waktu buat napas dulu 😌
}

void loop() {
  Serial.println("\n🚀 Mulai scanning perangkat I2C... (dari 0x01 sampai 0x7E)");
  Serial.println("⏳ Sabar ya, lagi nyari teman-temanmu...");

  byte error, address;
  int nDevices = 0;
  unsigned long startTime = millis();
  
  for (address = 1; address < 127; address++) {
    // Progress indicator sederhana
    if (address % 20 == 0) {
      Serial.print("."); // Titik-titik buat nunjukin progress
      Serial.flush();
    }
    
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("\n🎉 Yeay! Ketemu perangkat di alamat 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" ! Siapa dia ya? Coba cek datasheet-nya nanti~ 📖");
      nDevices++;
    } else if (error == 4) {
      Serial.print("\n🤔 Hmm, ada yang aneh di alamat 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("... Mungkin cuma lewat aja kok. 😏");
    }
  }
  
  Serial.println("\n"); // Spasi buat rapi
  
  // Box hasil scan pakai ASCII
  Serial.println("┌─────────────────────────────────────────────┐");
  if (nDevices == 0) {
    Serial.println("│ 😔 Ups, belum ada teman I2C yang ketemu nih. │");
    Serial.println("│                                             │");
    Serial.println("│ 💡 Tips cepat: Cek kabel SDA/SCL, VCC/GND,  │");
    Serial.println("│     dan pastiin pull-up resistor ada ya!    │");
    Serial.println("└─────────────────────────────────────────────┘");
  } else {
    Serial.print("│ 🎊 Total: ");
    Serial.print(nDevices);
    Serial.print(" perangkat ditemukan! Mantap! 🏆 ");
    for (int i = 0; i < (50 - (10 + String(nDevices).length())); i++) Serial.print(" ");
    Serial.println("│");
    Serial.println("│ Kamu bisa mulai mainin mereka sekarang! 🚀 │");
    Serial.println("└─────────────────────────────────────────────┘");
  }
  
  Serial.println("\n⏰ Scanning selesai dalam " + String(millis() - startTime) + " ms. Mau scan lagi? Tunggu 30 detik...");
  delay(30000); // Scan ulang setiap 30 detik
}
