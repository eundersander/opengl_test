// gl_bench_sphere.cpp
// Build: g++ gl_bench_sphere.cpp -o gl_bench -lEGL -lGL -ldl -lpthread


#include <chrono>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>

#ifdef USE_GLES
#include <EGL/egl.h>
#include "glad_gles_without_egl/include/glad/gles2.h"
#else
#include <EGL/egl.h>
#include "glad_gl_46/include/glad/gl.h"
#endif

#ifdef USE_GLES
#define API_NAME "OpenGL ES"
#else
#define API_NAME "Desktop OpenGL"
#endif

bool g_useFBO = true;

// CLI args
struct Args {
    int targetVerts = 1'000'000;
    size_t texels = 64 * 1024 * 1024;
    int fragOps = 10;
    int fragFetches = 1;
    int frames = 100;
    std::string dumpFile;
    int width = 1024;
    int height = 1024;
    int debug = 0;
    std::string geom = "sphere"; // or "fullscreen"
};

Args parseArgs(int argc,char** argv){
    Args a;
    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i],"--verts")) a.targetVerts = atoi(argv[++i]);
        else if(!strcmp(argv[i],"--texels")) a.texels = strtoull(argv[++i],nullptr,10);
        else if(!strcmp(argv[i],"--frag-ops")) a.fragOps = atoi(argv[++i]);
        else if(!strcmp(argv[i],"--frag-fetches")) a.fragFetches = atoi(argv[++i]);
        else if(!strcmp(argv[i],"--frames")) a.frames = atoi(argv[++i]);
        else if(!strcmp(argv[i],"--debug")) a.debug = atoi(argv[++i]);
        else if(!strcmp(argv[i],"--dump")) a.dumpFile = argv[++i];
        else if(!strcmp(argv[i],"--geom")) {
            a.geom = argv[++i];  // "sphere" or "fullscreen"
        }
        else if(!strcmp(argv[i],"--res")) {
            int w,h;
            if (sscanf(argv[++i], "%dx%d", &w, &h) == 2) {
                a.width = w;
                a.height = h;
            } else {
                std::cerr << "Invalid --res format, use WxH (e.g. 2048x2048)\n";
                exit(1);
            }
        }
        else {
            std::cerr << "Unrecognized command-line argument: " << argv[i] << "\n";
            exit(1);
        }        
    }
    return a;
}

// Compile/link helpers
GLuint compileShader(GLenum type,const char* src){
    GLuint s=glCreateShader(type);
    glShaderSource(s,1,&src,nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){ char log[1024]; glGetShaderInfoLog(s,1024,nullptr,log);
        std::cerr<<"Shader error: "<<log<<"\n"; exit(1);}
    return s;
}
GLuint linkProgram(GLuint vs,GLuint fs){
    GLuint p=glCreateProgram();
    glAttachShader(p,vs); glAttachShader(p,fs);
    glLinkProgram(p);
    GLint ok; glGetProgramiv(p,GL_LINK_STATUS,&ok);
    if(!ok){ char log[1024]; glGetProgramInfoLog(p,1024,nullptr,log);
        std::cerr<<"Link error: "<<log<<"\n"; exit(1);}
    return p;
}

// Dump framebuffer to PPM
void dumpPPM(const std::string& path, int W, int H, bool useFBO)
{
    std::vector<unsigned char> buf(W * H * 4);

    if (useFBO) {
        // Ensure we're reading from the FBO color attachment
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    } else {
        // Default framebuffer (Pbuffer): no explicit read buffer needed
        // glReadBuffer(GL_BACK) would also work, but default is fine
    }

    glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open " << path << " for writing\n";
        return;
    }

    out << "P6\n" << W << " " << H << "\n255\n";
    for (int i = 0; i < W * H; ++i) {
        out.put(buf[i * 4 + 0]);
        out.put(buf[i * 4 + 1]);
        out.put(buf[i * 4 + 2]);
    }
}



