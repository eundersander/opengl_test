#!/bin/bash
set -euo pipefail

echo "[INFO] Setting local EGL header and library paths"

export EGL_DIR="$PWD/nvidia-gl"
export CFLAGS="-I${EGL_DIR}"
export LDFLAGS="-L${EGL_DIR}"
export LD_LIBRARY_PATH="${EGL_DIR}:${LD_LIBRARY_PATH:-}"

echo "[INFO] Uninstalling any existing glcontext"
pip uninstall -y glcontext || true

echo "[INFO] Installing glcontext from local source with local EGL support"
CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" pip install --no-binary glcontext ./glcontext_patched

echo "[INFO] Verifying that glcontext.egl loads and creates a context"
python -c 'import glcontext.egl; ctx = glcontext.egl.create_context(mode="standalone"); print("EGL context created:", ctx)'
