#!/bin/bash
set -e

# Directories
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NVIDIA_GL_DIR="$ROOT_DIR/nvidia-gl"
EGL_JSON="$NVIDIA_GL_DIR/nvidia.json"
EGL_TEST_SRC="$ROOT_DIR/egl_test.cpp"
EGL_TEST_BIN="$ROOT_DIR/egl_test"

# Expected files
EXPECTED_FILES=(
  "$EGL_JSON"
  "$NVIDIA_GL_DIR/libEGL.so"
  "$NVIDIA_GL_DIR/libEGL.so.1"
  "$NVIDIA_GL_DIR/libEGL.so.1.1.0"
  "$NVIDIA_GL_DIR/libGL.so"
  "$NVIDIA_GL_DIR/libGL.so.1"
  "$NVIDIA_GL_DIR/libGL.so.1.7.0"
  "$NVIDIA_GL_DIR/libGLdispatch.so.0"
  "$NVIDIA_GL_DIR/libGLdispatch.so.0.0.0"
  "$NVIDIA_GL_DIR/libGLX.so.0"
  "$NVIDIA_GL_DIR/libGLX.so.0.0.0"
  "$NVIDIA_GL_DIR/libEGL_nvidia.so"
)

# Check that all expected files exist
for file in "${EXPECTED_FILES[@]}"; do
  if [[ ! -e "$file" ]]; then
    echo "ERROR: Required file missing: $file"
    exit 1
  fi
done

# Clean
echo "[INFO] Cleaning previous build..."
rm -f "$EGL_TEST_BIN"

# Build
echo "[INFO] Building egl_test..."
g++ "$EGL_TEST_SRC" -o "$EGL_TEST_BIN" \
    -I"$NVIDIA_GL_DIR" \
    -L"$NVIDIA_GL_DIR" \
    -lEGL

# Run
echo "[INFO] Running egl_test with NVIDIA EGL via GLVND..."
EGL_LOG_LEVEL=debug \
LD_LIBRARY_PATH="$NVIDIA_GL_DIR" \
__EGL_VENDOR_LIBRARY_FILENAMES="$EGL_JSON" \
"$EGL_TEST_BIN"
