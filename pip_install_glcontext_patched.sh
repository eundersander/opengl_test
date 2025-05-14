#!/bin/bash
set -euo pipefail

echo "[INFO] Setting compiler and linker environment for local EGL build"

export EGL_DIR="$PWD/nvidia-gl"
export CFLAGS="-I${EGL_DIR}/EGL"
export LDFLAGS="-L${EGL_DIR}"
export LD_LIBRARY_PATH="${EGL_DIR}:${LD_LIBRARY_PATH:-}"

echo "[INFO] Uninstalling any existing glcontext"
pip uninstall -y glcontext || true

echo "[INFO] Installing patched glcontext from local source (no cache, no wheels)"
pip install ./glcontext_patched --no-build-isolation --no-cache-dir --no-binary :all:

echo "[INFO] Verifying that glcontext.egl loads and creates a context"

python -c 'import glcontext.egl; ctx = glcontext.egl.create_context(mode="standalone"); print("EGL context created:", ctx)'
