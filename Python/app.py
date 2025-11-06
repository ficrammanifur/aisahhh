# app.py - Script Python untuk Laptop: Wake Word -> STT -> Gemini API -> TTS -> MQTT ke ESP32 Wokwi
# Fixes untuk Linux: Env vars suppress ALSA/Jack, device selection, fallback STT, manual mode
# Fitur: Sama seperti sebelumnya, plus wake word "aisah" continuous detection

import os
import threading
import time
import re
from dotenv import load_dotenv
import paho.mqtt.client as mqtt
import logging
from voice_processor import VoiceProcessor  # Import updated class
import google.generativeai as genai

# Load .env file
load_dotenv()

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

# Suppress ALSA/Jack globally
os.environ['PA_ALSA_PLUGHW'] = '1'

# Konfigurasi
GEMINI_API_KEY = os.getenv('GEMINI_API_KEY')
if not GEMINI_API_KEY:
    raise ValueError("GEMINI_API_KEY tidak ditemukan di .env file!")
MQTT_BROKER = 'test.mosquitto.org'
MQTT_PORT = 1883
MQTT_PUB_TOPIC = 'oled/chat'
MQTT_SUB_TOPIC = 'chat/confirm'
CLIENT_ID = 'Laptop_Chat_Client'
WAKE_WORD = 'aisah'
LANGUAGE = 'id-ID'

# Global untuk Gemini model
gemini_model = None

# Fungsi strip Markdown (untuk respons alami)
def strip_markdown(text):
    if not text:
        return ""
    text = re.sub(r'\*\*(.*?)\*\*', r'\1', text)
    text = re.sub(r'\*(.*?)\*', r'\1', text)
    text = re.sub(r'^[\*\-]\s+', '', text, flags=re.MULTILINE)
    text = re.sub(r'\n\s*\n', '\n', text)
    return text.strip()

# Ensure Gemini ready with fallbacks (optimized for Oct 2025)
def ensure_gemini_ready():
    global gemini_model
    if gemini_model is not None:
        return True
    try:
        model_names = [
            'gemini-2.5-flash', 'gemini-2.5-pro', 'gemini-1.5-flash', 'gemini-1.5-pro',
            'gemini-2.0-flash', 'gemini-pro',
            'models/gemini-2.5-flash', 'models/gemini-2.5-pro', 'models/gemini-1.5-flash', 'models/gemini-1.5-pro'
        ]
        available_models = []
        try:
            genai.configure(api_key=GEMINI_API_KEY)
            for model in genai.list_models():
                if 'gemini' in model.name.lower():
                    available_models.append(model.name)
                    logger.info(f"[AI] Available model: {model.name}")
        except Exception as e:
            logger.warning(f"[AI] Could not list models: {e}")
        
        for model_name in model_names:
            try:
                if available_models and model_name not in available_models:
                    full_model_name = f"models/{model_name}" if not model_name.startswith('models/') else model_name
                    if full_model_name not in available_models:
                        continue
                    model_name = full_model_name
                
                safety_settings = [
                    {"category": genai.types.HarmCategory.HARM_CATEGORY_HARASSMENT, "threshold": genai.types.HarmBlockThreshold.BLOCK_NONE},
                    {"category": genai.types.HarmCategory.HARM_CATEGORY_HATE_SPEECH, "threshold": genai.types.HarmBlockThreshold.BLOCK_NONE},
                    {"category": genai.types.HarmCategory.HARM_CATEGORY_SEXUALLY_EXPLICIT, "threshold": genai.types.HarmBlockThreshold.BLOCK_NONE},
                    {"category": genai.types.HarmCategory.HARM_CATEGORY_DANGEROUS_CONTENT, "threshold": genai.types.HarmBlockThreshold.BLOCK_NONE},
                ]
                
                test_model = genai.GenerativeModel(
                    model_name,
                    system_instruction=(
                        "Kamu adalah asisten virtual ramah berbahasa Indonesia. "
                        "Berikan jawaban ringkas, jelas, dan membantu. "
                        "Gunakan bahasa alami dan friendly."
                    ),
                    safety_settings=safety_settings
                )
                
                test_response = test_model.generate_content("Halo")
                gemini_model = test_model
                logger.info(f"[AI] Successfully initialized Gemini AI with model: {model_name}")
                return True
                
            except Exception as model_error:
                logger.warning(f"[AI] Failed to initialize model {model_name}: {model_error}")
                continue
        
        logger.error("[AI] No working Gemini model found")
        return False
        
    except Exception as e:
        logger.error(f"[AI] Error configuring Gemini API: {e}")
        return False

