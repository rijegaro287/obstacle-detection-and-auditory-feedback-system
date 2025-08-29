import numpy as np
import soundfile as sf

SAMPLE_RATE = 48000
TAP_REPETITIONS = 5
ALERT_DURATION = 1
TAP_DURATION = ALERT_DURATION / TAP_REPETITIONS
DECAY_FACTOR = 25.0
FILENAME = "tap_alert"

num_samples = int(TAP_DURATION * SAMPLE_RATE)

t = np.linspace(0., TAP_DURATION, num_samples, endpoint=False, dtype=np.float64)

noise = np.random.uniform(-1, 1, num_samples).astype(np.float64)

envelope = np.exp(-DECAY_FACTOR * t, dtype=np.float64)

alert = np.zeros(num_samples * TAP_REPETITIONS, dtype=np.float64)
for i in range(TAP_REPETITIONS):
    alert[i * num_samples:(i + 1) * num_samples] = (noise * envelope).astype(np.float64)

np.save(f'{FILENAME}.npy', alert)
sf.write(f'{FILENAME}.wav', alert, SAMPLE_RATE)
