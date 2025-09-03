cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ObstacleDetectionModule)

# AddressSanitizer configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(ArducamDepthCamera REQUIRED)

set(CLIBS ArducamDepthCamera::ArducamDepthCamera4C)
set(CXXLIBS ArducamDepthCamera::ArducamDepthCamera)

# Define the module executable
#add_executable(obstacle_detection_module obstacle_detection_module.cpp
#               ../image-capture-module/image_capture_module.cpp
#)
#target_include_directories(obstacle_detection_module PRIVATE
#    ${CMAKE_CURRENT_SOURCE_DIR}/include
#    ${CMAKE_CURRENT_SOURCE_DIR}/../image-capture-module/include
#)
#target_link_libraries(obstacle_detection_module PRIVATE ${CXXLIBS} ${OpenCV_LIBS})

# Define the test executable
add_executable(test_obstacle_detection_module test/test_obstacle_detection_module.cpp
               ../image-capture-module/image_capture_module.cpp
               obstacle_detection_module.cpp)
target_include_directories(test_obstacle_detection_module PRIVATE 
                            ${CMAKE_CURRENT_SOURCE_DIR}/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/../image-capture-module/include)
target_link_libraries(test_obstacle_detection_module PRIVATE ${CXXLIBS} ${OpenCV_LIBS})

# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_obstacle_detection_module 
         COMMAND test_obstacle_detection_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
