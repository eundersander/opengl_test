

# export LD_DEBUG=libs
unset LD_DEBUG

export __EGL_VENDOR_LIBRARY_FILENAMES=$PWD/nvidia-gl/nvidia.json

# note we clear all other stuff in LD_LIBRARY_PATH

# do we want to consider so files in /lib/x86_64-linux-gnu?
export LD_LIBRARY_PATH=$PWD/nvidia-gl

# export LD_LIBRARY_PATH=$PWD/nvidia-gl:$LD_LIBRARY_PATH

export PYOPENGL_PLATFORM=egl

# unset __EGL_VENDOR_LIBRARY_FILENAMES
# unset PYOPENGL_PLATFORM
