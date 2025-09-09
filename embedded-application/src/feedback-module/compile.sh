cmake -B build -S . -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build

# testing
# ctest --output-on-failure -V

./build/test_feedback_module
