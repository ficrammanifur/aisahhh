# Panduan Wiring ESP32 DevKit dengan Komponen Audio dan Aktuator

## 🔌 Ringkasan Wiring (Rapi)
ESP32 DevKit biasa (pin numbers sesuai label pada board)

### INMP441 (I2S Microphone)
- VCC → 3.3V (ESP32 3V3)
- GND → GND
- L/R → GND (pilih channel left/mono)
- WS → GPIO35 (LRCLK / WS) — input-only, OK untuk WS
- SCK → GPIO33 (BCLK / SCK)
- SD → GPIO32 (DOUT / data out from mic)

### MAX98357A (I2S DAC / Amplifier)
- VIN → 5V (separate 5V supply disarankan)
- GND → GND (sambungkan ke GND ESP32)
- GAIN → GND (default gain)
- DIN → GPIO25 (I2S Data In)
- BCLK → GPIO27 (I2S Bit Clock)
- LRC → GPIO26 (I2S LRCLK / LRC)

### OLED 0.92" (SSD1306, I2C)
- VCC → 3.3V
- GND → GND
- SDA → GPIO21
- SCL → GPIO22

### Servo SG90
- Signal → GPIO19 (PWM)
- VCC → 5V (eksternal recommended)
- GND → GND (sama dengan ESP32 & 5V supply)

## 🗺️ Diagram Koneksi (ASCII — View Cepat)

```
         +------------------ ESP32 DevKit ------------------+
         |                                                |
 3.3V ---+----+-----------------------------------+  VCC(3.3V) |
            |                                   |           |
           INMP441                             OLED        |
        VCC -> 3.3V                      VCC -> 3.3V         |
        GND -> GND                       GND -> GND         |
        L/R -> GND                       SDA -> GPIO21      |
        WS  -> GPIO35 (LRCLK)            SCL -> GPIO22      |
        SCK -> GPIO33 (BCLK)            (I2C addr 0x3C)    |
        SD  -> GPIO32 (DATA OUT)                           |
                                                        GND |
 MAX98357A                                            (common)
  VIN -> 5V (ext PSU)                                   |
  GND -> GND -------------------------------------------+
  GAIN-> GND
  DIN -> GPIO25 (I2S DOUT)
  BCLK-> GPIO27 (I2S BCLK)
  LRC -> GPIO26 (I2S LRC)

 Servo SG90:
  +5V (ext PSU) ---+
                   |
  Signal -> GPIO19 |
  GND  -> GND -----+
```

## ✅ Praktik Terbaik & Catatan Penting

1. **Ground bersama**: Semua GND (ESP32, 5V PSU untuk speaker & servo, OLED, mic) HARUS disatukan.
2. **Supply servo & amp terpisah**: Gunakan adaptor 5V (2A atau lebih) untuk MAX98357A + servo. Jangan tarik servo dari USB ESP32 bila servo kuat bergerak.
3. **Kapasitor decoupling**: Pasang 470–1000µF antara 5V dan GND dekat board amp/servo untuk meredam spike arus. Tambahkan 0.1µF keramik juga jika ada.
4. **INMP441 = 3.3V only**. Jangan sambungkan VCC mic ke 5V.
5. **Pin input-only**: GPIO35 tidak bisa dipakai sebagai output — cocok untuk WS (input).
6. **I2S pin mapping**: Kita gunakan dua blok I2S pada satu I2S driver (atau mic on I2S0 RX, speaker on I2S1 TX) — pada sketsa nanti pastikan config sesuai.
7. **Jaga kabel pendek**: Untuk I2S (clock & data) gunakan kabel pendek untuk stabilitas sinyal.
8. **Jangan hubungkan SDA/SCL ke 5V** — I2C OLED pakai 3.3V.

## 🔀 Saran Pin Alternatif (Jika Konflik Muncul)

- Jika GPIO19 konflik atau servo jitter: Coba GPIO18 atau GPIO17 (pastikan pin bebas).
- Jika mau gunakan I2S RX + TX terpisah: Pakai I2S_NUM_0 untuk mic (GPIO33/35/32) dan I2S_NUM_1 untuk speaker (GPIO27/26/25) supaya tidak perlu re-map pin tiap read/write.

## 📋 Checklist Pasang Sebelum Tes Gabungan

- [ ] Semua kabel GND terhubung (ESP32 ↔ 5V PSU ↔ speaker ↔ servo ↔ OLED ↔ mic)
- [ ] 5V PSU cukup (≥2A) untuk speaker + servo
- [ ] INMP441 VCC ke 3.3V saja
- [ ] Kapasitor 470–1000µF di 5V dekat MAX98357A / servo supply
- [ ] Lepas servo saat pertama kali tes mic + speaker untuk menghindari reset
- [ ] Pastikan kabel data I2S (BCLK/WS/DIN/SD) pendek dan rapih
