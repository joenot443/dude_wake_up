// IMPORTANT include order: ofMain MUST precede anything that pulls in
// <GL/gl.h> (via Qt or windows.h), because OF pulls in <GL/glew.h>
// which refuses to compile if gl.h was seen first.
#include "ofMain.h"
#include "ofGLProgrammableRenderer.h"
#include "ofFbo.h"
#include "ofShader.h"

// Now the Windows+GLFW native layer for wglShareLists. GLFW's header
// must be included BEFORE glfw3native.h, and WIN32 defines (HGLRC etc)
// must exist first.
#include <windows.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

// Finally Qt (QDebug / QElapsedTimer only — nothing GL from Qt here).
#include "OFStageEngine.h"
#include "StageWidget.h"
#include <QDebug>
#include <QElapsedTimer>
#include <algorithm>
#include <cstring>
#include <memory>

namespace of_impl {

// Engine state. A.2.a keeps this bag of raw-GL handles; A.2.b will add
// ofFbo + ofShader + ofGLProgrammableRenderer alongside, once OF is
// properly initialized via ofCreateWindow.
struct State {
    GLuint rawFbo  = 0;
    GLuint rawTex  = 0;
    GLuint rawProg = 0;
    GLuint rawVao  = 0;
    GLuint rawVbo  = 0;
    GLint  locTime = -1;
    GLint  locRes  = -1;

    static constexpr int W = 1920;
    static constexpr int H = 1080;
};

} // namespace of_impl

namespace {

// Minimal plasma shader expressed through OF's shader API. Lets us
// prove ofShader works end-to-end under Qt.
constexpr const char* kVertSrc = R"(
#version 330 core
in vec2 position;
void main() { gl_Position = vec4(position, 0.0, 1.0); }
)";

constexpr const char* kFragSrc = R"(
#version 330 core
uniform float uTime;
uniform vec2  uRes;
out vec4 fragColor;
void main() {
    vec2 uv = gl_FragCoord.xy / uRes;
    // Different color pattern than nottawa_shell's plasma so we can
    // visually tell them apart.
    float r = 0.5 + 0.5 * sin(uTime         + uv.x * 6.0);
    float g = 0.5 + 0.5 * sin(uTime * 0.7   + uv.y * 6.0);
    float b = 0.5 + 0.5 * sin(uTime * 1.3   + (uv.x + uv.y) * 4.0);
    fragColor = vec4(r, g, b, 1.0);
}
)";

} // anon

OFStageEngine::OFStageEngine(StageWidget* widget, void* qtHglrc)
  : widget_(widget), qtHglrc_(qtHglrc) {}

OFStageEngine::~OFStageEngine() {
    requestStop();
    wait(2000);
}

void OFStageEngine::requestStop() { stop_.store(true); }

