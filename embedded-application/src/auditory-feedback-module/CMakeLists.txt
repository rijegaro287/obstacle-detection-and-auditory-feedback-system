cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(AuditoryFeedbackModule)

configure_file(./tap_alert.npy ./tap_alert.npy COPYONLY)
configure_file(./dataset/hrirs.npy ./dataset/hrirs.npy COPYONLY)
configure_file(./dataset/positions.npy ./dataset/positions.npy COPYONLY)
configure_file(./dataset/verbal_feedback_signals.npy ./dataset/verbal_feedback_signals.npy COPYONLY)

include(FetchContent)

FetchContent_Declare(kfr
  GIT_REPOSITORY https://github.com/kfrlib/kfr.git
  GIT_TAG        origin/main
)

FetchContent_MakeAvailable(kfr)

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Define the test executable
add_executable(test_auditory_feedback_module 
  test/test_auditory_feedback_module.cpp
  auditory_feedback_module.cpp
  kd_tree.cpp
)
target_include_directories(test_auditory_feedback_module PRIVATE include)
target_link_libraries(test_auditory_feedback_module PRIVATE kfr kfr_dsp)

# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_auditory_feedback_module 
         COMMAND test_auditory_feedback_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