void initGL(int W, int H, EGLDisplay& dpy, EGLContext& ctx, EGLSurface& surf, bool useFBO)
{
    dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (dpy == EGL_NO_DISPLAY) { std::cerr << "eglGetDisplay failed\n"; exit(1); }
    if (!eglInitialize(dpy, nullptr, nullptr)) { std::cerr << "eglInitialize failed\n"; exit(1); }

    EGLConfig cfg;
    EGLint n;
    EGLint cfgAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    #ifdef USE_GLES
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    #else
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    #endif
        EGL_NONE
    };
    if (!eglChooseConfig(dpy, cfgAttribs, &cfg, 1, &n) || n == 0) {
        std::cerr << "eglChooseConfig failed\n"; exit(1);
    }

#ifdef USE_GLES
    eglBindAPI(EGL_OPENGL_ES_API);
    EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
#else
    eglBindAPI(EGL_OPENGL_API);
    EGLint ctxAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 6,
        EGL_NONE
    };
#endif

    ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctxAttribs);
    if (ctx == EGL_NO_CONTEXT) { std::cerr << "eglCreateContext failed\n"; exit(1); }

    EGLint surfAttribs[] = { EGL_WIDTH, W, EGL_HEIGHT, H, EGL_NONE };
    surf = eglCreatePbufferSurface(dpy, cfg, surfAttribs);
    if (surf == EGL_NO_SURFACE) { std::cerr << "eglCreatePbufferSurface failed\n"; exit(1); }

    if (!eglMakeCurrent(dpy, surf, surf, ctx)) { std::cerr << "eglMakeCurrent failed\n"; exit(1); }

#ifdef USE_GLES
    if (!gladLoadGLES2((GLADloadfunc)eglGetProcAddress)) {
        std::cerr << "Failed to init GLES via GLAD\n"; exit(1);
    }
#else
    if (!gladLoadGL((GLADloadfunc)eglGetProcAddress)) {
        std::cerr << "Failed to init GL via GLAD\n"; exit(1);
    }
#endif

    std::cout << "API: " << API_NAME << "\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "Vendor:   " << glGetString(GL_VENDOR)   << "\n";
    std::cout << "Version:  " << glGetString(GL_VERSION)  << "\n";

    glViewport(0, 0, W, H);

    // ---------------------------------------------------------------
    // Optional FBO path — ONLY if useFBO == true
    // ---------------------------------------------------------------
    if (useFBO) {
        GLuint tex = 0, fbo = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "FBO incomplete: 0x" << std::hex << status << std::dec << "\n";
            exit(1);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, W, H);

        std::cout << "Initialized FBO (" << W << "x" << H << ")\n";

        GLint maxTex = 0;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTex);
        std::cout << "GL_MAX_TEXTURE_SIZE = " << maxTex << "\n";        
    } else {
        std::cout << "Initialized plain EGL Pbuffer (" << W << "x" << H << ")\n";
    }
}



// Build a tessellated sphere
struct Mesh { std::vector<float> verts; std::vector<unsigned int> idx; };

Mesh buildSphere(int targetVerts){
    // Estimate lon,lat ~ sqrt(N), aspect 2:1
    int lonSegs = std::max(3,(int)std::sqrt(targetVerts));
    int latSegs = std::max(2, lonSegs/2);

    int nVerts = (latSegs+1)*(lonSegs+1);
    int nTris = latSegs*lonSegs*2;

    Mesh m;
    m.verts.reserve(nVerts*5); // pos+uv
    m.idx.reserve(nTris*3);

    for(int lat=0;lat<=latSegs;lat++){
        float v=(float)lat/latSegs;
        float theta=v*M_PI;
        for(int lon=0;lon<=lonSegs;lon++){
            float u=(float)lon/lonSegs;
            float phi=u*2*M_PI;
            float x=sin(theta)*cos(phi);
            float y=cos(theta);
            float z=sin(theta)*sin(phi);
            m.verts.insert(m.verts.end(),{x,y,z,u,v});
        }
    }
    for(int lat=0;lat<latSegs;lat++){
        for(int lon=0;lon<lonSegs;lon++){
            int i0=lat*(lonSegs+1)+lon;
            int i1=i0+lonSegs+1;
            m.idx.insert(m.idx.end(),{i0,i1,i0+1,i1,i1+1,i0+1});
        }
    }
    std::cout<<"Sphere: lat="<<latSegs<<" lon="<<lonSegs
             <<" verts="<<nVerts<<" tris="<<nTris<<"\n";
    return m;
}

