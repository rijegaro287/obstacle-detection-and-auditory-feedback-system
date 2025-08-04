cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(AuditoryFeedbackModule)

# AddressSanitizer configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Define the module executable
add_executable(auditory_feedback_module auditory_feedback_module.cpp)
target_include_directories(auditory_feedback_module PRIVATE include)

# Define the test executable
add_executable(test_auditory_feedback_module test/test_auditory_feedback_module.cpp)
target_include_directories(test_auditory_feedback_module PRIVATE include)

# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_auditory_feedback_module 
         COMMAND test_auditory_feedback_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
