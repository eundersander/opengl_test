# pip install PyOpenGL PyOpenGL_accelerate Pillow glfw numpy

import OpenGL.GL as gl
import glfw
import numpy as np
from PIL import Image

# Initialize GLFW
if not glfw.init():
    raise Exception("glfw cannot be initialized!")

# Configure GLFW for offscreen rendering
glfw.window_hint(glfw.VISIBLE, False)

# Create a window
window = glfw.create_window(800, 600, "Offscreen", None, None)
if not window:
    glfw.terminate()
    raise Exception("glfw window cannot be created!")

# Make the context current
glfw.make_context_current(window)

# OpenGL code to render a triangle
vertices = [-0.5, -0.5, 0.0, 0.5, -0.5, 0.0, 0.0, 0.5, 0.0]
vertices = np.array(vertices, dtype=np.float32)

# Create Vertex Buffer Object
vbo = gl.glGenBuffers(1)
gl.glBindBuffer(gl.GL_ARRAY_BUFFER, vbo)
gl.glBufferData(gl.GL_ARRAY_BUFFER, vertices.nbytes, vertices, gl.GL_STATIC_DRAW)

# Define vertex position attribute
gl.glEnableVertexAttribArray(0)
gl.glVertexAttribPointer(0, 3, gl.GL_FLOAT, gl.GL_FALSE, 0, None)

# Render a frame
gl.glClearColor(0.5, 0.5, 0.5, 1)
gl.glClear(gl.GL_COLOR_BUFFER_BIT)
gl.glDrawArrays(gl.GL_TRIANGLES, 0, 3)

# Read buffer into numpy array and convert to image
pixels = gl.glReadPixels(0, 0, 800, 600, gl.GL_RGB, gl.GL_UNSIGNED_BYTE)
image = Image.frombytes("RGB", (800, 600), pixels)
image = image.transpose(Image.FLIP_TOP_BOTTOM)

# Save to file
image.save('triangle.png')

# Clean up
glfw.terminate()
