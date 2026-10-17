#include "StageWidget.h"
#include <QDebug>

namespace {
constexpr const char* kVert = R"(
#version 330 core
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
out vec2 vUV;
void main() { vUV = inUV; gl_Position = vec4(inPos, 0.0, 1.0); }
)";
constexpr const char* kFrag = R"(
#version 330 core
in vec2 vUV;
uniform sampler2D uTex;
out vec4 outColor;
void main() { outColor = texture(uTex, vUV); }
)";

uint32_t compile(QOpenGLFunctions_3_3_Core* gl, uint32_t type, const char* src) {
    uint32_t s = gl->glCreateShader(type);
    gl->glShaderSource(s, 1, &src, nullptr);
    gl->glCompileShader(s);
    int ok = 0; gl->glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024]{}; gl->glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        qWarning() << "[stage-widget] compile fail:" << log;
    }
    return s;
}
uint32_t link(QOpenGLFunctions_3_3_Core* gl, uint32_t vs, uint32_t fs) {
    uint32_t p = gl->glCreateProgram();
    gl->glAttachShader(p, vs); gl->glAttachShader(p, fs);
    gl->glLinkProgram(p);
    gl->glDeleteShader(vs); gl->glDeleteShader(fs);
    return p;
}
} // anon

StageWidget::StageWidget(QWidget* parent) : QOpenGLWidget(parent) {
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
    setMinimumSize(640, 360);
}

StageWidget::~StageWidget() {
    makeCurrent();
    if (program_) glDeleteProgram(program_);
    if (vbo_)     glDeleteBuffers(1, &vbo_);
    if (vao_)     glDeleteVertexArrays(1, &vao_);
    doneCurrent();
}

void StageWidget::initializeGL() {
    initializeOpenGLFunctions();
    program_ = link(this,
                    compile(this, GL_VERTEX_SHADER, kVert),
                    compile(this, GL_FRAGMENT_SHADER, kFrag));
    locTex_ = glGetUniformLocation(program_, "uTex");

    const float verts[] = {
        -1.f,-1.f, 0.f,0.f,
         1.f,-1.f, 1.f,0.f,
        -1.f, 1.f, 0.f,1.f,
         1.f, 1.f, 1.f,1.f,
    };
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glBindVertexArray(0);

    emit glReady();
}

void StageWidget::resizeGL(int w, int h) { glViewport(0, 0, w, h); }

void StageWidget::setEngineTexture(uint32_t id) {
    textureId_.store(id, std::memory_order_release);
    QMetaObject::invokeMethod(this, [this]{ update(); }, Qt::QueuedConnection);
}

void StageWidget::paintGL() {
    const uint32_t tex = textureId_.load(std::memory_order_acquire);
    glClearColor(0.055f, 0.059f, 0.075f, 1.f);  // #0e0f13 — matches Theme bg
    glClear(GL_COLOR_BUFFER_BIT);
    if (tex == 0) return;
    glUseProgram(program_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(locTex_, 0);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
