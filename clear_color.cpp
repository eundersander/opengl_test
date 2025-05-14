#include <Magnum/Platform/WindowlessEglApplication.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/Image.h>
#include <Magnum/PixelFormat.h>
#include <Corrade/Containers/Array.h>
#include <fstream>
#include <Magnum/Math/Color.h> // For Color4
#include <Magnum/GL/BufferImage.h>   // for GL::BufferImage2D

using namespace Magnum;

class ClearApp : public Platform::WindowlessEglApplication {
public:
    explicit ClearApp(const Arguments& arguments): Platform::WindowlessEglApplication{arguments} {}

    int exec() override {
        Vector2i size{256, 256};

        GL::Renderbuffer color;
        color.setStorage(GL::RenderbufferFormat::RGBA8, size);

        GL::Framebuffer framebuffer{{{}, size}};
        framebuffer.attachRenderbuffer(GL::Framebuffer::ColorAttachment{0}, color);
        framebuffer.bind();
        framebuffer.clearColor(0, Color4{0.2f, 0.3f, 0.5f, 1.0f});
        framebuffer.clear(GL::FramebufferClear::Color);

        GL::BufferImage2D image = framebuffer.read(
            Range2Di{{}, size},
            PixelFormat::RGBA8Unorm,
            GL::BufferUsage::StaticRead
        );
        

        Containers::Array<char> data = image.buffer().data();
        std::ofstream out("clear_color.raw", std::ios::binary);
        out.write(data.begin(), data.size());
        out.close();        

        Debug{} << "Saved clear_color.raw";
        return 0;
    }
};

MAGNUM_WINDOWLESSEGLAPPLICATION_MAIN(ClearApp)
