cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(AuditoryFeedbackModule)

configure_file(./tap_alert.npy ./tap_alert.npy COPYONLY)
configure_file(./dataset/hrirs.npy ./dataset/hrirs.npy COPYONLY)
configure_file(./dataset/positions.npy ./dataset/positions.npy COPYONLY)
configure_file(./dataset/verbal_feedback_signals.npy ./dataset/verbal_feedback_signals.npy COPYONLY)

set(CMAKE_PREFIX_PATH "./kfr/install/lib/cmake" ${CMAKE_PREFIX_PATH})
find_package(KFR CONFIG REQUIRED)

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Define the library
add_library(feedback_module STATIC
  feedback_module.cpp
  kd_tree.cpp
)
target_include_directories(feedback_module PUBLIC 
  include
  ../control-module/include
)
target_link_libraries(feedback_module PUBLIC control_iface kfr kfr_dsp)

add_library(feedback_iface STATIC 
  feedback_iface.cpp
)
target_include_directories(feedback_iface PUBLIC include)
target_link_libraries(feedback_iface PUBLIC feedback_module)

# # Define the test executable using the library
# add_executable(test_feedback_module
#   test/test_feedback_module.cpp
# )
# target_link_libraries(test_feedback_module PUBLIC feedback_module)


# # Enable testing for this module
# enable_testing()

# # Add the test
# add_test(NAME test_feedback_module 
#          COMMAND test_feedback_module
#          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
