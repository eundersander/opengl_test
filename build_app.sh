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

# Use NVIDIA headers for compilation
EGL_INCLUDE_DIR=$(realpath ../nvidia-gl)

# Link against system Mesa for build
EGL_LIBRARY=/lib/x86_64-linux-gnu/libEGL.so.1

# Add NVIDIA libs to LD_LIBRARY_PATH for runtime
export LD_LIBRARY_PATH=$(realpath ../nvidia-gl):${LD_LIBRARY_PATH}

# Sanity checks
if [ ! -f "${EGL_INCLUDE_DIR}/EGL/egl.h" ]; then
  echo "ERROR: Missing EGL/egl.h in ${EGL_INCLUDE_DIR}"
  exit 1
fi

if [ ! -f "${EGL_LIBRARY}" ]; then
  echo "ERROR: Missing libEGL.so.1 at ${EGL_LIBRARY}"
  exit 1
fi

# Set the path to Magnum installation
MAGNUM_PREFIX="../magnum_root/install_root"

# Run CMake
cmake \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DCMAKE_PREFIX_PATH=${MAGNUM_PREFIX} \
  -DMAGNUM_TARGET_EGL=ON \
  -DEGL_INCLUDE_DIR=${EGL_INCLUDE_DIR} \
  -DEGL_LIBRARY=${EGL_LIBRARY} \
  ..

# Build
make -j VERBOSE=1


# Optionally move binary somewhere
# cp ./egl_offscreen ../
