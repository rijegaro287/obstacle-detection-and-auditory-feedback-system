import numpy as np
import soundfile as sf

SAMPLE_RATE = 48000
TAP_REPETITIONS = 5
ALERT_DURATION = 1
TAP_DURATION = ALERT_DURATION / TAP_REPETITIONS
DECAY_FACTOR = 25.0
FILENAME = "tap_alert.wav"

# 1. Generate a time array
num_samples = int(TAP_DURATION * SAMPLE_RATE)
t = np.linspace(0., TAP_DURATION, num_samples, endpoint=False, dtype=np.float64)

# 2. Create a broadband noise signal
# Using white noise provides a rich, full-frequency spectrum.
noise = np.random.uniform(-1, 1, num_samples)

# 3. Create a percussive envelope (sharp attack, fast exponential decay)
# This shapes the noise into a "tap" or "click" sound.
envelope = np.exp(-DECAY_FACTOR * t, dtype=np.float64)

# 4. Apply the envelope to the noise
tap_sound = noise * envelope

# 5. Repeat the sound for the specified number of taps
alert = np.zeros(num_samples * TAP_REPETITIONS, dtype=np.float64)
for i in range(TAP_REPETITIONS):
    alert[i * num_samples:(i + 1) * num_samples] = tap_sound

sf.write(FILENAME, alert, SAMPLE_RATE)
