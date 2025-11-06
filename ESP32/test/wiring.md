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
                     ╔══════════════════════════════════════╗
                     ║             ESP32 DEVKIT V1          ║
                     ║              (Top View)              ║
╔════════════════════╩══════════════════════════════════════╩═════════════════════╗
║ GND [1]   o─────────────────────────────────────────────────────────o   GND [1] ║
║ 3V3 [2]   o──┬───────────────────────────────────────────────────┬──o   EN [2]  ║
║              │                                                   │              ║
║              │               USB Connector Area                  │              ║
║              │                                                   │              ║
║              └───────── OLED VCC, INMP441 VCC (3.3V) ────────────┘              ║
║                                                                                 ║
║ GPIO36 [VP] o                    │                    o GPIO23 [3]              ║
║ GPIO39 [VN] o                    │                    o GPIO22 ←── OLED SCL     ║
║    GPIO34   o                    │                    o GPIO21 ←── OLED SDA     ║
║    GPIO35   o──┬──→ INMP441 WS   │                    o GPIO19 ←── Servo Signal ║
║    GPIO32   o──┴──→ INMP441 SD   │                    o GPIO18                  ║
║    GPIO33   o────→ INMP441 SCK   │                    o GPIO5                   ║
║    GPIO25   o────→ MAX98357A DIN │                    o GPIO17                  ║
║    GPIO26   o────→ MAX98357A LRC │                    o GPIO16                  ║
║    GPIO27   o────→ MAX98357A BCLK│                    o GPIO4                   ║
║    GPIO14   o                    │                    o GPIO0                   ║
║    GPIO12   o                    │                    o GPIO2                   ║
║    GPIO13   o                    │                    o GPIO15                  ║
║ GND [15]    o────────────────────────────────────────────────────────o          ║
║ VIN [16] (5V) o──→ MAX98357A VIN, Servo +5V (ext PSU)                           ║
╚═════════════════════════════════════════════════════════════════════════════════╝

**Catatan Pinout Lengkap ESP32 DevKit V1:**
- **Sisi Kiri (16 pin, dari atas ke bawah):** GND [1], 3V3 [2], GPIO36 (VP) [3], GPIO39 (VN) [4], GPIO34 [5], GPIO35 [6], GPIO32 [7], GPIO33 [8], GPIO25 [9], GPIO26 [10], GPIO27 [11], GPIO14 [12], GPIO12 [13], GPIO13 [14], GND [15], VIN/5V [16].
- **Sisi Kanan (14 pin, dari atas ke bawah):** GND [1], EN/Reset [2], GPIO23 [3], GPIO22 [4], GPIO21 [5], GPIO19 [6], GPIO18 [7], GPIO5 [8], GPIO17 [9], GPIO16 [10], GPIO4 [11], GPIO0 [12], GPIO2 [13], GPIO15 [14]. (Tidak ada pin 15-16 di sisi kanan.)
- **Koneksi Tambahan:** Semua GND disatukan (common ground). 3V3 untuk komponen 3.3V (OLED, INMP441). VIN/5V eksternal untuk MAX98357A & Servo (disarankan ≥2A).
- **Pin Tidak Digunakan:** GPIO34, GPIO36, GPIO39 (input-only, sensitif ADC), GPIO12, GPIO13, GPIO14, GPIO0, GPIO2, GPIO4, GPIO5, GPIO16, GPIO17, GPIO18, GPIO23 (bebas untuk ekspansi, tapi hindari konflik boot seperti GPIO0/GPIO2).
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
