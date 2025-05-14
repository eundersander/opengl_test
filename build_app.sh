#!/bin/bash

# Set the default build type and directory
BUILD_TYPE="Release"
BUILD_DIR="build"
CLEAN=0

# Process command-line arguments
for arg in "$@"
do
  if [ "$arg" == "--debug" ]; then
    BUILD_TYPE="Debug"
    BUILD_DIR="build_debug"
  elif [ "$arg" == "--clean" ]; then
    CLEAN=1
  fi
done

# Clean build directory if requested
if [ $CLEAN -eq 1 ] && [ -d "${BUILD_DIR}" ]; then
  rm -rf ${BUILD_DIR}
fi

# Create build directory if needed
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

# Set the path to Magnum installation
MAGNUM_PREFIX="../magnum_root/install_root"

# Run CMake
cmake \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DCMAKE_PREFIX_PATH=${MAGNUM_PREFIX} \
  -DMAGNUM_TARGET_EGL=ON \
  ..


# Build
make -j

# Optionally move binary somewhere
# cp ./egl_offscreen ../
