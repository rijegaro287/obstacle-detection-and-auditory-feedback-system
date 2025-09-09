import random
import numpy as np
import soundfile as sf
from scipy import signal

print('=' * 75)
print('NON-VERBAL')

fs = 48000

left_channel = np.load("./output_non_verbal_l.npy")
right_channel = np.load("./output_non_verbal_r.npy")

print(f"Left channel shape: {left_channel.shape}")
print(f"Right channel shape: {right_channel.shape}")
print('=' * 75)

rend = np.zeros((left_channel.shape[0], 2), dtype=np.float64)
rend[:, 0] = left_channel
rend[:, 1] = right_channel

sf.write('./output_non_verbal.wav', rend, int(fs))

print('=' * 75)
print('VERBAL')

fs = 22050

signal = np.load("output_verbal.npy")

print(f'Verbal signal shape: {signal.shape}')
print('=' * 75)

sf.write('output_verbal.wav', signal, int(fs))
