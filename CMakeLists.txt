cmake_minimum_required(VERSION 3.16)
project(MagnumClearColor)

# Required if Magnum isn't globally installed
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_SOURCE_DIR}/magnum_root/install_root")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/magnum_root/magnum/modules/;${CMAKE_MODULE_PATH}")

# Ensure custom EGL is used
set(EGL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/nvidia-gl/EGL")
set(EGL_LIBRARY "${CMAKE_SOURCE_DIR}/nvidia-gl/libEGL.so")
include(FindEGL)
message(STATUS "Using custom EGL: ${EGL_LIBRARY} / ${EGL_INCLUDE_DIR}")

if(MAGNUM_TARGET_EGL)
  set(WINDOWLESS_APPLICATION_PACKAGE "WindowlessEglApplication")
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

# Load required Magnum components
find_package(Magnum REQUIRED
  GL
  ${WINDOWLESS_APPLICATION_PACKAGE}
)

# Build executable
add_executable(clear_color clear_color.cpp)
target_link_libraries(clear_color PRIVATE
  Magnum::GL
  Magnum::${WINDOWLESS_APPLICATION_PACKAGE}
)
