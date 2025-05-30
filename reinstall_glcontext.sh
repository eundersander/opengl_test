export LD_LIBRARY_PATH=$PWD/nvidia-gl:$LD_LIBRARY_PATH
export LIBRARY_PATH=$PWD/nvidia-gl
export CPATH=$PWD/nvidia-gl
export CFLAGS="-I$PWD/nvidia-gl"
export LDFLAGS="-L$PWD/nvidia-gl"

rm -rf ~/.cache/pip/wheels
pip uninstall -y glcontext
pip install -vvv --no-binary glcontext glcontext 2>&1 | tee build.log
