set -o errexit
set -o nounset
set -o pipefail 

sudo apt update -y
sudo apt full-upgrade -y

# GTest
sudo apt install libgtest-dev -y

# Python3 venv
sudo apt install python3-venv
python3 -m venv venv
source venv/bin/activate

# CMake
sudo apt install cmake -y

# Clang
sudo apt install clang -y

# Alsa
sudo apt install alsa-utils libasound2-dev -y

# Glib
sudo apt install libglib2.0-dev -y

# OpenCV
sudo apt install libopencv-dev -y

# eSpeak NG
sudo apt install espeak-ng -y

# Generate Feedback Module files
cd ./embedded-application/src/feedback-module
bash install_dependencies.sh

cd ./dataset
bash download_dataset.sh
python convert_to_npy.py
python generate_taps.py
python generate_verbal_feedback.py
rm *.wav

sudo apt autoremove -y

cd ../../../../

# Arducam TOF SDK
git clone https://github.com/ArduCAM/Arducam_tof_camera.git
cd Arducam_tof_camera
./Install_dependencies.sh
cd ..
rm -rf Arducam_tof_camera
