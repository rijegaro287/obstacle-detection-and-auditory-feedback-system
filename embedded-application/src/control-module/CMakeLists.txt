cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ControlModule)

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

add_subdirectory(
    ../auditory-feedback-module
    ${CMAKE_CURRENT_BINARY_DIR}/auditory-feedback-module
)
add_subdirectory(
    ../transmission-module
    ${CMAKE_CURRENT_BINARY_DIR}/transmission-module
)

# Define the module executable
add_executable(control_module control_module.cpp)
target_include_directories(control_module PRIVATE include)

# Define the test executable
add_executable(test_control_module test/test_control_module.cpp)
target_include_directories(test_control_module PRIVATE include)

# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_control_module 
         COMMAND test_control_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
