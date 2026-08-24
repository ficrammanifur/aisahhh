# 🎤 MicroVoice - Voice-Activated AI Assistant for IoT

**MicroVoice** adalah sistem asisten virtual berbasis suara yang mengintegrasikan speech recognition, AI generatif (Gemini), text-to-speech, dan IoT control melalui MQTT. Sempurna untuk smart home, robotic projects, dan embedded systems dengan support ESP32 microcontroller.

> **Konsep**: Membawa AI voice assistant dari cloud ke micro-scale IoT devices. STT → AI Logic → TTS → MQTT → Microcontroller Action.

---

## 🎯 Use Cases

✅ **Smart Home Control** - Voice command untuk smart lights, fans, air conditioner  
✅ **IoT Dashboard** - Display AI response di OLED screen via ESP32  
✅ **Robotic Assistant** - Voice-controlled robot dengan intelligent responses  
✅ **Educational Project** - Learn AI, IoT, speech processing integration  
✅ **Prototype Development** - Quick setup untuk voice-enabled products  

---

## ✨ Fitur Utama

- **🎙️ Wake Word Detection** - Continuous listening untuk "Aisah" trigger
- **🔊 Speech-to-Text (STT)** - Konversi ucapan ke teks (Google STT + offline Sphinx fallback)
- **🤖 AI-Powered Responses** - Google Gemini 2.5 Flash untuk intelligent replies
- **🔄 Text-to-Speech (TTS)** - Convert AI response ke audio (gTTS)
- **📡 MQTT IoT Integration** - Real-time communication dengan ESP32/IoT devices
- **🌐 Bahasa Indonesia Native** - Full support untuk speech recognition & TTS
- **⚙️ Auto Microphone Detection** - Automatic device selection & fallback
- **🛡️ Robust Error Handling** - Graceful degradation dengan fallback mechanisms
- **🔐 Environment-based Config** - Secure API key management via .env

---

## 📂 Struktur Project

```
MicroVoice/
│
├── Python/                           # Laptop/Desktop Application
│   ├── app.py                       # Main application (Wake word + Chat engine)
│   ├── voice_processor.py           # VoiceProcessor class (STT, TTS, audio I/O)
│   ├── requirements.txt             # Python dependencies
│   ├── env-example                  # Template untuk environment variables
│   └── README_PYTHON.md             # Setup guide untuk Python
│
├── ESP32/                           # Microcontroller Firmware
│   ├── MicroVoice.ino              # Main Arduino sketch (MQTT subscriber + display)
│   ├── config.h                    # WiFi & MQTT configuration
│   ├── lib/                        # Libraries (MQTT client, OLED driver, dll)
│   └── test/                       # Test sketches untuk Wokwi simulator
│
├── docs/                            # Documentation
│   ├── ARCHITECTURE.md             # System architecture & flow
│   ├── MQTT_PROTOCOL.md            # MQTT topic specifications
│   └── TROUBLESHOOTING.md          # Common issues & solutions
│
├── LICENSE                          # MIT License
└── README.md                        # This file (main documentation)
```

---

## 🚀 Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    MicroVoice System Flow                    │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  [Microphone] → [Wake Word Detection] → [STT]              │
│       ↓              ↓                     ↓                 │
│   Audio Input   "Aisah detected"      Transcribe           │
│                                          Text               │
│                                           ↓                 │
│                  ┌────────────────────────────────┐         │
│                  │  Google Gemini API             │         │
│                  │  (Generate Response)           │         │
│                  └────────────────────────────────┘         │
│                           ↓                                  │
│                      AI Response                             │
│                           ↓                                  │
│                  ┌──────────────────┐                        │
│                  │ [TTS Generator]  │                        │
│                  └──────────────────┘                        │
│                      Audio File (.mp3)                       │
│                           ↓                                  │
│      ┌────────────────────┴────────────────────┐            │
│      ↓                                          ↓            │
│  [Speaker Output]                    [MQTT Publish]         │
│                                    topic: oled/chat         │
│                                           ↓                  │
│                            ┌──────────────────────────┐     │
│                            │    ESP32 Microcontroller │     │
│                            │  (MQTT Subscriber)       │     │
│                            ├──────────────────────────┤     │
│                            │ • OLED Display           │     │
│                            │ • Speaker/Buzzer         │     │
│                            │ • LED Indicators         │     │
│                            │ • Relay Control (IoT)    │     │
│                            └──────────────────────────┘     │
│                                           ↓                  │
│                          [Smart Home/Robot Action]          │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