Mesh buildFullscreenTriangle() {
    Mesh m;
    // A single triangle that covers the entire screen when mapped to NDC
    m.verts = {
        // pos (x,y,z)    uv (u,v)
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         3.0f, -1.0f, 0.0f, 2.0f, 0.0f,
        -1.0f,  3.0f, 0.0f, 0.0f, 2.0f
    };
    m.idx = {0, 1, 2};
    return m;
}


GLuint makeShaders(const Args& args) {
#ifdef USE_GLES
    const char* version_vert = "#version 320 es\n";
    const char* version_frag = "#version 320 es\nprecision highp float;\n";
#else
    const char* version_vert = "#version 330 core\n";
    const char* version_frag = "#version 330 core\n";
#endif

    std::string vsrc, fsrc;

    if (args.debug) {
        // Debug shaders: solid color per triangle
        vsrc = R"(
            layout(location=0) in vec3 pos;
            flat out vec3 vColor;
            void main() {
                gl_Position = vec4(pos, 1.0);
                float id = float(gl_VertexID % 64) / 64.0;
                vColor = vec3(fract(id*13.0), fract(id*7.0), fract(id*3.0));
            })";
        fsrc = R"(
            flat in vec3 vColor;
            out vec4 color;
            void main() {
                color = vec4(vColor, 1.0);
            })";
    } else {
        // Benchmark shaders
        vsrc = R"(
            layout(location=0) in vec3 pos;
            layout(location=1) in vec2 uv;
            out vec2 vUV;
            void main() {
                gl_Position = vec4(pos, 1.0);
                vUV = uv;
            })";

        std::ostringstream fss;
        fss << R"(
            in vec2 vUV;
            out vec4 color;
            uniform sampler2D tex;
            void main() {
        )";

        // We’ll use 4 independent accumulators to break dependencies.
        // Each FMA = 2 FLOPs. 4 FMAs per loop iteration = 8 FLOPs.
        // Loop runs fragOps/8 times -> exactly fragOps FLOPs total.

        int iters = std::max(1, args.fragOps / 8);

        fss << "    float a = vUV.x;\n";
        fss << "    float b = vUV.y;\n";
        fss << "    float c = vUV.x + vUV.y;\n";
        fss << "    float d = 1.0;\n";

        fss << "    #pragma unroll\n";
        fss << "    for (int i = 0; i < " << iters << "; ++i) {\n";
        fss << "        a = a * 1.0001 + 0.0001;\n";
        fss << "        b = b * 1.0002 + 0.0002;\n";
        fss << "        c = c * 1.0003 + 0.0003;\n";
        fss << "        d = d * 1.0004 + 0.0004;\n";
        fss << "    }\n";
        fss << "    float accf = a + b + c + d;\n";
        fss << "    vec4 acc = vec4(accf);\n";


        // Sequential texture streaming: each fragment walks linearly through the texture
        fss << "    ivec2 texSize = textureSize(tex, 0);\n";
        fss << "    int texelsPerRow = texSize.x;\n";
        fss << "    int base = int(gl_FragCoord.y) * texelsPerRow + int(gl_FragCoord.x);\n";
        fss << "    for (int i = 0; i < " << args.fragFetches << "; ++i) {\n";
        fss << "        int idx = base * " << args.fragFetches << " + i;\n";
        fss << "        ivec2 coord = ivec2(idx % texelsPerRow, idx / texelsPerRow);\n";
        fss << "        acc += texelFetch(tex, coord, 0);\n";
        fss << "    }\n";

        

        fss << "    color = acc;\n}\n";
        fsrc = fss.str();
    }

    // Prepend GLSL version
    std::string vsrc_full = std::string(version_vert) + vsrc;
    std::string fsrc_full = std::string(version_frag) + fsrc;

    GLuint vs = compileShader(GL_VERTEX_SHADER, vsrc_full.c_str());
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsrc_full.c_str());
    return linkProgram(vs, fs);
}
    
