import ctypes
import os

os.environ["LD_LIBRARY_PATH"] = os.getcwd() + "/nvidia-gl"

try:
    lib = ctypes.CDLL("./nvidia-gl/libEGL.so.1")
    print("libEGL.so.1 loaded ✅")
    print("eglGetError addr:", lib.eglGetError)
except Exception as e:
    print("Failed to load libEGL.so.1:", e)