# Setup VoiceProcessor (auto-detect device)
vp = VoiceProcessor(wake_word=WAKE_WORD, language=LANGUAGE)  # Akan list mics

# Setup MQTT dengan callbacks
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("MQTT connected to broker")
        client.subscribe(MQTT_SUB_TOPIC)
        print(f"Subscribed to {MQTT_SUB_TOPIC} for confirmations")
    else:
        print(f"Failed to connect, return code {rc}")

def on_message(client, userdata, msg):
    print(f"Received confirmation from ESP32: {msg.payload.decode()}")

mqtt_client = mqtt.Client(CLIENT_ID)
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
mqtt_client.loop_start()

def generate_response(prompt):
    if not ensure_gemini_ready():
        logger.error("Gemini tidak siap!")
        return "Maaf, ada kesalahan dalam menghasilkan respons."
    try:
        response = gemini_model.generate_content(prompt)
        clean_response = strip_markdown(response.text)
        return clean_response
    except Exception as e:
        logger.error(f"Error Gemini: {e}")
        return "Maaf, ada kesalahan dalam menghasilkan respons."

def send_to_mqtt(message):
    mqtt_client.publish(MQTT_PUB_TOPIC, message)
    print(f"Pesan dikirim ke ESP32: {message}")

def play_tts(text):
    """Generate dan play TTS menggunakan VoiceProcessor."""
    audio_file = vp.text_to_speech(text, output_file='response.mp3')
    if audio_file:
        vp.play_audio(audio_file)
        # Clean up file setelah play
        time.sleep(0.1)
        if os.path.exists(audio_file):
            os.remove(audio_file)
            logger.info("TTS file cleaned up.")

def process_conversation():
    """Proses full conversation setelah wake word detected."""
    print("Wake word detected! Listening for full command...")
    # Rekam full audio setelah wake word
    audio = vp.start_listening()
    if audio:
        # STT full
        full_text = vp.speech_to_text(audio)
        if full_text:
            print(f"Full command: {full_text}")
            # Generate respons Gemini
            prompt = f"Pertanyaan pengguna: {full_text}\nBerikan jawaban singkat dan ramah dalam bahasa Indonesia."
            response = generate_response(prompt)
            if response:
                # Play TTS
                threading.Thread(target=play_tts, args=(response,), daemon=True).start()
                # Kirim ke ESP32
                send_to_mqtt(response)
            else:
                error_msg = "Maaf, tidak bisa generate respons."
                play_tts(error_msg)
                send_to_mqtt(error_msg)
        else:
            error_msg = "Maaf, suara tidak terdeteksi."
            play_tts(error_msg)
            send_to_mqtt(error_msg)
    vp.stop_listening()

def continuous_listen():
    """Thread untuk continuous wake word detection (dengan retry)."""
    while True:
        try:
            # Listen in chunks untuk detect wake word
            audio_chunk = vp.start_listening()
            if audio_chunk:
                if vp.detect_wake_word(audio_chunk):
                    # Trigger full process
                    threading.Thread(target=process_conversation, daemon=True).start()
            time.sleep(0.5)  # Pause antar listen
        except Exception as e:
            logger.error(f"Error in continuous listen: {e}")
            time.sleep(1)

# Main loop: Start continuous listening thread + manual mode option
if __name__ == "__main__":
    print("=== Aisah AI: Wake Word 'Aisah' + Chat via Mic ke ESP32 Wokwi ===")
    print(f"Gemini API Key loaded: {'Yes' if GEMINI_API_KEY else 'No'}")
    print("Mulai listening... Bicara 'Aisah' + pertanyaan Anda.")
    
    use_manual = input("Test manual input dulu? (y/n, kalau mic bermasalah): ").lower() == 'y'
    
    try:
        if use_manual:
            print("Mode manual: Masukkan teks langsung.")
            while True:
                text = input("Command: ")
                if WAKE_WORD in text.lower():
                    full_text = text  # Simulate STT
                    prompt = f"Pertanyaan pengguna: {full_text}\nBerikan jawaban singkat dan ramah dalam bahasa Indonesia."
                    response = generate_response(prompt)
                    if response:
                        play_tts(response)
                        send_to_mqtt(response)
                time.sleep(0.1)
        else:
            # Start listening thread
            listen_thread = threading.Thread(target=continuous_listen, daemon=True)
            listen_thread.start()
            
            # Keep main thread alive
            while True:
                time.sleep(1)
    except KeyboardInterrupt:
        if use_manual:
            print("\nManual mode dihentikan.")
        else:
            print("\nProgram dihentikan.")
    finally:
        vp.stop_listening()
        mqtt_client.loop_stop()
        mqtt_client.disconnect()
