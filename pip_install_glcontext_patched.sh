#!/bin/bash
set -euo pipefail

echo "[INFO] Setting local EGL header and library paths"

export EGL_DIR="$PWD/nvidia-gl"
export CFLAGS="-I${EGL_DIR}"
export LDFLAGS="-L${EGL_DIR} -lEGL"
export LD_LIBRARY_PATH="${EGL_DIR}:${LD_LIBRARY_PATH:-}"

echo "[INFO] Uninstalling any existing glcontext"
pip uninstall -y glcontext || true

echo "[INFO] Deleting old build artifacts"
rm -rf glcontext_patched/build glcontext_patched/glcontext.egg-info

echo "[INFO] Installing glcontext from local source with full verbose output"
# use --no-cache-dir to prevent wheel reuse, and --verbose to show the compiler line
pip install --no-binary :all: --no-build-isolation --no-cache-dir ./glcontext_patched --verbose

echo "[INFO] Build finished. Checking that eglGetError is now defined in compiled .so"
EGLOBJ=$(find ~/.conda/envs/$(basename $CONDA_PREFIX)/lib/python*/site-packages/glcontext/ -name "egl.cpython-*-linux-gnu.so" | head -n 1)

if [ -z "$EGLOBJ" ]; then
    echo "[ERROR] Could not find built egl .so file"
    exit 1
fi

nm -D "$EGLOBJ" | grep eglGetError || {
    echo "[ERROR] eglGetError is still undefined — likely linking failed"
    exit 1
}

echo "[INFO] Verifying runtime: glcontext.egl context creation"
python -c 'import glcontext.egl; ctx = glcontext.egl.create_context(mode="standalone"); print("EGL context created:", ctx)'
