#!/bin/bash
set -e

# Directories
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NVIDIA_GL_DIR="$ROOT_DIR/nvidia-gl"
EGL_JSON="$NVIDIA_GL_DIR/nvidia.json"
EGL_TEST_SRC="$ROOT_DIR/egl_test.cpp"
EGL_TEST_BIN="$ROOT_DIR/egl_test"

# Optional: confirm the JSON file exists
if [[ ! -f "$EGL_JSON" ]]; then
  echo "ERROR: Missing $EGL_JSON"
  exit 1
fi

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
