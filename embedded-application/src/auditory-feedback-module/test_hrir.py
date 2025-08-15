import random
import numpy as np
import soundfile as sf
from scipy import signal

audio = sf.read('./tap_alert.wav')[0]

hrirs = np.load('./dataset/hrirs.npy')
positions = np.load('./dataset/positions.npy')

print('=' * 75)
print(f"HRIRs shape: {hrirs.shape}")
print(f"Positions shape: {positions.shape}")
print('=' * 75)

fs = 48000
audio_samples = audio.shape[0]
n_filters = 10

rend = np.zeros((n_filters * audio_samples, 2), dtype=np.float64)
for i in range(n_filters):
    sample_idx = np.random.randint(0, len(positions))
    
    position = positions[sample_idx]
    hrir = hrirs[sample_idx]
    gain = random.random();
    
    print(f'Position {sample_idx}: {position} with gain {gain:.2f}')

    H = np.zeros((hrir.shape[0], 2), dtype=np.float64)
    H[:, 0] = hrir[:, 0]
    H[:, 1] = hrir[:, 1]

    audio_start = int(i * audio_samples)
    audio_end = int((i + 1) * audio_samples)

    rend_L = signal.fftconvolve(audio, H[:, 0], mode='same')
    rend_R = signal.fftconvolve(audio, H[:, 1], mode='same')

    rend_L /= np.max(np.abs(rend_L))
    rend_R /= np.max(np.abs(rend_R))

    rend[audio_start:audio_end, 0] = rend_L
    rend[audio_start:audio_end, 1] = rend_R

    rend[audio_start:audio_end, :] *= gain

sf.write('./output.wav', rend, int(fs))
