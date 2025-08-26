mkdir -p build
cd build
cmake ..
make

# testing
# ctest --output-on-failure -V

# cd ..
# ./build/auditory_feedback_module

./auditory_feedback_module
