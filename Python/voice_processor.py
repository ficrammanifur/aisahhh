import speech_recognition as sr
from pydub import AudioSegment
from pydub.playback import play
import numpy as np
import threading
import logging
from datetime import datetime
import os
from gtts import gTTS
import pyaudio  # Untuk list devices

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Suppress ALSA/Jack warnings via env (set di script atau global)
os.environ['PA_ALSA_PLUGHW'] = '1'  # Force ALSA tanpa Jack

class VoiceProcessor:
    """Handle speech-to-text, text-to-speech, and wake word detection"""
    
    def __init__(self, wake_word='xanny', language='id-ID', device_index=None):
        self.recognizer = sr.Recognizer()
        self.device_index = device_index  # Manual device selection
        self.microphone = sr.Microphone(device_index=device_index)
        self.wake_word = wake_word.lower()
        self.language = language
        self.is_listening = False
        self.audio_buffer = []
        
        # List & print available mics
        self.list_available_mics()
    
    def list_available_mics(self):
        """List PyAudio input devices untuk debug."""
        p = pyaudio.PyAudio()
        print("\n=== Available Microphones ===")
        for i in range(p.get_device_count()):
            info = p.get_device_info_by_index(i)
            if info['maxInputChannels'] > 0:
                print(f"Device {i}: {info['name']} (Channels: {info['maxInputChannels']}, Rate: {info['defaultSampleRate']:.0f}Hz)")
                if self.device_index is None and 'default' in info['name'].lower():
                    self.device_index = i  # Auto-select default
                    self.microphone = sr.Microphone(device_index=self.device_index)
                    print(f"Auto-selected default mic: Device {i}")
        p.terminate()
        print("==============================")
    
    def detect_wake_word(self, audio_data):
        """Detect wake word in audio"""
        try:
            text = self.speech_to_text(audio_data)
            if text and self.wake_word in text.lower():
                logger.info(f"Wake word detected: {self.wake_word}")
                return True
            return False
        except Exception as e:
            logger.error(f"Error detecting wake word: {e}")
            return False
    
    def speech_to_text(self, audio_data):
        """Convert speech to text using Google Speech Recognition (fallback Sphinx)"""
        try:
            # If audio_data is a file path
            if isinstance(audio_data, str):
                with sr.AudioFile(audio_data) as source:
                    audio = self.recognizer.record(source)
            else:
                audio = audio_data
            
            # Primary: Google STT
            text = self.recognizer.recognize_google(audio, language=self.language)
            logger.info(f"Transcribed (Google): {text}")
            return text
        except sr.UnknownValueError:
            logger.warning("Google STT: Tidak bisa memahami audio")
        except sr.RequestError as e:
            logger.error(f"Google STT Error: {e}")
        
        # Fallback: PocketSphinx (offline, install pip install pocketsphinx)
        try:
            text = self.recognizer.recognize_sphinx(audio, language='en-US')  # Default en, adjust model for id-ID if needed
            logger.info(f"Transcribed (Sphinx fallback): {text}")
            return text
        except ImportError:
            logger.warning("PocketSphinx not installed for fallback. pip install pocketsphinx")
        except Exception as e:
            logger.error(f"Sphinx Error: {e}")
        
        return None
    
    def text_to_speech(self, text, output_file='response.mp3'):
        """Convert text to speech using gTTS"""
        try:
            tts = gTTS(text=text, lang=self.language.split('-')[0], slow=False)
            tts.save(output_file)
            logger.info(f"TTS saved to {output_file}")
            return output_file
        except Exception as e:
            logger.error(f"Error in text_to_speech: {e}")
            return None
    
    def play_audio(self, audio_file):
        """Play audio file"""
        try:
            sound = AudioSegment.from_mp3(audio_file)
            play(sound)
            logger.info(f"Playing audio: {audio_file}")
        except Exception as e:
            logger.error(f"Error playing audio: {e}")
    
    def start_listening(self):
        """Start listening for audio input (shorter timeout for wake word)"""
        self.is_listening = True
        logger.info("Started listening...")
        
        try:
            with self.microphone as source:
                self.recognizer.adjust_for_ambient_noise(source, duration=0.5)  # Faster adjust
                # Shorter timeout untuk continuous listen
                audio = self.recognizer.listen(source, timeout=2, phrase_time_limit=3)
                logger.info(f"Audio captured: {len(audio.get_raw_data())} bytes")
                return audio
        except sr.WaitTimeoutError:
            logger.warning("Listen timeout: No speech detected")
            return None
        except sr.RequestError as e:
            logger.error(f"Microphone error: {e}")
            return None
        except Exception as e:
            logger.error(f"Error listening: {e}")
            return None
        finally:
            self.is_listening = False
    
    def stop_listening(self):
        """Stop listening"""
        self.is_listening = False
        logger.info("Stopped listening")
    
    def process_audio_file(self, filepath):
        """Process an audio file with format conversion"""
        original_path = filepath
        converted_path = None
        try:
            logger.info(f"Processing audio file: {filepath}")
            # Convert to WAV using pydub if not already WAV
            if not filepath.lower().endswith('.wav'):
                logger.info("Converting audio to WAV...")
                audio = AudioSegment.from_file(filepath)
                converted_path = filepath.rsplit('.', 1)[0] + '.wav'
                audio.export(converted_path, format='wav')
                filepath = converted_path
                logger.info(f"Converted to: {converted_path}")
            
            with sr.AudioFile(filepath) as source:
                audio = self.recognizer.record(source)
            transcription = self.speech_to_text(audio)
            logger.info(f"Final transcription: {transcription}")
            return transcription
        except Exception as e:
            logger.error(f"Error processing audio file: {e}")
            return None
        finally:
            # Clean up converted file if created
            if converted_path and os.path.exists(converted_path) and converted_path != original_path:
                os.remove(converted_path)
                logger.info(f"Cleaned up converted file: {converted_path}")
