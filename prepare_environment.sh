set -o errexit
set -o nounset
set -o pipefail 

sudo apt update
sudo apt full-upgrade -y

# GTest
sudo apt install libgtest-dev

# Python3 venv
sudo apt install python3-venv
python3 -m venv venv
source venv/bin/activate

# CMake
sudo apt install cmake

# Clang
sudo apt install clang

# Alsa
sudo apt install alsa-utils libasound2-dev

# Glib
sudo apt install libglib2.0-dev

# OpenCV
sudo apt install libopencv-dev

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