GLuint createTexture(const Args& args, GLuint prog) {
    // Each texel is 4 bytes (RGBA8)
    size_t numTexels = args.texels;
    size_t side = (size_t)std::sqrt((double)numTexels);  // assume square texture
    if (side * side < numTexels) side++;                 // round up

    std::cout << "Allocating texture: " << side << "x" << side
              << " (" << (side*side) << " texels)\n";

    std::vector<unsigned char> data(side * side * 4);
    for (size_t i = 0; i < data.size(); ++i) {
        // Simple repeating pattern
        data[i] = (unsigned char)(i & 0xFF);
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 (GLsizei)side, (GLsizei)side,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    GLint texW = (GLint)side;
    GLint texH = (GLint)side;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Bind sampler uniform
    GLint loc = glGetUniformLocation(prog, "tex");
    if (loc >= 0) {
        glUseProgram(prog);
        glUniform1i(loc, 0);  // texture unit 0
    }

    return tex;
}


int main(int argc,char** argv){
    Args args=parseArgs(argc,argv);
    

    EGLDisplay dpy;
    EGLContext ctx;
    EGLSurface surf;
    initGL(args.width, args.height, dpy, ctx, surf, g_useFBO);

    // Build mesh
    Mesh mesh;
    if (args.geom == "fullscreen") {
        mesh = buildFullscreenTriangle();
    } else if (args.geom == "sphere") {
        mesh = buildSphere(args.targetVerts);
    } else {
        std::cerr << "Unknown geom: " << args.geom << "\n";
        exit(1);
    }

    // === VAO setup (core profile requires this) ===
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo,ibo;
    glGenBuffers(1,&vbo); glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,mesh.verts.size()*4,mesh.verts.data(),GL_STATIC_DRAW);
    glGenBuffers(1,&ibo); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,mesh.idx.size()*4,mesh.idx.data(),GL_STATIC_DRAW);

    GLuint prog = makeShaders(args);

    glUseProgram(prog);

    // Create and bind texture if fragFetches > 0
    GLuint tex = 0;
    if (args.fragFetches > 0) {
        tex = createTexture(args, prog);
    }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
   

    // clear to red
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // disable depth and culling to ensure we write something
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Warmup
    for(int i=0;i<5;i++){ glDrawElements(GL_TRIANGLES,mesh.idx.size(),GL_UNSIGNED_INT,0); glFinish(); }

    // === Timing ===
    auto start = std::chrono::high_resolution_clock::now();
    for (int f = 0; f < args.frames; f++) {
        glDrawElements(GL_TRIANGLES, mesh.idx.size(), GL_UNSIGNED_INT, 0);
        glFinish();
    }
    auto end = std::chrono::high_resolution_clock::now();
    double ms  = std::chrono::duration<double,std::milli>(end-start).count() / args.frames;
    double fps = 1000.0 / ms;

    std::cout << "Avg frame: " << ms << " ms (" << fps << " FPS)\n";

    // === Vertex throughput ===
    size_t nVerts = mesh.verts.size() / 5;
    std::cout << "Vertex throughput: " << (nVerts * fps) / 1e6 << " Mverts/sec\n";

    // === Fragment count estimate ===
    // Ideally: use occlusion query for exact shaded fragments.
    // Here: pick based on geometry mode.
    double fragsPerFrame;
    if (args.geom == "fullscreen") {
        fragsPerFrame = (double)args.width * args.height;
    } else {
        // Sphere covers ~circle in framebuffer. Approximate by area of inscribed circle.
        double radius = std::min(args.width, args.height) / 2.0;
        fragsPerFrame = M_PI * radius * radius;
    }

    // === Texture fetch throughput ===
    double gtexFetches = 0.0;
    double gbps = 0.0;
    if (args.fragFetches > 0) {
        double fetchesPerFrame = fragsPerFrame * args.fragFetches;
        gtexFetches = fetchesPerFrame * fps / 1e9;
        gbps = gtexFetches * 16.0 / 1.0;  // 16 bytes per RGBA8 texel
    }
    std::cout << "Texture fetch rate: " << gtexFetches << " Gfetches/sec\n";
    std::cout << "Bandwidth: " << gbps << " GB/s\n";


    // === FLOP throughput ===
    double gflops = 0.0;
    if (args.fragOps > 0) {
        double flopsPerPixel = (double)args.fragOps; // already exact FLOPs
        gflops = fragsPerFrame * flopsPerPixel * fps / 1e9;
    }
    std::cout << "GFLOP/s: " << gflops << "\n";

    if(!args.dumpFile.empty()){
        dumpPPM(args.dumpFile,args.width,args.height, g_useFBO);
        std::cout<<"Dumped frame to "<<args.dumpFile<<"\n";
    }
    return 0;
}
