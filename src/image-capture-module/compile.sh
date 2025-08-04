mkdir -p build
cd build
cmake ..
make

# testing
ctest --output-on-failure -V
