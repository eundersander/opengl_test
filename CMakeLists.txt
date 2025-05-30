cmake_minimum_required(VERSION 3.16)
project(MagnumClearColor)

# Required if Magnum isn't globally installed
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_SOURCE_DIR}/magnum_root/install_root")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/magnum_root/magnum/modules/;${CMAKE_MODULE_PATH}")

find_package(EGL REQUIRED)
message(STATUS "Using custom EGL: ${EGL_LIBRARY} / ${EGL_INCLUDE_DIR}")

if(MAGNUM_TARGET_EGL)
  set(WINDOWLESS_APPLICATION_PACKAGE "WindowlessEglApplication")
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

# Load required Magnum components
find_package(Magnum REQUIRED
  ${WINDOWLESS_APPLICATION_PACKAGE}
)

# Build executable
add_executable(clear_color clear_color.cpp)
# target_link_libraries(clear_color PRIVATE
#   Magnum::GL
#   Magnum::${WINDOWLESS_APPLICATION_PACKAGE}
#   EGL::EGL
# )

target_include_directories(clear_color PRIVATE
  ${CMAKE_SOURCE_DIR}/magnum_root/install_root/include
)

# Explicitly list only the needed .a and .so files, skip libOpenGL.so
target_link_libraries(clear_color PRIVATE
  ${CMAKE_SOURCE_DIR}/magnum_root/install_root/lib/libMagnumGL.so
  ${CMAKE_SOURCE_DIR}/magnum_root/install_root/lib/libMagnumWindowlessEglApplication.a
  ${CMAKE_SOURCE_DIR}/magnum_root/install_root/lib/libMagnum.so
  ${CMAKE_SOURCE_DIR}/magnum_root/install_root/lib/libCorradeUtility.so
  EGL::EGL
  dl
)