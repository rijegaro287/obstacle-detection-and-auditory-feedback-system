cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(TransmissionModule)

# AddressSanitizer configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Define the module executable
add_executable(transmission_module transmission_module.cpp)
target_include_directories(transmission_module PRIVATE include)

# Define the test executable
add_executable(test_transmission_module test/test_transmission_module.cpp)
target_include_directories(test_transmission_module PRIVATE include)

# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_transmission_module 
         COMMAND test_transmission_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
