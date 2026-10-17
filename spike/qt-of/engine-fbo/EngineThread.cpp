#include "EngineThread.h"
#include "EngineWidget.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>
#include <QOffscreenSurface>
#include <algorithm>
#include <cstring>

namespace {

// A tiny plasma — enough that we can see it moving on screen and know
// the engine thread is actually ticking, not just drawing a frozen
// first frame.
constexpr const char* kVertSrc = R"(
#version 330 core
layout(location = 0) in vec2 inPos;
void main() { gl_Position = vec4(inPos, 0.0, 1.0); }
)";

constexpr const char* kFragSrc = R"(
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

uint32_t compileShader(QOpenGLFunctions_3_3_Core* gl, uint32_t type, const char* src) {
    uint32_t s = gl->glCreateShader(type);
    gl->glShaderSource(s, 1, &src, nullptr);
    gl->glCompileShader(s);
    int ok = 0; gl->glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]{}; gl->glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        qWarning() << "[engine] shader compile fail:" << log;
    }
    return s;
}
uint32_t linkProgram(QOpenGLFunctions_3_3_Core* gl, uint32_t vs, uint32_t fs) {
    uint32_t p = gl->glCreateProgram();
    gl->glAttachShader(p, vs);
    gl->glAttachShader(p, fs);
    gl->glLinkProgram(p);
    int ok = 0; gl->glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024]{}; gl->glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        qWarning() << "[engine] program link fail:" << log;
    }
    gl->glDeleteShader(vs); gl->glDeleteShader(fs);
    return p;
}

} // anon

EngineThread::EngineThread(EngineWidget* widget, QOffscreenSurface* surface)
  : widget_(widget), surface_(surface) {}

EngineThread::~EngineThread() {
    requestStop();
    wait(2000);
}

void EngineThread::requestStop() { stop_ = true; }

double   EngineThread::currentFps()        const { return emaFps_.load();      }
quint64  EngineThread::currentFrame()      const { return frameCount_.load();  }
double   EngineThread::currentP99FrameMs() const { return p99FrameMs_.load();  }

void EngineThread::run() {
    // 1) Engine QOpenGLContext sharing with the Qt-global share context
    //    (installed by AA_ShareOpenGLContexts). The share link is what
    //    lets our FBO's texture id be valid in the UI widget's context.
    ctx_ = new QOpenGLContext;
    ctx_->setFormat(QSurfaceFormat::defaultFormat());
    ctx_->setShareContext(QOpenGLContext::globalShareContext());
    if (!ctx_->create()) {
        qCritical() << "[engine] QOpenGLContext::create failed";
        return;
    }

    // 2) Surface was created on the GUI thread (Qt requirement) and
    //    handed to us at construction. We just use it.
    if (!surface_ || !surface_->isValid()) {
        qCritical() << "[engine] offscreen surface missing/invalid";
        return;
    }

    if (!ctx_->makeCurrent(surface_)) {
        qCritical() << "[engine] makeCurrent failed";
        return;
    }

    gl_ = std::make_unique<QOpenGLFunctions_3_3_Core>();
    gl_->initializeOpenGLFunctions();

    qInfo() << "[engine] GL:"
            << reinterpret_cast<const char*>(gl_->glGetString(GL_VENDOR))
            << reinterpret_cast<const char*>(gl_->glGetString(GL_RENDERER))
            << reinterpret_cast<const char*>(gl_->glGetString(GL_VERSION));

    initGL_();

    // 3) Render loop — spin as fast as the driver + v-wait allows,
    //    cap at 60Hz. No glFinish() per frame (that collapsed fps on
    //    Day 2); glFlush() is enough to kick the command stream, the
    //    driver synchronizes shared textures automatically.
    QElapsedTimer wallClock;
    wallClock.start();
    const qint64 startMs = wallClock.elapsed();

    // Rolling frame-time window for the p99 stat the UI reads.
    constexpr int kWindow = 128;
    double frameTimesMs[kWindow]{};
    int    frameTimeIdx = 0;

    qint64 lastFrameNs = wallClock.nsecsElapsed();

    while (!stop_.load()) {
        const qint64 now  = wallClock.elapsed();
        const float  t    = (now - startMs) / 1000.f;

        renderFrame_(t);

        const qint64 nowNs    = wallClock.nsecsElapsed();
        const double frameMs  = (nowNs - lastFrameNs) / 1'000'000.0;
        lastFrameNs           = nowNs;

        frameTimesMs[frameTimeIdx] = frameMs;
        frameTimeIdx = (frameTimeIdx + 1) % kWindow;

        // Update stats periodically (every 32 frames).
        const uint64_t fc = frameCount_.load();
        if ((fc & 31) == 0 && fc > kWindow) {
            // Simple EMA fps + p99 frame ms from the ring buffer.
            double sortedMs[kWindow];
            std::memcpy(sortedMs, frameTimesMs, sizeof(sortedMs));
            std::sort(std::begin(sortedMs), std::end(sortedMs));
            const double p99 = sortedMs[(kWindow * 99) / 100];
            double sum = 0; for (double v : sortedMs) sum += v;
            const double avgMs = sum / kWindow;
            emaFps_.store(1000.0 / avgMs);
            p99FrameMs_.store(p99);
        }

        // 60Hz cap: aim at 16.6ms per frame, but if GPU is already
        // slow (frameMs > 16) we don't sleep.
        const qint64 slackMs = 16 - static_cast<qint64>(frameMs);
        if (slackMs > 0) msleep(static_cast<unsigned long>(slackMs));
    }

    teardownGL_();
    delete ctx_; ctx_ = nullptr;
    // surface_ is owned by main's deleter — don't double-free.
}

