cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ControlModule)

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui imgcodecs)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(GTest REQUIRED)
include(GoogleTest)

# AddressSanitizer configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Control module library
add_library(control_module STATIC
    control_module.cpp
)
target_include_directories(control_module PUBLIC
    include
    ../configuration-module/include
    ../image-capture-module/include
    ../obstacle-detection-module/include
    ../feedback-module/include
    ../transmission-module/include
)
target_link_libraries(control_module PUBLIC
    configuration_iface
    image_capture_iface
    obstacle_detection_iface
    feedback_iface
    transmission_iface
)

add_library(control_iface STATIC
    control_iface.cpp
)
target_include_directories(control_iface PUBLIC include)
target_link_libraries(control_iface PUBLIC control_module ${OpenCV_LIBS})

# Define the test executable
add_executable(test_control_module 
    test/test_control_module.cpp
)
target_link_libraries(test_control_module PRIVATE 
    GTest::gtest_main
    control_module
)
gtest_discover_tests(test_control_module
    DISCOVERY_TIMEOUT 30
)