---

## 📋 Requirements

### Hardware
- **Laptop/Desktop** dengan microphone (Windows, macOS, Linux)
- **ESP32** microcontroller (atau compatible board)
- **OLED Display** (optional, untuk display response)
- **Speakers** (untuk audio output)
- **WiFi connectivity** (untuk MQTT & API calls)

### Software
- **Python 3.8+**
- **Arduino IDE** (untuk compile ESP32 firmware)
- **FFmpeg** (untuk audio processing)
- **Google Gemini API Key** (free from [Google AI Studio](https://aistudio.google.com/))

### Internet Services
- **Google Speech-to-Text API** (via SpeechRecognition library)
- **Google Gemini API** (for AI responses)
- **Google Text-to-Speech API** (via gTTS)
- **MQTT Broker** (test.mosquitto.org atau self-hosted)

---

## 🔧 Installation Guide

### Step 1: Clone Repository

```bash
git clone https://github.com/ficrammanifur/MicroVoice.git
cd MicroVoice
```

### Step 2: Setup Python Application

#### Install System Dependencies

**Linux/Ubuntu:**
```bash
sudo apt-get update
sudo apt-get install python3-dev python3-pip portaudio19-dev ffmpeg
```

**macOS:**
```bash
brew install python3 portaudio ffmpeg
```

**Windows:**
- Install Python 3.8+ from [python.org](https://www.python.org/)
- Install FFmpeg from [ffmpeg.org](https://ffmpeg.org/download.html)
- Install PortAudio development files

#### Install Python Dependencies

```bash
cd Python
pip install -r requirements.txt
```

**Troubleshooting PyAudio on Linux:**
```bash
pip install --upgrade setuptools wheel
pip install pyaudio --no-cache-dir
```

#### Configure Environment

```bash
cp env-example .env
# Edit .env dengan text editor
nano .env
```

Add your Gemini API Key:
```env
GEMINI_API_KEY=your_api_key_here
```

### Step 3: Setup ESP32 Firmware

#### Install Arduino IDE & Boards

1. Download [Arduino IDE](https://www.arduino.cc/en/software)
2. Install ESP32 board support:
   - Go to `File → Preferences`
   - Add to "Additional Boards Manager URLs": `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Go to `Tools → Board → Boards Manager`
   - Search "esp32" and install

#### Configure WiFi & MQTT

Edit `ESP32/config.h`:
```cpp
#define WIFI_SSID     "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"
#define MQTT_BROKER   "test.mosquitto.org"
#define MQTT_PORT     1883
#define MQTT_CLIENT   "ESP32_MicroVoice"
```

#### Upload to ESP32

1. Connect ESP32 to computer via USB
2. Open `ESP32/MicroVoice.ino` in Arduino IDE
3. Select board: `Tools → Board → ESP32 Dev Module`
4. Select port: `Tools → Port → COM3` (atau port yang terdeteksi)
5. Click `Upload` button

---

## ▶️ Quick Start

### Run Python Application

```bash
cd Python
python app.py
```

**Output:**
```
=== MicroVoice: Wake Word Detection + Gemini Chat ===
Gemini API Key loaded: Yes
=== Available Microphones ===
Device 0: Built-in Microphone (Channels: 2, Rate: 44100Hz)
Auto-selected default mic: Device 0
==============================
Started listening...
Waiting for wake word 'Aisah'...
```

**Usage:**
1. Speak naturally: **"Aisah, berapa hasil 2+2?"**
2. System akan:
   - Detect "Aisah" wake word
   - Transcribe pertanyaan
   - Generate response via Gemini
   - Play response audio
   - Send to ESP32 via MQTT

### Monitor MQTT Messages

```bash
# Install MQTT client (optional)
sudo apt-get install mosquitto-clients

# Subscribe to response topic
mosquitto_sub -h test.mosquitto.org -t oled/chat
```

---

## 🎮 Operating Modes

### Mode 1: Voice Input (Normal)

```
User: "Aisah, apa saja bahaya merokok?"
↓
System detects wake word → records audio → transcribes
↓
Sends to Gemini API
↓
Gemini: "Bahaya merokok meliputi: 1) Kanker paru... 2) Jantung... 3)..."
↓
Converts to speech → plays audio → publishes to MQTT
↓
ESP32 displays on OLED screen
```

### Mode 2: Manual/Testing

```
python app.py
Test manual input dulu? (y/n): y
Command: Aisah, berapa 5 x 5?
```

### Mode 3: Continuous Mode

Program runs infinitely, always listening for wake word. Stop with `Ctrl+C`.

---

## 🔊 Audio Pipeline

```
Input Microphone
        ↓
Noise Reduction (ambient noise subtraction)
        ↓
Speech Recognition (Google STT API)
        ↓
Text Processing (cleanup, normalization)
        ↓
Wake Word Detection (keyword match)
        ↓
Gemini API Call (with system prompt)
        ↓
Response Text
        ↓
Text-to-Speech (gTTS - Google TTS)
        ↓
MP3 Audio File
        ↓
Playback (pydub) + MQTT Publish
```

---

## 📡 MQTT Protocol

### Topics & Payloads

#### Publish (Laptop → ESP32)
**Topic:** `oled/chat`
```json
{
  "message": "Merokok dapat menyebabkan kanker paru-paru...",
  "timestamp": "2026-08-24T10:30:45Z",
  "source": "Gemini API"
}
```

#### Subscribe (ESP32 → Laptop)
**Topic:** `chat/confirm`
```
Payload: "OK" atau "Message received and displayed"
```

---

## ⚙️ Configuration

### app.py Settings

```python
WAKE_WORD = 'aisah'                 # Wake word untuk deteksi
LANGUAGE = 'id-ID'                  # Language code (id-ID, en-US, dll)
MQTT_BROKER = 'test.mosquitto.org'  # MQTT server address
MQTT_PORT = 1883                    # MQTT port
MQTT_PUB_TOPIC = 'oled/chat'       # Topic untuk publish response
MQTT_SUB_TOPIC = 'chat/confirm'     # Topic untuk subscribe confirmation
```

### ESP32 Settings (config.h)

```cpp
#define WIFI_SSID "Your_WiFi"           // WiFi network name
#define WIFI_PASSWORD "Your_Password"   // WiFi password
#define MQTT_BROKER "test.mosquitto.org" // MQTT server
#define MQTT_PORT 1883                  // MQTT port
#define OLED_ADDRESS 0x3C               // I2C address untuk OLED
```

---

## 🐛 Troubleshooting

### Issue: Microphone Not Detected

**Solution:**
```bash
# List all audio devices
python -c "import pyaudio; p = pyaudio.PyAudio(); [print(f'{i}: {p.get_device_info_by_index(i)[\"name\"]}') for i in range(p.get_device_count())]"
```

Manual device selection di `app.py`:
```python
vp = VoiceProcessor(wake_word=WAKE_WORD, language=LANGUAGE, device_index=2)
```

### Issue: "ALSA Warnings" (Linux)

Already handled automatically. If still showing:
```bash
export PA_ALSA_PLUGHW=1
python app.py
```

### Issue: Google STT Not Working

1. Check internet connection
2. Verify Gemini API key is valid
3. Test with manual mode first
4. Try PocketSphinx offline fallback (installed via requirements.txt)

### Issue: ESP32 Not Receiving MQTT

**Checklist:**
- [ ] ESP32 connected to WiFi (check serial monitor)
- [ ] MQTT broker is reachable (test with: `mosquitto_pub -h test.mosquitto.org -t test/topic -m "hello"`)
- [ ] Topic names match exactly (case-sensitive!)
- [ ] Firewall not blocking MQTT port 1883

### Issue: Permission Denied (PyAudio on Linux)

```bash
sudo usermod -a -G audio $USER
# Log out and log back in
```

---

## 📚 Dependencies

### Python Libraries

| Package | Version | Purpose |
|---------|---------|---------|
| **SpeechRecognition** | 3.9.0 | Speech-to-text recognition |
| **google-generativeai** | 0.8.5 | Gemini AI API |
| **paho-mqtt** | 1.6.1 | MQTT client |
| **gtts** | 2.5.4 | Google Text-to-Speech |
| **pyaudio** | 0.2.13 | Audio I/O interface |
| **pydub** | 0.25.1 | Audio processing |
| **python-dotenv** | 1.0.1 | Environment variables |
| **numpy** | 1.26.4 | Numerical computing |

### Arduino Libraries (ESP32)

- **PubSubClient** - MQTT library
- **Adafruit_SSD1306** - OLED display driver
- **Adafruit_GFX** - Graphics library
- **WiFi** - Built-in ESP32 WiFi

---

## 🔐 Security Best Practices

1. **Never commit `.env` file** - Already in `.gitignore`
2. **Rotate API keys periodically** - Update GEMINI_API_KEY
3. **Use HTTPS/TLS for MQTT** in production
4. **Change default WiFi credentials** on ESP32
5. **Use strong MQTT broker passwords** (not test.mosquitto.org in production)
6. **Restrict MQTT topics** with ACL rules

---

## 📖 Documentation

- **[ARCHITECTURE.md](docs/ARCHITECTURE.md)** - Detailed system architecture
- **[MQTT_PROTOCOL.md](docs/MQTT_PROTOCOL.md)** - MQTT message specifications
- **[TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md)** - Common issues & solutions
- **[README_PYTHON.md](Python/README_PYTHON.md)** - Python setup guide
- **[README_ESP32.md](ESP32/README_ESP32.md)** - ESP32 firmware guide

---

## 🚀 Roadmap

- [ ] Multi-language support (en, ja, ko, zh)
- [ ] Custom wake word training
- [ ] Local LLM integration (Ollama, LLaMA)
- [ ] Voice cloning for TTS
- [ ] Multi-turn conversation memory
- [ ] Web dashboard for monitoring
- [ ] Docker containerization
- [ ] Mobile app for control
- [ ] Voice command profiling
- [ ] CI/CD pipeline setup

---

## 🎓 Educational Value

This project teaches:
- **Speech Processing** - How voice recognition works
- **Machine Learning** - Using pre-trained AI models
- **IoT/MQTT** - Device-to-device communication
- **Microcontroller Programming** - ESP32 firmware development
- **API Integration** - Google Cloud APIs
- **Audio Processing** - Digital signal processing basics
- **Full-Stack Development** - From hardware to cloud services

---

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Commit changes: `git commit -m 'Add amazing feature'`
4. Push to branch: `git push origin feature/amazing-feature`
5. Open a Pull Request

### Areas for Contribution
- Bug fixes and performance improvements
- Documentation improvements
- ESP32 firmware enhancements
- Language support additions
- Test coverage
- Example projects

---

## 📝 License

MIT License - Free for personal and commercial use. See [LICENSE](LICENSE) file for details.

---

## 💬 Support & Community

- **Issues**: [GitHub Issues](https://github.com/ficrammanifur/MicroVoice/issues)
- **Discussions**: [GitHub Discussions](https://github.com/ficrammanifur/MicroVoice/discussions)
- **Email**: [Email support]
- **Documentation**: Check [docs/](docs/) folder

---

## 🙏 Acknowledgments

- **Google Cloud** - APIs untuk STT, Gemini, TTS
- **Espressif** - ESP32 microcontroller & SDK
- **Open Source Community** - Libraries dan frameworks

---

## 📊 Project Stats

- **Language Composition**: Python 32.9%, C++ 67.1% (firmware)
- **First Commit**: November 6, 2025
- **Last Update**: August 24, 2026
- **License**: MIT
- **Status**: Active Development ✅

---

**Built with ❤️ for makers, educators, and IoT enthusiasts**

*"Bringing AI Voice Intelligence to Micro-Scale Systems"*

---

**Quick Links:**
- 🌐 [GitHub Repository](https://github.com/ficrammanifur/MicroVoice)
- 📖 [Documentation](https://github.com/ficrammanifur/MicroVoice/tree/main/docs)
- 🐛 [Report Issues](https://github.com/ficrammanifur/MicroVoice/issues)
- ⭐ Star this repo if you find it useful!
