
# export DRIVER_LIB_DIR=/lib/x86_64-linux-gnu

# export __EGL_VENDOR_LIBRARY_FILENAMES=$PWD/nvidia-gl/nvidia.json
# export LD_LIBRARY_PATH=$PWD/nvidia-gl:$DRIVER_LIB_DIR:$LD_LIBRARY_PATH
# export PYOPENGL_PLATFORM=egl


unset __EGL_VENDOR_LIBRARY_FILENAMES
unset PYOPENGL_PLATFORM

export LD_LIBRARY_PATH=/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH \
