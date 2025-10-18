cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ObstacleDetectionAndAuditoryFeedbackSystem)

# AddressSanitizer configuration
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

# Add all modules as subdirectories
add_subdirectory(src/configuration-module)
add_subdirectory(src/control-module)
add_subdirectory(src/image-capture-module)
add_subdirectory(src/obstacle-detection-module)
add_subdirectory(src/feedback-module)
add_subdirectory(src/transmission-module)

add_executable(main
  src/main.cpp
)
target_link_libraries(main PUBLIC
  configuration_module
  control_module
  image_capture_module
  obstacle_detection_module
  feedback_module
  transmission_module
)

