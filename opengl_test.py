# pip install PyOpenGL PyOpenGL_accelerate Pillow glfw numpy

import glfw
from OpenGL import GL
import numpy as np
from PIL import Image

WIDTH, HEIGHT = 800, 600

# Initialize GLFW
glfw.set_error_callback(lambda code, desc: print(f"GLFW Error {code}: {desc.decode()}"))
if not glfw.init():
    raise RuntimeError("GLFW init failed")

# Headless context (invisible window)
glfw.window_hint(glfw.VISIBLE, glfw.FALSE)
glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3)
glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3)
glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)

window = glfw.create_window(WIDTH, HEIGHT, "Offscreen", None, None)
if not window:
    glfw.terminate()
    raise RuntimeError("GLFW window creation failed")
glfw.make_context_current(window)

# --- Framebuffer setup ---
fbo = GL.glGenFramebuffers(1)
GL.glBindFramebuffer(GL.GL_FRAMEBUFFER, fbo)

tex = GL.glGenTextures(1)
GL.glBindTexture(GL.GL_TEXTURE_2D, tex)
GL.glTexImage2D(GL.GL_TEXTURE_2D, 0, GL.GL_RGB, WIDTH, HEIGHT, 0, GL.GL_RGB, GL.GL_UNSIGNED_BYTE, None)
GL.glTexParameteri(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MIN_FILTER, GL.GL_LINEAR)
GL.glFramebufferTexture2D(GL.GL_FRAMEBUFFER, GL.GL_COLOR_ATTACHMENT0, GL.GL_TEXTURE_2D, tex, 0)

rbo = GL.glGenRenderbuffers(1)
GL.glBindRenderbuffer(GL.GL_RENDERBUFFER, rbo)
GL.glRenderbufferStorage(GL.GL_RENDERBUFFER, GL.GL_DEPTH24_STENCIL8, WIDTH, HEIGHT)
GL.glFramebufferRenderbuffer(GL.GL_FRAMEBUFFER, GL.GL_DEPTH_STENCIL_ATTACHMENT, GL.GL_RENDERBUFFER, rbo)

assert GL.glCheckFramebufferStatus(GL.GL_FRAMEBUFFER) == GL.GL_FRAMEBUFFER_COMPLETE

# --- Triangle geometry ---
vertices = np.array([
    -0.5, -0.5, 0.0,
     0.5, -0.5, 0.0,
     0.0,  0.5, 0.0,
], dtype=np.float32)

# --- Shaders ---
VERTEX_SHADER = """
#version 330 core
layout(location = 0) in vec3 position;
void main() {
    gl_Position = vec4(position, 1.0);
}
"""

FRAGMENT_SHADER = """
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.0, 1.0, 0.0, 1.0);  // Green
}
"""

def compile_shader(src, type):
    shader = GL.glCreateShader(type)
    GL.glShaderSource(shader, src)
    GL.glCompileShader(shader)
    if not GL.glGetShaderiv(shader, GL.GL_COMPILE_STATUS):
        raise RuntimeError(GL.glGetShaderInfoLog(shader).decode())
    return shader

vs = compile_shader(VERTEX_SHADER, GL.GL_VERTEX_SHADER)
fs = compile_shader(FRAGMENT_SHADER, GL.GL_FRAGMENT_SHADER)
program = GL.glCreateProgram()
GL.glAttachShader(program, vs)
GL.glAttachShader(program, fs)
GL.glLinkProgram(program)
if not GL.glGetProgramiv(program, GL.GL_LINK_STATUS):
    raise RuntimeError(GL.glGetProgramInfoLog(program).decode())

GL.glDeleteShader(vs)
GL.glDeleteShader(fs)

# --- VAO + VBO ---
vao = GL.glGenVertexArrays(1)
vbo = GL.glGenBuffers(1)

GL.glBindVertexArray(vao)
GL.glBindBuffer(GL.GL_ARRAY_BUFFER, vbo)
GL.glBufferData(GL.GL_ARRAY_BUFFER, vertices.nbytes, vertices, GL.GL_STATIC_DRAW)
GL.glEnableVertexAttribArray(0)
GL.glVertexAttribPointer(0, 3, GL.GL_FLOAT, GL.GL_FALSE, 0, None)

# --- Render to FBO ---
GL.glBindFramebuffer(GL.GL_FRAMEBUFFER, fbo)
GL.glViewport(0, 0, WIDTH, HEIGHT)
GL.glClearColor(0.1, 0.1, 0.1, 1.0)
GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT)
GL.glUseProgram(program)
GL.glBindVertexArray(vao)
GL.glDrawArrays(GL.GL_TRIANGLES, 0, 3)

# --- Read pixels ---
pixels = GL.glReadPixels(0, 0, WIDTH, HEIGHT, GL.GL_RGB, GL.GL_UNSIGNED_BYTE)
image = Image.frombytes("RGB", (WIDTH, HEIGHT), pixels)
image = image.transpose(Image.FLIP_TOP_BOTTOM)
image.save("triangle_fbo.png")

# --- Clean up ---
GL.glDeleteProgram(program)
GL.glDeleteBuffers(1, [vbo])
GL.glDeleteVertexArrays(1, [vao])
GL.glDeleteRenderbuffers(1, [rbo])
GL.glDeleteTextures(1, [tex])
GL.glDeleteFramebuffers(1, [fbo])
glfw.destroy_window(window)
glfw.terminate()
