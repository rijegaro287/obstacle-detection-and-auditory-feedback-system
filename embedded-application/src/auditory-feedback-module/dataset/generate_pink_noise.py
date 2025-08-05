import numpy as np
import scipy.signal as signal
import soundfile as sf

def generate_pink_noise(duration_s, fs):
    """
    Generate pink noise using Voss–McCartney algorithm.
    """
    num_samples = int(duration_s * fs)
    num_rows = 16
    array = np.random.randn(num_rows, num_samples)
    array = np.cumsum(array, axis=1)
    weights = 2**np.arange(num_rows)
    pink = (array / weights[:, None]).sum(axis=0)
    pink /= np.max(np.abs(pink))
    return pink

def bandpass_filter(signal_in, fs, lowcut=500.0, highcut=10000.0, order=4):
    """
    Apply a Butterworth bandpass filter.
    """
    nyq = 0.5 * fs
    low = lowcut / nyq
    high = highcut / nyq
    b, a = signal.butter(order, [low, high], btype='band') # type: ignore
    return signal.lfilter(b, a, signal_in)

def apply_window(signal_in, fs, fade_ms=20):
    """
    Apply a raised‐cosine (Hann) fade‐in/out of fade_ms at start and end.
    """
    fade_samples = int((fade_ms / 1000) * fs)
    window = np.ones_like(signal_in)
    fade_in = 0.5 * (1 - np.cos(np.linspace(0, np.pi, fade_samples)))
    fade_out = fade_in[::-1]
    window[:fade_samples] = fade_in
    window[-fade_samples:] = fade_out
    return signal_in * window

# Parameters
fs = 48000              # Sampling rate (Hz)
duration = 10          # Burst length in seconds (300 ms)
lowcut, highcut = 500, 10000  # Band‐limit in Hz
fade_ms = 200            # Fade‐in/out duration in ms

# Generate pink noise burst
noise = generate_pink_noise(duration, fs)
noise = bandpass_filter(noise, fs, lowcut, highcut)
noise = apply_window(noise, fs, fade_ms)

# Normalize to –3 dBFS
noise /= np.max(np.abs(noise))
noise *= 10**(-3/20)

# Save to WAV file
sf.write('pink_noise.wav', noise, fs)
