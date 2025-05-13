import os
os.environ["PYOPENGL_PLATFORM"] = "egl"  # Force EGL backend

import moderngl
import numpy as np
from PIL import Image

# Create EGL context
ctx = moderngl.create_standalone_context()

# Identify backend
vendor = ctx.info.get("GL_VENDOR", "")
renderer = ctx.info.get("GL_RENDERER", "")
version = ctx.info.get("GL_VERSION", "")
print(f"Vendor:   {vendor}")
print(f"Renderer: {renderer}")
print(f"Version:  {version}")

# Optional: exit if Mesa
if "Mesa" in renderer or "llvmpipe" in renderer:
    raise RuntimeError("Renderer is Mesa/llvmpipe — not hardware-accelerated")

# Framebuffer
width, height = 800, 600
tex = ctx.texture((width, height), components=3)
fbo = ctx.framebuffer(color_attachments=[tex])
fbo.use()
fbo.clear(0.1, 0.1, 0.1, 1.0)  # Background

# Shaders
vs = """
#version 330
in vec2 in_pos;
void main() {
    gl_Position = vec4(in_pos, 0.0, 1.0);
}
"""
fs = """
#version 330
out vec4 fragColor;
void main() {
    fragColor = vec4(0.0, 1.0, 0.0, 1.0);  // Green
}
"""
prog = ctx.program(vertex_shader=vs, fragment_shader=fs)

# Triangle
vertices = np.array([-0.5, -0.5,  0.5, -0.5,  0.0, 0.5], dtype='f4')
vbo = ctx.buffer(vertices)
vao = ctx.simple_vertex_array(prog, vbo, 'in_pos')
vao.render(moderngl.TRIANGLES)

# Save image
data = tex.read()
image = Image.frombytes('RGB', (width, height), data)
image = image.transpose(Image.FLIP_TOP_BOTTOM)
image.save("triangle_egl_identified.png")
print("Image saved as triangle_egl_identified.png")

print("Spinning until you kill this...")
import time
while True:
    time.sleep(0)

