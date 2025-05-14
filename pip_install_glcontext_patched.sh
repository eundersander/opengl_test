#!/bin/bash
set -euo pipefail

echo "[INFO] Building glcontext_patched using NVIDIA headers and Mesa linker"

export EGL_DIR="$PWD/nvidia-gl"
export CFLAGS="-I${EGL_DIR}/EGL"
export LDFLAGS="-L/usr/lib/x86_64-linux-gnu -lEGL"
export LD_LIBRARY_PATH="${EGL_DIR}:${LD_LIBRARY_PATH:-}"

echo "[INFO] Uninstalling existing glcontext"
pip uninstall -y glcontext || true

echo "[INFO] Deleting build artifacts"
rm -rf glcontext_patched/build glcontext_patched/glcontext.egg-info

echo "[INFO] Installing glcontext_patched from source"
pip install --no-binary :all: --no-build-isolation --no-cache-dir ./glcontext_patched --verbose

echo "[INFO] Checking for eglGetError symbol"
EGLOBJ=$(find ~/.conda/envs/$(basename "$CONDA_PREFIX")/lib/python*/site-packages/glcontext/ -name "egl.cpython-*-linux-gnu.so" | head -n 1)

if nm -D "$EGLOBJ" | grep -q eglGetError; then
    echo "[OK] eglGetError symbol is present"
else
    echo "[ERROR] eglGetError not found in built glcontext .so"
    exit 1
fi

echo "[INFO] Verifying runtime EGL context creation"
PYOPENGL_PLATFORM=egl \
__EGL_VENDOR_LIBRARY_FILENAMES=$PWD/nvidia-gl/nvidia.json \
LD_LIBRARY_PATH=$PWD/nvidia-gl:$LD_LIBRARY_PATH \
python -c 'import glcontext.egl; print(glcontext.egl.create_context(mode="standalone"))'
