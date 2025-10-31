cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

project(TransmissionModule)

find_package(ALSA REQUIRED)

find_package(GTest REQUIRED)
include(GoogleTest)

# AddressSanitizer configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Define the library
add_library(transmission_module STATIC
  transmission_module.cpp
)
target_include_directories(transmission_module PUBLIC
  include
  ../control-module/include
  ${ALSA_INCLUDE_DIRS}
)
target_link_libraries(transmission_module PUBLIC
  ${ALSA_LIBRARIES}
  control_iface
)

add_library(transmission_iface STATIC
  transmission_iface.cpp
)
target_include_directories(transmission_iface PUBLIC include)
target_link_libraries(transmission_iface PUBLIC transmission_module)

# Define the test executable
add_executable(test_transmission_module 
    test/test_transmission_module.cpp
)
target_link_libraries(test_transmission_module PRIVATE 
    GTest::gtest_main
    control_module
)
gtest_discover_tests(test_transmission_module)
