#include "EngineWidget.h"

#include <QDebug>
#include <QOpenGLContext>
#include <QDateTime>

namespace {
constexpr const char* kVertSrc = R"(
#version 330 core
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
out vec2 vUV;
void main() {
    vUV = inUV;
    gl_Position = vec4(inPos, 0.0, 1.0);
}
)";

constexpr const char* kFragSrc = R"(
#version 330 core
in vec2 vUV;
uniform sampler2D uTex;
out vec4 outColor;
void main() {
    outColor = texture(uTex, vUV);
}
)";

uint32_t compileShader(QOpenGLFunctions_3_3_Core* gl, uint32_t type, const char* src) {
    uint32_t s = gl->glCreateShader(type);
    gl->glShaderSource(s, 1, &src, nullptr);
    gl->glCompileShader(s);
    int ok = 0; gl->glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]{}; gl->glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        qWarning() << "shader compile fail:" << log;
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
        qWarning() << "program link fail:" << log;
    }
    gl->glDeleteShader(vs); gl->glDeleteShader(fs);
    return p;
}
} // anon

EngineWidget::EngineWidget(QWidget* parent) : QOpenGLWidget(parent) {
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
    setMinimumSize(640, 360);
}

EngineWidget::~EngineWidget() {
    makeCurrent();
    if (program_) glDeleteProgram(program_);
    if (vbo_)     glDeleteBuffers(1, &vbo_);
    if (vao_)     glDeleteVertexArrays(1, &vao_);
    doneCurrent();
}

void EngineWidget::initializeGL() {
    initializeOpenGLFunctions();

    qInfo() << "EngineWidget GL:" << reinterpret_cast<const char*>(glGetString(GL_VENDOR))
            << reinterpret_cast<const char*>(glGetString(GL_RENDERER))
            << reinterpret_cast<const char*>(glGetString(GL_VERSION));

    // Fullscreen-quad program.
    uint32_t vs = compileShader(this, GL_VERTEX_SHADER, kVertSrc);
    uint32_t fs = compileShader(this, GL_FRAGMENT_SHADER, kFragSrc);
    program_ = linkProgram(this, vs, fs);
    locTex_  = glGetUniformLocation(program_, "uTex");

    // Fullscreen-quad geometry. {pos.x,pos.y, uv.x,uv.y}.
    const float verts[] = {
        -1.f, -1.f,  0.f, 0.f,
         1.f, -1.f,  1.f, 0.f,
        -1.f,  1.f,  0.f, 1.f,
         1.f,  1.f,  1.f, 1.f,
    };
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    qInfo() << "EngineWidget initializeGL done, about to emit glReady";
    emit glReady();
    qInfo() << "EngineWidget glReady emitted";
}

void EngineWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void EngineWidget::setEngineTexture(uint32_t texId) {
    textureId_.store(texId, std::memory_order_release);
    QMetaObject::invokeMethod(this, [this]() { update(); }, Qt::QueuedConnection);
}

void EngineWidget::paintGL() {
    const uint32_t tex = textureId_.load(std::memory_order_acquire);

    glClearColor(0.08f, 0.08f, 0.10f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (tex == 0) return;  // engine hasn't drawn anything yet

    glUseProgram(program_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(locTex_, 0);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    const uint64_t n = paintCount_.fetch_add(1, std::memory_order_relaxed) + 1;
    if ((n & 59) == 0) {
        // Log once per second-ish so we can see the pipeline is live.
        qInfo() << "UI paint" << n << "tex=" << tex;
    }
}
