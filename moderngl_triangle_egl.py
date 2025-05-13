import moderngl
import numpy as np
from PIL import Image

# Create a headless EGL context
ctx = moderngl.create_standalone_context()

# Create framebuffer and texture
width, height = 800, 600
tex = ctx.texture((width, height), components=3)
fbo = ctx.framebuffer(color_attachments=[tex])
fbo.use()
fbo.clear(0.1, 0.1, 0.1, 1.0)  # Dark gray background

# Vertex shader
vs_src = """
#version 330
in vec2 in_pos;
void main() {
    gl_Position = vec4(in_pos, 0.0, 1.0);
}
"""

# Fragment shader
fs_src = """
#version 330
out vec4 fragColor;
void main() {
    fragColor = vec4(0.0, 1.0, 0.0, 1.0);  // Green
}
"""

# Compile shader program
prog = ctx.program(vertex_shader=vs_src, fragment_shader=fs_src)

# Triangle vertex buffer (2D)
vertices = np.array([
    -0.5, -0.5,
     0.5, -0.5,
     0.0,  0.5,
], dtype='f4')
vbo = ctx.buffer(vertices)
vao = ctx.simple_vertex_array(prog, vbo, 'in_pos')

# Draw
vao.render(moderngl.TRIANGLES)

# Read pixels
data = tex.read()
image = Image.frombytes('RGB', (width, height), data)
image = image.transpose(Image.FLIP_TOP_BOTTOM)
image.save('triangle_egl.png')
