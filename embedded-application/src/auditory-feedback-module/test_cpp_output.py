import random
import numpy as np
import soundfile as sf
from scipy import signal

fs = 48000

left_channel = np.load("./output_l.npy")
right_channel = np.load("./output_r.npy")

print('=' * 75)
print(f"Left channel shape: {left_channel.shape}")
print(f"Right channel shape: {right_channel.shape}")
print('=' * 75)

rend = np.zeros((left_channel.shape[0], 2), dtype=np.float64)
rend[:, 0] = left_channel
rend[:, 1] = right_channel

sf.write('./output.wav', rend, int(fs))
