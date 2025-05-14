#!/bin/bash

# Function to check the exit status of a command and exit if it failed
check_command() {
  if [ $? -ne 0 ]; then
    echo "Error: $1 failed" >&2
    exit 1
  fi
}

# Helper function to clean and create the build directory
clean_and_create_build_dir() {
  # If --clean was provided, remove the build directory
  if [ $CLEAN -eq 1 ] && [ -d "${BUILD_DIR}" ]; then
    rm -rf ${BUILD_DIR}
  fi
  # Create the build directory if it doesn't exist
  mkdir -p ${BUILD_DIR} && cd ${BUILD_DIR}
}

# Set the default build type and directory
BUILD_TYPE="Release"
BUILD_DIR="build"
CLEAN=0

# Process the command-line arguments
for arg in "$@"
do
  if [ "$arg" == "--debug" ]; then
    # If --debug is provided, set the build type to Debug and the directory to build_debug
    BUILD_TYPE="Debug"
    BUILD_DIR="build_debug"
  elif [ "$arg" == "--clean" ]; then
    # If --clean is provided, set the CLEAN variable
    CLEAN=1
  fi
done

if [ $CLEAN -eq 1 ] && [ -d "magnum_root" ]; then
  echo "Cleaning full magnum_root directory..."
  rm -rf magnum_root
fi

# Create the magnum_root directory if it doesn't exist
mkdir -p magnum_root && cd magnum_root

# If --clean was provided, remove the install_root directory
if [ $CLEAN -eq 1 ] && [ -d "install_root" ]; then
  rm -rf install_root
fi

# Create the install_root directory if it doesn't exist
mkdir -p install_root

# Clone the corrade repository if it doesn't exist
if [ ! -d "corrade" ]; then
  git clone https://github.com/mosra/corrade.git
  check_command "Cloning corrade repository"
fi
cd corrade
clean_and_create_build_dir

# Run CMake and make
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=../../install_root ..
check_command "Running CMake for corrade"
make
check_command "Building corrade"
make install -j$(nproc)
cd ../../

# Clone the magnum repository if it doesn't exist
if [ ! -d "magnum" ]; then
  git clone https://github.com/mosra/magnum.git
  check_command "Cloning magnum repository"
fi

# patch magnum CMakeLists.txt?
# MAGNUM_SRC_DIR="magnum"
# PATCH_FILE="${MAGNUM_SRC_DIR}/src/Magnum/Platform/CMakeLists.txt"

# if grep -q "add_library(MagnumEglContextObjects OBJECT" "$PATCH_FILE"; then
#     if ! grep -q "target_include_directories(MagnumEglContextObjects PRIVATE \${EGL_INCLUDE_DIR})" "$PATCH_FILE"; then
#         echo "Patching MagnumEglContextObjects to include custom EGL include path..."
#         sed -i '/add_library(MagnumEglContextObjects OBJECT/ a \
# if(EGL_INCLUDE_DIR)\n  target_include_directories(MagnumEglContextObjects PRIVATE ${EGL_INCLUDE_DIR})\nendif()' "$PATCH_FILE"
#     else
#         echo "Patch already applied to MagnumEglContextObjects. Skipping."
#     fi
# else
#     echo "ERROR: Could not find MagnumEglContextObjects definition in $PATCH_FILE"
#     exit 1
# fi

cd magnum
clean_and_create_build_dir

# EGL_DIR=$(realpath ../../../nvidia-gl)

# # Sanity check: must contain egl.h and libEGL.so
# if [ ! -f "${EGL_DIR}/EGL/egl.h" ]; then
#   echo "ERROR: Missing ${EGL_DIR}/EGL/egl.h"
#   exit 1
# fi

# if [ ! -f "${EGL_DIR}/libEGL.so" ]; then
#   echo "ERROR: Missing ${EGL_DIR}/libEGL.so"
#   exit 1
# fi

# See also gfx_batch/CMakeLists.txt find_package(Magnum ...). See also https://doc.magnum.graphics/magnum/building.html#building-features.
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=../../install_root -DMAGNUM_TARGET_EGL=ON -DMAGNUM_WITH_WINDOWLESSEGLAPPLICATION=ON -DMAGNUM_WITH_OPENGLTESTER=ON -DMAGNUM_WITH_DEBUGTOOLS=ON \
  ..
check_command "Running CMake for magnum"
make
check_command "Building magnum"
make install -j$(nproc)
cd ../../


