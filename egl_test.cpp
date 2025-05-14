#include <EGL/egl.h>
#include <stdio.h>

int main() {
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (dpy == EGL_NO_DISPLAY) {
        printf("eglGetDisplay failed\n");
        return 1;
    }

    if (!eglInitialize(dpy, NULL, NULL)) {
        printf("eglInitialize failed\n");
        return 1;
    }

    const char* vendor = eglQueryString(dpy, EGL_VENDOR);
    const char* version = eglQueryString(dpy, EGL_VERSION);
    const char* extensions = eglQueryString(dpy, EGL_EXTENSIONS);

    printf("EGL_VENDOR    = %s\n", vendor);
    printf("EGL_VERSION   = %s\n", version);
    printf("EGL_EXTENSIONS= %s\n", extensions);

    eglTerminate(dpy);
    return 0;
}
