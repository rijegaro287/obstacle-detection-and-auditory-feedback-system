import wave

import numpy as np
import soundfile as sf
import sounddevice as sd
from piper import PiperVoice

signals = [
    'Obstáculo al frente',
    'Obstáculo arriba',
    'Obstáculo abajo',
    'Obstáculo a la derecha',
    'Obstáculo a la izquierda',
    'Obstáculo arriba a la derecha',
    'Obstáculo arriba a la izquierda',
    'Obstáculo abajo a la derecha',
    'Obstáculo abajo a la izquierda',
]

voice = PiperVoice.load('./piper-model/es_MX-claude-high.onnx')

for signal in signals:
    with wave.open(f'./{signal}.wav', 'wb') as wav_file:
        voice.synthesize_wav(signal, wav_file)

signal_audio_data = []
max_length = 0
for signal in signals:
    audio_data, sample_rate = sf.read(f'./{signal}.wav', dtype='float64')
    signal_audio_data.append(audio_data)
    max_length = max(max_length, len(audio_data))

signal_audio_data_padded = np.zeros((len(signals), max_length), dtype=np.float64)
for i, audio_data in enumerate(signal_audio_data):
    signal_audio_data_padded[i, :len(audio_data)] = audio_data

np.save('./verbal_feedback_signals.npy', signal_audio_data_padded)

verbal_signals = np.load('./verbal_feedback_signals.npy')
for i, signal in enumerate(signals):
    sd.play(verbal_signals[i], samplerate=sample_rate)
    sd.wait()
