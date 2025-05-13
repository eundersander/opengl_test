export LD_LIBRARY_PATH=$PWD/nvidia-gl:$LD_LIBRARY_PATH
export LIBRARY_PATH=$PWD/nvidia-gl
export CPATH=$PWD/nvidia-gl
export CFLAGS="-I$PWD/nvidia-gl"
export LDFLAGS="-L$PWD/nvidia-gl"

pip uninstall -y glcontext
pip install --no-binary glcontext glcontext