cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ControlModule)

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Control module library
add_library(control_module STATIC
    control_module.cpp
)
target_include_directories(control_module PUBLIC include)
target_include_directories(control_module PRIVATE 
    ../feedback-module/include
    ../transmission-module/include
)
target_link_libraries(control_module PRIVATE
    feedback_iface
    transmission_iface
)

add_library(control_iface STATIC
    control_iface.cpp
)
target_include_directories(control_iface PUBLIC include)
target_link_libraries(control_iface PRIVATE control_module)

# Define the test executable
add_executable(test_control_module 
    test/test_control_module.cpp
)
target_link_libraries(test_control_module PRIVATE 
    control_module
    feedback_iface
    transmission_iface
)

# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_control_module 
         COMMAND test_control_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
