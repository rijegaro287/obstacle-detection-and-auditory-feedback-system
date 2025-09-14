pip install numpy scipy pysoundfile sounddevice python-sofa piper-tts

mkdir -p ./libs
cd ./libs

git clone https://github.com/kfrlib/kfr.git 
cd kfr
cmake -B build-release -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./install -DCMAKE_CXX_COMPILER=clang++
ninja -C build-release install
cmake -B build-debug -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=./install -DCMAKE_CXX_COMPILER=clang++
ninja -C build-debug install
