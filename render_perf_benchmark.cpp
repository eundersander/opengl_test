// gl_bench_sphere.cpp
// Build: g++ gl_bench_sphere.cpp -o gl_bench -lEGL -lGL -ldl -lpthread


#include <EGL/egl.h>
#include <chrono>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <fstream>

#include "glad/include/glad/gl.h"

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
};

Args parseArgs(int argc,char** argv){
    Args a;
    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i],"--verts")) a.targetVerts = atoi(argv[++i]);
        else if(!strcmp(argv[i],"--texels")) a.texels = strtoull(argv[++i],nullptr,10);
        else if(!strcmp(argv[i],"--frag-ops")) a.fragOps = atoi(argv[++i]);
        else if(!strcmp(argv[i],"--frag-fetches")) a.fragFetches = atoi(argv[++i]);
        else if(!strcmp(argv[i],"--frames")) a.frames = atoi(argv[++i]);
        else if(!strcmp(argv[i],"--dump")) a.dumpFile = argv[++i];
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
void dumpPPM(const std::string& path,int w,int h){
    std::vector<unsigned char> buf(w*h*4);
    glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,buf.data());
    std::ofstream out(path,std::ios::binary);
    out<<"P6\n"<<w<<" "<<h<<"\n255\n";
    for(int i=0;i<w*h;i++){
        out.put(buf[i*4+0]);
        out.put(buf[i*4+1]);
        out.put(buf[i*4+2]);
    }
}

void initGL(int W, int H, EGLDisplay& dpy, EGLContext& ctx, EGLSurface& surf) {

    // 1. Get display and initialize
    dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (dpy == EGL_NO_DISPLAY) {
        std::cerr << "eglGetDisplay failed\n";
        exit(1);
    }
    if (!eglInitialize(dpy, nullptr, nullptr)) {
        std::cerr << "eglInitialize failed\n";
        exit(1);
    }

    // 2. Choose EGL config
    EGLConfig cfg;
    EGLint n;
    EGLint cfgAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    if (!eglChooseConfig(dpy, cfgAttribs, &cfg, 1, &n) || n == 0) {
        std::cerr << "eglChooseConfig failed\n";
        exit(1);
    }

    // 3. Bind OpenGL API
    if (!eglBindAPI(EGL_OPENGL_API)) {
        std::cerr << "eglBindAPI failed\n";
        exit(1);
    }

    EGLint ctxAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_NONE
    };
    ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctxAttribs);
    if (ctx == EGL_NO_CONTEXT) {
        std::cerr << "eglCreateContext failed\n";
        exit(1);
    }

    // 5. Create pbuffer surface
    EGLint surfAttribs[] = {
        EGL_WIDTH, W,
        EGL_HEIGHT, H,
        EGL_NONE
    };
    surf = eglCreatePbufferSurface(dpy, cfg, surfAttribs);
    if (surf == EGL_NO_SURFACE) {
        std::cerr << "eglCreatePbufferSurface failed\n";
        exit(1);
    }

    // 6. Make current
    if (!eglMakeCurrent(dpy, surf, surf, ctx)) {
        std::cerr << "eglMakeCurrent failed\n";
        exit(1);
    }
    
    if (!gladLoadGL((GLADloadfunc)eglGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        exit(1);
    }

    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << "\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "Vendor:   " << glGetString(GL_VENDOR) << "\n";
    std::cout << "Version:  " << glGetString(GL_VERSION) << "\n";    
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



int main(int argc,char** argv){
    Args args=parseArgs(argc,argv);
    

    EGLDisplay dpy;
    EGLContext ctx;
    EGLSurface surf;
    initGL(args.width, args.height, dpy, ctx, surf);

    // Build mesh
    Mesh mesh=buildSphere(args.targetVerts);

    // === VAO setup (core profile requires this) ===
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo,ibo;
    glGenBuffers(1,&vbo); glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,mesh.verts.size()*4,mesh.verts.data(),GL_STATIC_DRAW);
    glGenBuffers(1,&ibo); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,mesh.idx.size()*4,mesh.idx.data(),GL_STATIC_DRAW);

    // pass-through vert shader
    // const char* vsrc=R"(#version 330
    //     layout(location=0) in vec3 pos;
    //     layout(location=1) in vec2 uv;
    //     out vec2 vUV;
    //     void main(){ gl_Position=vec4(pos,1.0); vUV=uv; })";

    // debug vertex shader to show triangulation
    const char* vsrc=R"(#version 330
        layout(location=0) in vec3 pos;
        layout(location=1) in vec2 uv;

        flat out vec3 vColor;   // no interpolation

        void main() {
            gl_Position = vec4(pos, 1.0);
            // simple hash of vertex id → color
            float id = float(gl_VertexID % 64) / 64.0;
            vColor = vec3(fract(id*13.0), fract(id*7.0), fract(id*3.0));
        })";

    // debug fragment shader to show triangulation
    std::string fsrc=R"(#version 330
        flat in vec3 vColor;
        out vec4 color;
        void main() {
            color = vec4(vColor, 1.0);
        })";

    // std::string fsrc="#version 330\nin vec2 vUV; out vec4 color;\n"
    //                  "uniform sampler2D tex0;\nvoid main(){ vec4 c=vec4(0.0);\n";
    // for(int i=0;i<args.fragFetches;i++) fsrc+="c+=texture(tex0,vUV);\n";
    // for(int i=0;i<args.fragOps;i++) fsrc+="c=sin(c*1.111+0.1);\n";
    // fsrc+="color=c; }\n";

    // debug: just draw green
    // std::string fsrc=R"(#version 330
    //     out vec4 color;
    //     void main() { color = vec4(0,1,0,1); })";    

    GLuint vs=compileShader(GL_VERTEX_SHADER,vsrc);
    GLuint fs=compileShader(GL_FRAGMENT_SHADER,fsrc.c_str());
    GLuint prog=linkProgram(vs,fs);
    glUseProgram(prog);

    // Simple 1x1 white tex
    GLuint tex; glGenTextures(1,&tex);
    glBindTexture(GL_TEXTURE_2D,tex);
    unsigned char white[4]={255,255,255,255};
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,white);
    glUniform1i(glGetUniformLocation(prog,"tex0"),0);

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

    auto start=std::chrono::high_resolution_clock::now();
    for(int f=0;f<args.frames;f++){
        glDrawElements(GL_TRIANGLES,mesh.idx.size(),GL_UNSIGNED_INT,0);
        glFinish();
    }
    auto end=std::chrono::high_resolution_clock::now();
    double ms=std::chrono::duration<double,std::milli>(end-start).count()/args.frames;

    std::cout<<"Avg frame: "<<ms<<" ms ("<<1000.0/ms<<" FPS)\n";
    double vertsPerSec=(double)mesh.verts.size()/5*(1000.0/ms); // each vertex has 5 floats
    std::cout<<"Throughput: "<<(mesh.verts.size()/5)*(1000.0/ms)/1e6<<" Mverts/sec\n";
    double pixelsPerFrame = (double)args.width * args.height;
    double texFetchesPerSec = pixelsPerFrame * args.fragFetches * (1000.0/ms);
    std::cout << "Tex fetch rate: " << texFetchesPerSec/1e9 << " Gfetches/sec\n";

    if(!args.dumpFile.empty()){
        dumpPPM(args.dumpFile,args.width,args.height);
        std::cout<<"Dumped frame to "<<args.dumpFile<<"\n";
    }
    return 0;
}
