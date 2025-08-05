import random
import numpy as np
import soundfile as sf
from scipy import signal

audio = sf.read('./pink_noise.wav')[0]

hrirs = np.load('./hrirs.npy')
positions = np.load('./positions.npy')

print('=' * 75)
print(f"HRIRs shape: {hrirs.shape}")
print(f"Positions shape: {positions.shape}")
print('=' * 75)

fs = 48000
duration_s = 10

rend = np.zeros((fs*duration_s, 2), dtype=np.float64)
n_samples = fs
for i in range(duration_s):
    sample_idx = np.random.randint(0, len(positions))
    
    position = positions[sample_idx]
    hrir = hrirs[sample_idx]
    gain = random.random();
    
    print(f'Position {sample_idx}: {position} with gain {gain:.2f}')

    H = np.zeros((hrir.shape[0], 2), dtype=np.float64)
    H[:, 0] = hrir[:, 0]
    H[:, 1] = hrir[:, 1]

    audio_start = int(i * n_samples)
    audio_end = int((i + 1) * n_samples)

    rend_L = signal.fftconvolve(audio[audio_start:audio_end], H[:, 0], mode='same')
    rend_R = signal.fftconvolve(audio[audio_start:audio_end], H[:, 1], mode='same')

    rend_L /= np.max(np.abs(rend_L))
    rend_R /= np.max(np.abs(rend_R))

    rend[audio_start:audio_end, 0] = rend_L
    rend[audio_start:audio_end, 1] = rend_R

    rend[audio_start:audio_end, :] *= gain

sf.write('./output.wav', rend, int(fs))
