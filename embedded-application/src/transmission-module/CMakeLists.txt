cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

project(TransmissionModule)

configure_file(verbal_feedback_signals.npy verbal_feedback_signals.npy COPYONLY)

find_package(PkgConfig REQUIRED)
find_package(ALSA REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)

# AddressSanitizer configuration
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
# set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")

file(GLOB TRANSMISSION_MODULE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# Define the library
add_library(transmission_module STATIC
  ${TRANSMISSION_MODULE_SOURCES}
)
target_include_directories(transmission_module PUBLIC
  include
  ${ALSA_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GIO_INCLUDE_DIRS}
)
target_link_libraries(transmission_module 
  ${ALSA_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GIO_LIBRARIES}
)


# Define the test executable using the library
add_executable(test_transmission_module 
  test/test_transmission_module.cpp
)
target_include_directories(test_transmission_module PRIVATE
  include
  ${ALSA_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
  ${GIO_INCLUDE_DIRS}
)
target_link_libraries(test_transmission_module transmission_module)


# Enable testing for this module
enable_testing()

# Add the test
add_test(NAME test_transmission_module 
         COMMAND test_transmission_module
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