bool OFStageEngine::initGLFWContext() {
    if (!glfwInit()) {
        emit engineError("glfwInit failed");
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window_ = glfwCreateWindow(64, 64, "nottawa-engine", nullptr, nullptr);
    if (!window_) {
        emit engineError("glfwCreateWindow failed");
        return false;
    }
    glfwMakeContextCurrent(window_);
    return true;
}

bool OFStageEngine::shareWithQt() {
    // Hand Qt's HGLRC and ours to WGL to merge object namespaces.
    // Modern NVIDIA/AMD drivers accept this call even when objects
    // already exist in either context.
    HGLRC engineHglrc = glfwGetWGLContext(window_);
    HGLRC qtH        = reinterpret_cast<HGLRC>(qtHglrc_);
    if (!qtH || !engineHglrc) {
        emit engineError("missing HGLRC — Qt or GLFW context not available");
        return false;
    }
    if (!wglShareLists(qtH, engineHglrc)) {
        DWORD err = GetLastError();
        emit engineError(QString("wglShareLists failed (GetLastError=%1)").arg(err));
        return false;
    }
    return true;
}

bool OFStageEngine::initOF() {
    // Phase A.2.a — prove the GLFW↔Qt context-sharing plumbing works
    // end-to-end with RAW GL (via GLEW, which OF brings in). Full OF
    // init (ofCreateWindow + ofFbo/ofShader) requires the ofRunApp
    // path and lands in A.2.b. OF is linked and headers are reachable
    // from this translation unit — the lib build is validated.
    of_ = std::make_unique<of_impl::State>();

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        emit engineError(QString("glewInit failed: %1")
                         .arg(reinterpret_cast<const char*>(glewGetErrorString(err))));
        return false;
    }

    // Raw-GL FBO + color texture — the color texture IS what Qt reads.
    glGenTextures(1, &of_->rawTex);
    glBindTexture(GL_TEXTURE_2D, of_->rawTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, of_impl::State::W, of_impl::State::H, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &of_->rawFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, of_->rawFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, of_->rawTex, 0);
    GLenum st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (st != GL_FRAMEBUFFER_COMPLETE) {
        emit engineError(QString("FBO incomplete: 0x%1").arg(st, 0, 16));
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Shader program.
    auto compile = [](GLenum type, const char* src)->GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        return s;
    };
    GLuint vs = compile(GL_VERTEX_SHADER,   kVertSrc);
    GLuint fs = compile(GL_FRAGMENT_SHADER, kFragSrc);
    of_->rawProg = glCreateProgram();
    glAttachShader(of_->rawProg, vs);
    glAttachShader(of_->rawProg, fs);
    glBindAttribLocation(of_->rawProg, 0, "position");
    glLinkProgram(of_->rawProg);
    GLint ok = 0;
    glGetProgramiv(of_->rawProg, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024]{};
        glGetProgramInfoLog(of_->rawProg, sizeof(log), nullptr, log);
        emit engineError(QString("link failed: %1").arg(log));
        return false;
    }
    glDeleteShader(vs); glDeleteShader(fs);
    of_->locTime = glGetUniformLocation(of_->rawProg, "uTime");
    of_->locRes  = glGetUniformLocation(of_->rawProg, "uRes");

    // Fullscreen-quad geometry.
    const float verts[] = { -1.f,-1.f, 1.f,-1.f, -1.f,1.f, 1.f,1.f };
    glGenVertexArrays(1, &of_->rawVao);
    glGenBuffers(1, &of_->rawVbo);
    glBindVertexArray(of_->rawVao);
    glBindBuffer(GL_ARRAY_BUFFER, of_->rawVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), nullptr);
    glBindVertexArray(0);

    emit engineInfo(QString("raw-GL ready — renderer=%1 fbo=%2x%3 texId=%4 — OF lib linked (ofFbo wiring: A.2.b)")
                    .arg(reinterpret_cast<const char*>(glGetString(GL_RENDERER)))
                    .arg(of_impl::State::W)
                    .arg(of_impl::State::H)
                    .arg(of_->rawTex));
    return true;
}

void OFStageEngine::renderFrame(float t) {
    frameCount_.fetch_add(1);
    glBindFramebuffer(GL_FRAMEBUFFER, of_->rawFbo);
    glViewport(0, 0, of_impl::State::W, of_impl::State::H);
    glUseProgram(of_->rawProg);
    glUniform1f(of_->locTime, t);
    glUniform2f(of_->locRes, (float)of_impl::State::W, (float)of_impl::State::H);
    glBindVertexArray(of_->rawVao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glFlush();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    widget_->setEngineTexture(of_->rawTex);
}

void OFStageEngine::teardown() {
    if (of_) {
        if (of_->rawVbo)  glDeleteBuffers(1, &of_->rawVbo);
        if (of_->rawVao)  glDeleteVertexArrays(1, &of_->rawVao);
        if (of_->rawProg) glDeleteProgram(of_->rawProg);
        if (of_->rawFbo)  glDeleteFramebuffers(1, &of_->rawFbo);
        if (of_->rawTex)  glDeleteTextures(1, &of_->rawTex);
        of_.reset();
    }
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}

void OFStageEngine::run() {
    emit engineInfo("run() entered");
    if (!initGLFWContext()) { emit engineError("initGLFWContext failed"); return; }
    emit engineInfo("GLFW context created");
    if (!shareWithQt())     { teardown(); return; }
    emit engineInfo("wglShareLists ok — Qt & GLFW share texture namespace");
    if (!initOF())          { teardown(); return; }

    // Render loop — same cadence pattern as nottawa_shell's placeholder.
    QElapsedTimer wall; wall.start();
    const qint64 startMs = wall.elapsed();
    constexpr int kN = 128;
    double samples[kN]{};
    int idx = 0;
    qint64 lastNs = wall.nsecsElapsed();

    while (!stop_.load()) {
        const float t = (wall.elapsed() - startMs) / 1000.f;
        renderFrame(t);
        const qint64 nowNs = wall.nsecsElapsed();
        const double frameMs = (nowNs - lastNs) / 1'000'000.0;
        lastNs = nowNs;
        samples[idx] = frameMs; idx = (idx + 1) % kN;
        const uint64_t fc = frameCount_.load();
        if ((fc & 31) == 0 && fc > kN) {
            double s[kN]; std::memcpy(s, samples, sizeof(s));
            std::sort(std::begin(s), std::end(s));
            double sum = 0; for (double v : s) sum += v;
            emaFps_.store(1000.0 / (sum / kN));
            p99FrameMs_.store(s[(kN*99)/100]);
        }
        const qint64 slack = 16 - static_cast<qint64>(frameMs);
        if (slack > 0) msleep(static_cast<unsigned long>(slack));
    }
    teardown();
}
