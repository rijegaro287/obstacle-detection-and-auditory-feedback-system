cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project(ObstacleDetectionModule)

# AddressSanitizer configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(ArducamDepthCamera REQUIRED)

find_package(GTest REQUIRED)
include(GoogleTest)

set(CLIBS ArducamDepthCamera::ArducamDepthCamera4C)
set(CXXLIBS ArducamDepthCamera::ArducamDepthCamera)

# Define the module library
add_library(obstacle_detection_module STATIC
  obstacle_detection_module.cpp
)
target_include_directories(obstacle_detection_module PUBLIC
  include
  ../control-module/include
  ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(obstacle_detection_module PUBLIC
  ${CXXLIBS}
  ${OpenCV_LIBS}
  control_iface
)
target_link_libraries(obstacle_detection_module PRIVATE control_iface)

# Define the interface library
add_library(obstacle_detection_iface STATIC
  obstacle_detection_iface.cpp
)
target_include_directories(obstacle_detection_iface PUBLIC include)
target_link_libraries(obstacle_detection_iface PRIVATE obstacle_detection_module)

# Define the test executable with GTest
add_executable(test_obstacle_detection_module 
  test/test_obstacle_detection_module.cpp
)
target_link_libraries(test_obstacle_detection_module PRIVATE 
  obstacle_detection_module
)
gtest_discover_tests(test_obstacle_detection_module
  DISCOVERY_TIMEOUT 30
)