void EngineThread::initGL_() {
    // Create the FBO + its color attachment. The color texture is what
    // the UI widget reads — share-context makes its id valid there too.
    gl_->glGenTextures(1, &fboColor_);
    gl_->glBindTexture(GL_TEXTURE_2D, fboColor_);
    gl_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fboW_, fboH_, 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl_->glBindTexture(GL_TEXTURE_2D, 0);

    gl_->glGenFramebuffers(1, &fbo_);
    gl_->glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                GL_TEXTURE_2D, fboColor_, 0);
    GLenum status = gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        qCritical() << "[engine] FBO incomplete:" << Qt::hex << status;
    }
    gl_->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Fullscreen program.
    uint32_t vs = compileShader(gl_.get(), GL_VERTEX_SHADER, kVertSrc);
    uint32_t fs = compileShader(gl_.get(), GL_FRAGMENT_SHADER, kFragSrc);
    program_ = linkProgram(gl_.get(), vs, fs);
    locTime_ = gl_->glGetUniformLocation(program_, "uTime");
    locRes_  = gl_->glGetUniformLocation(program_, "uRes");

    const float verts[] = { -1.f,-1.f,  1.f,-1.f,  -1.f,1.f,  1.f,1.f };
    gl_->glGenVertexArrays(1, &vao_);
    gl_->glGenBuffers(1, &vbo_);
    gl_->glBindVertexArray(vao_);
    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    gl_->glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    gl_->glEnableVertexAttribArray(0);
    gl_->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    gl_->glBindVertexArray(0);

    qInfo() << "[engine] initGL done — fbo=" << fbo_ << " color=" << fboColor_
            << " size=" << fboW_ << "x" << fboH_;
}

void EngineThread::renderFrame_(float t) {
    frameCount_.fetch_add(1);
    gl_->glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    gl_->glViewport(0, 0, fboW_, fboH_);
    gl_->glUseProgram(program_);
    gl_->glUniform1f(locTime_, t);
    gl_->glUniform2f(locRes_, static_cast<float>(fboW_), static_cast<float>(fboH_));
    gl_->glBindVertexArray(vao_);
    gl_->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    gl_->glBindVertexArray(0);

    // glFlush is enough — the driver synchronizes shared textures
    // across contexts automatically. glFinish per-frame tanked fps on
    // Day 2 (~34fps instead of 60) because it blocks the CPU.
    gl_->glFlush();
    gl_->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    widget_->setEngineTexture(fboColor_);
}

void EngineThread::teardownGL_() {
    if (!gl_) return;
    if (vbo_)      gl_->glDeleteBuffers(1, &vbo_);
    if (vao_)      gl_->glDeleteVertexArrays(1, &vao_);
    if (program_)  gl_->glDeleteProgram(program_);
    if (fbo_)      gl_->glDeleteFramebuffers(1, &fbo_);
    if (fboColor_) gl_->glDeleteTextures(1, &fboColor_);
    ctx_->doneCurrent();
}
