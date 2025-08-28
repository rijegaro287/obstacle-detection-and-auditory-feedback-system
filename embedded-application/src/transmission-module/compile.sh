mkdir -p build

cd build
cmake ..
make

# testing
# ctest --output-on-failure -V

cd ..
./build/transmission_module

# busctl introspect org.bluez /org/bluez/hci0