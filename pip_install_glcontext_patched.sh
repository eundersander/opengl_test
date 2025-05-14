#!/bin/bash
set -euo pipefail

echo "[INFO] Using local NVIDIA EGL headers and system Mesa libEGL for build"

export EGL_DIR="$PWD/nvidia-gl"
export CFLAGS="-I${EGL_DIR}/EGL"
export LDFLAGS="/usr/lib/x86_64-linux-gnu/libEGL.so.1"
export LD_LIBRARY_PATH="${EGL_DIR}:${LD_LIBRARY_PATH:-}"

echo "[INFO] Uninstalling any existing glcontext"
pip uninstall -y glcontext || true

echo "[INFO] Deleting old build artifacts"
rm -rf glcontext_patched/build glcontext_patched/glcontext.egg-info

echo "[INFO] Installing glcontext_patched from local source with EGL support"
pip install --no-binary :all: --no-build-isolation --no-cache-dir ./glcontext_patched --verbose

echo "[INFO] Checking for presence of eglGetError symbol in compiled .so"
EGLOBJ=$(find ~/.conda/envs/$(basename "$CONDA_PREFIX")/lib/python*/site-packages/glcontext/ -name "egl.cpython-*-linux-gnu.so" | head -n 1)
nm -D "$EGLOBJ" | grep eglGetError || echo "[WARNING] eglGetError not statically linked (but runtime may still succeed)"

echo "[INFO] Verifying runtime EGL context creation"
PYOPENGL_PLATFORM=egl \
__EGL_VENDOR_LIBRARY_FILENAMES=$PWD/nvidia-gl/nvidia.json \
LD_LIBRARY_PATH=$PWD/nvidia-gl:$LD_LIBRARY_PATH \
python -c 'import glcontext.egl; print(glcontext.egl.create_context(mode="standalone"))'
