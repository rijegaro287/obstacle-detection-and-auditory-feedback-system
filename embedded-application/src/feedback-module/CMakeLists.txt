cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(AuditoryFeedbackModule)

# configure_file(./tap_alert.npy ./tap_alert.npy COPYONLY)
# configure_file(./dataset/hrirs.npy ./dataset/hrirs.npy COPYONLY)
# configure_file(./dataset/positions.npy ./dataset/positions.npy COPYONLY)
# configure_file(./dataset/verbal_feedback_signals.npy ./dataset/verbal_feedback_signals.npy COPYONLY)

include(FetchContent)

FetchContent_Declare(kfr
  GIT_REPOSITORY https://github.com/kfrlib/kfr.git
  GIT_TAG        origin/main
)

FetchContent_MakeAvailable(kfr)

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# add_subdirectory(../control-module ${CMAKE_CURRENT_BINARY_DIR}/control-module)

# Define the library
add_library(feedback_module STATIC
  feedback_module.cpp
  kd_tree.cpp
)
target_include_directories(feedback_module PUBLIC include)
target_include_directories(feedback_module PRIVATE ../control-module/include)
target_link_libraries(feedback_module PUBLIC kfr kfr_dsp)
target_link_libraries(feedback_module PRIVATE control_iface)

add_library(feedback_iface STATIC 
  feedback_iface.cpp
)
target_include_directories(feedback_iface PUBLIC include)
target_link_libraries(feedback_iface PRIVATE feedback_module)

# Define the test executable using the library
add_executable(test_feedback_module
  test/test_feedback_module.cpp
)
target_link_libraries(test_feedback_module PRIVATE feedback_module)


# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_feedback_module 
         COMMAND test_feedback_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
