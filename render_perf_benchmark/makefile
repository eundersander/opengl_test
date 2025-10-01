CXX = g++

# Default to Release build unless DEBUG=1 is set
ifeq ($(DEBUG),1)
    CXXFLAGS = -O0 -g -std=c++17 -DDEBUG
else
    CXXFLAGS = -O2 -g -std=c++17 -Wno-narrowing
endif

GL_LDFLAGS = -lEGL -lGL -ldl -lpthread
GLES_LDFLAGS = -lEGL -lGLESv2 -ldl -lpthread

SRC = render_perf_benchmark.cpp

# Desktop OpenGL build
GL_INCLUDES = -Iglad_gl_46/include
GL_SOURCES  = glad_gl_46/src/gl.c
GL_TARGET   = render_perf_benchmark_gl

# OpenGL ES build
GLES_INCLUDES = -Iglad_gles/include
GLES_SOURCES  = glad_gles_without_egl/src/gles2.c
GLES_TARGET   = render_perf_benchmark_gles

all: $(GL_TARGET) $(GLES_TARGET)

$(GL_TARGET): $(SRC) $(GL_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(GL_INCLUDES) $(GL_LDFLAGS)

$(GLES_TARGET): $(SRC) $(GLES_SOURCES)
	$(CXX) $(CXXFLAGS) -DUSE_GLES -DGLAD_GLES2_USE_SYSTEM_EGL -o $@ $^ $(GLES_INCLUDES) $(GLES_LDFLAGS)

clean:
	rm -f $(GL_TARGET) $(GLES_TARGET) *.o
