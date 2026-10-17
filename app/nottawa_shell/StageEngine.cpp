#include "StageEngine.h"
#include "StageWidget.h"

#include <QDebug>
#include <QElapsedTimer>
#include <algorithm>
#include <cstring>

namespace {
constexpr const char* kVert = R"(
#version 330 core
layout(location = 0) in vec2 inPos;
void main() { gl_Position = vec4(inPos, 0.0, 1.0); }
)";
constexpr const char* kFrag = R"(
#version 330 core
uniform float uTime;
uniform vec2  uRes;
out vec4 outColor;
void main() {
    vec2 uv = gl_FragCoord.xy / uRes;
    float r = 0.5 + 0.5 * sin(10.0 * uv.x + uTime);
    float g = 0.5 + 0.5 * sin(10.0 * uv.y + uTime * 1.3);
    float b = 0.5 + 0.5 * sin(10.0 * (uv.x + uv.y) + uTime * 0.7);
    outColor = vec4(r, g, b, 1.0);
}
)";

uint32_t compile(QOpenGLFunctions_3_3_Core* gl, uint32_t type, const char* src) {
    uint32_t s = gl->glCreateShader(type);
    gl->glShaderSource(s, 1, &src, nullptr);
    gl->glCompileShader(s);
    int ok = 0; gl->glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]{}; gl->glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        qWarning() << "[stage] compile fail:" << log;
    }
    return s;
}
uint32_t link(QOpenGLFunctions_3_3_Core* gl, uint32_t vs, uint32_t fs) {
    uint32_t p = gl->glCreateProgram();
    gl->glAttachShader(p, vs); gl->glAttachShader(p, fs);
    gl->glLinkProgram(p);
    int ok = 0; gl->glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024]{}; gl->glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        qWarning() << "[stage] link fail:" << log;
    }
    gl->glDeleteShader(vs); gl->glDeleteShader(fs);
    return p;
}
} // anon

StageEngine::StageEngine(StageWidget* widget, QOffscreenSurface* surface)
  : widget_(widget), surface_(surface) {}

StageEngine::~StageEngine() {
    requestStop();
    wait(2000);
}

void StageEngine::requestStop() { stop_.store(true); }

void StageEngine::run() {
    ctx_ = new QOpenGLContext;
    ctx_->setFormat(QSurfaceFormat::defaultFormat());
    ctx_->setShareContext(QOpenGLContext::globalShareContext());
    if (!ctx_->create()) { qCritical() << "[stage] ctx create failed"; return; }

    if (!surface_ || !surface_->isValid()) {
        qCritical() << "[stage] surface invalid"; return;
    }
    if (!ctx_->makeCurrent(surface_)) {
        qCritical() << "[stage] makeCurrent failed"; return;
    }

    gl_ = std::make_unique<QOpenGLFunctions_3_3_Core>();
    gl_->initializeOpenGLFunctions();
    qInfo() << "[stage] GL:"
            << reinterpret_cast<const char*>(gl_->glGetString(GL_RENDERER));

    initGL();

    QElapsedTimer wall; wall.start();
    const qint64 startMs = wall.elapsed();

    constexpr int kN = 128;
    double samples[kN]{};
    int    idx = 0;
    qint64 lastNs = wall.nsecsElapsed();

    while (!stop_.load()) {
        const qint64 now = wall.elapsed();
        const float  t   = (now - startMs) / 1000.f;

        renderFrame(t);

        const qint64 nowNs   = wall.nsecsElapsed();
        const double frameMs = (nowNs - lastNs) / 1'000'000.0;
        lastNs = nowNs;
        samples[idx] = frameMs; idx = (idx + 1) % kN;

        const uint64_t fc = frameCount_.load();
        if ((fc & 31) == 0 && fc > kN) {
            double s[kN]; std::memcpy(s, samples, sizeof(s));
            std::sort(std::begin(s), std::end(s));
            double sum = 0; for (double v : s) sum += v;
            emaFps_.store(1000.0 / (sum / kN));
            p99FrameMs_.store(s[(kN * 99) / 100]);
        }

        const qint64 slack = 16 - static_cast<qint64>(frameMs);
        if (slack > 0) msleep(static_cast<unsigned long>(slack));
    }

    teardownGL();
    delete ctx_; ctx_ = nullptr;
}

void StageEngine::initGL() {
    gl_->glGenTextures(1, &fboColor_);
    gl_->glBindTexture(GL_TEXTURE_2D, fboColor_);
    gl_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fboW_, fboH_, 0, GL_RGBA,
                      GL_UNSIGNED_BYTE, nullptr);
    gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl_->glBindTexture(GL_TEXTURE_2D, 0);

    gl_->glGenFramebuffers(1, &fbo_);
    gl_->glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                GL_TEXTURE_2D, fboColor_, 0);
    if (gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        qCritical() << "[stage] FBO incomplete";
    gl_->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    program_ = link(gl_.get(),
                    compile(gl_.get(), GL_VERTEX_SHADER,   kVert),
                    compile(gl_.get(), GL_FRAGMENT_SHADER, kFrag));
    locTime_ = gl_->glGetUniformLocation(program_, "uTime");
    locRes_  = gl_->glGetUniformLocation(program_, "uRes");

    const float v[] = { -1.f,-1.f, 1.f,-1.f, -1.f,1.f, 1.f,1.f };
    gl_->glGenVertexArrays(1, &vao_);
    gl_->glGenBuffers(1, &vbo_);
    gl_->glBindVertexArray(vao_);
    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    gl_->glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    gl_->glEnableVertexAttribArray(0);
    gl_->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), nullptr);
    gl_->glBindVertexArray(0);
}

void StageEngine::renderFrame(float t) {
    frameCount_.fetch_add(1);
    gl_->glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    gl_->glViewport(0, 0, fboW_, fboH_);
    gl_->glUseProgram(program_);
    gl_->glUniform1f(locTime_, t);
    gl_->glUniform2f(locRes_, (float)fboW_, (float)fboH_);
    gl_->glBindVertexArray(vao_);
    gl_->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    gl_->glBindVertexArray(0);
    gl_->glFlush();
    gl_->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    widget_->setEngineTexture(fboColor_);
}

void StageEngine::teardownGL() {
    if (!gl_) return;
    if (vbo_)      gl_->glDeleteBuffers(1, &vbo_);
    if (vao_)      gl_->glDeleteVertexArrays(1, &vao_);
    if (program_)  gl_->glDeleteProgram(program_);
    if (fbo_)      gl_->glDeleteFramebuffers(1, &fbo_);
    if (fboColor_) gl_->glDeleteTextures(1, &fboColor_);
    ctx_->doneCurrent();
}
