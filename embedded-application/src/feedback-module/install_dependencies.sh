set -o errexit
set -o nounset
set -o pipefail 

sudo apt-get install libportaudio2 

pip install numpy scipy pysoundfile sounddevice python-sofa piper-tts librosa
