import os
import ctypes.util

for name in ['EGL', 'GL', 'GLX']:
    path = ctypes.util.find_library(name)
    if path:
        try:
            full = os.popen(f'ldconfig -p | grep {path}').read().strip()
            print(f"{name}: {full}")
        except:
            print(f"{name}: {path}")
    else:
        print(f"{name}: not found")
