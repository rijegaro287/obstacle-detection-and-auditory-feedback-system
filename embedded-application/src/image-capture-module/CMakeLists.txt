cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ImageCaptureModule)

# AddressSanitizer configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Define the module executable
add_executable(image_capture_module image_capture_module.cpp)
target_include_directories(image_capture_module PRIVATE include)

# Define the test executable
add_executable(test_image_capture_module test/test_image_capture_module.cpp)
target_include_directories(test_image_capture_module PRIVATE include)

# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_image_capture_module 
         COMMAND test_image_capture_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
