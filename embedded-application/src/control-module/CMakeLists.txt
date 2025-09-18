cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ControlModule)

find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui imgcodecs)
include_directories(${OpenCV_INCLUDE_DIRS})

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

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

# # Define the test executable
# add_executable(test_control_module 
#     test/test_control_module.cpp
# )
# target_link_libraries(test_control_module PUBLIC 
#     control_module
#     # image_capture_iface
#     # obstacle_detection_iface
#     feedback_iface
#     transmission_iface
# )

# # Enable testing for this module
# enable_testing()

# # Add the test
# add_test(NAME test_control_module 
#          COMMAND test_control_module
#          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
