mkdir -p build
cd build
cmake ..
make

# testing
# ctest --output-on-failure -V

cd ..
./build/test_control_module
