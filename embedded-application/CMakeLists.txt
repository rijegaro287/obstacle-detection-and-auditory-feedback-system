cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ObstacleDetectionAndAuditoryFeedbackSystem)

include(CTest)

set_property(GLOBAL PROPERTY COVERAGE_EXECUTABLES "")

function(enable_coverage target)
  if(NOT COVERAGE)
    return()
  endif()

  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    return()
  endif()

  target_compile_options(${target} PRIVATE -O0 -g)

  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${target} PRIVATE
      -fprofile-instr-generate
      -fcoverage-mapping
    )
    target_link_options(${target} PRIVATE
      -fprofile-instr-generate
      -fcoverage-mapping
    )
  else()
    target_compile_options(${target} PRIVATE --coverage)
    target_link_options(${target} PRIVATE --coverage)
  endif()

  get_target_property(_target_type ${target} TYPE)
  if(_target_type STREQUAL "EXECUTABLE")
    set_property(GLOBAL APPEND PROPERTY COVERAGE_EXECUTABLES ${target})
  endif()
endfunction()

# AddressSanitizer configuration (disabled during coverage builds to avoid conflicts)
if(NOT COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif()

# Add all modules as subdirectories
add_subdirectory(src/configuration-module)
add_subdirectory(src/control-module)
add_subdirectory(src/image-capture-module)
add_subdirectory(src/obstacle-detection-module)
add_subdirectory(src/feedback-module)
add_subdirectory(src/transmission-module)

add_executable(obstacle-detection-and-auditory-feedback-system
  src/main.cpp
)
target_link_libraries(obstacle-detection-and-auditory-feedback-system PUBLIC
  configuration_module
  control_module
  image_capture_module
  obstacle_detection_module
  feedback_module
  transmission_module
)

enable_testing()
enable_coverage(obstacle-detection-and-auditory-feedback-system)

if(COVERAGE)
  get_property(_coverage_execs GLOBAL PROPERTY COVERAGE_EXECUTABLES)
  if(_coverage_execs)
    list(REMOVE_DUPLICATES _coverage_execs)
    set(_coverage_file_content "")
    foreach(_exec IN LISTS _coverage_execs)
      string(APPEND _coverage_file_content "$<TARGET_FILE:${_exec}>\n")
    endforeach()
    file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/coverage_targets.txt" CONTENT "${_coverage_file_content}")
  endif()
endif()
