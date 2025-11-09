cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(AuditoryFeedbackModule)

configure_file(./dataset/tap_alert.npy ./dataset/tap_alert.npy COPYONLY)
configure_file(./dataset/hrirs.npy ./dataset/hrirs.npy COPYONLY)
configure_file(./dataset/positions.npy ./dataset/positions.npy COPYONLY)
configure_file(./dataset/verbal_feedback_signals.npy ./dataset/verbal_feedback_signals.npy COPYONLY)

include(FetchContent)

FetchContent_Declare(kfr
  GIT_REPOSITORY https://github.com/kfrlib/kfr.git
  GIT_TAG        38c3205277a49a08d8eca13d11d4d7b7cf15e38e
)
FetchContent_MakeAvailable(kfr)

find_package(GTest REQUIRED)
include(GoogleTest)

# AddressSanitizer configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Define the library
add_library(feedback_module STATIC
  feedback_module.cpp
  kd_tree.cpp
)
target_include_directories(feedback_module PUBLIC 
  include
  ../control-module/include
)
target_link_libraries(feedback_module PUBLIC 
  kfr 
  kfr_dft
  control_iface 
)
enable_coverage(feedback_module)

add_library(feedback_iface STATIC 
  feedback_iface.cpp
)
target_include_directories(feedback_iface PUBLIC include)
target_link_libraries(feedback_iface PUBLIC feedback_module)
enable_coverage(feedback_iface)

# Define the test executable
enable_testing()

add_executable(test_feedback_module 
  test/test_feedback_module.cpp
)
target_link_libraries(test_feedback_module PRIVATE 
  GTest::gtest_main
  feedback_module
)
# Use gtest_discover_tests with proper working directory
# This ensures tests run from build root where dataset paths are correct
gtest_discover_tests(test_feedback_module
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DISCOVERY_TIMEOUT 30
)
enable_coverage(test_feedback_module)
