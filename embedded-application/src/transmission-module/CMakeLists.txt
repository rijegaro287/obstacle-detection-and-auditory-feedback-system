cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

project(TransmissionModule)

configure_file(verbal_feedback_signals.npy verbal_feedback_signals.npy COPYONLY)

find_package(ALSA REQUIRED)

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Define the library
add_library(transmission_module STATIC
  transmission_module.cpp
)
target_include_directories(transmission_module PUBLIC
  include
  ../control-module/include
  ${ALSA_INCLUDE_DIRS}
)
target_link_libraries(transmission_module PRIVATE
  ${ALSA_LIBRARIES}
)

add_library(transmission_iface STATIC
  transmission_iface.cpp
)
target_include_directories(transmission_iface PUBLIC include)
target_link_libraries(transmission_iface PRIVATE transmission_module)

# Define the test executable using the library
add_executable(test_transmission_module 
  test/test_transmission_module.cpp
)
target_include_directories(test_transmission_module PRIVATE
  include
  ${ALSA_INCLUDE_DIRS}
)
target_link_libraries(test_transmission_module transmission_module)


# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_transmission_module 
         COMMAND test_transmission_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
