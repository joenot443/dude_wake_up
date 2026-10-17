#include "StageWidget.h"

#include "EngineBridge.h"
#include "FrameExchange.h"

#include <QDebug>
#include <QOpenGLContext>
#include <QScreen>
#include <QtGui/qopenglcontext_platform.h>

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
        qWarning() << "[stage] compile fail:" << log;
    }
    return s;
}
} // anon

StageWidget::StageWidget(QWidget* parent) : QOpenGLWidget(parent) {
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
    setMinimumSize(640, 360);
    // Swap interval is 0 (see main.cpp), so pace repaints here.
    repaintTimer_.setTimerType(Qt::PreciseTimer);
    connect(&repaintTimer_, &QTimer::timeout, this, [this] { update(); });
    const qreal hz = screen() ? screen()->refreshRate() : 60.0;
    // Round the interval down so no engine frame is skipped.
    repaintTimer_.start(qMax(1, int(1000.0 / (hz > 0 ? hz : 60.0))));
}

StageWidget::~StageWidget() {
    makeCurrent();
    if (program_) glDeleteProgram(program_);
    if (vbo_)     glDeleteBuffers(1, &vbo_);
    if (vao_)     glDeleteVertexArrays(1, &vao_);
    doneCurrent();
}

void StageWidget::setSourceNode(const QString& connectableId) {
    if (connectableId == nodeId_) return;
    auto* engine = EngineBridge::instance();
    if (!nodeId_.isEmpty()) engine->stopSharing(nodeId_);
    nodeId_ = connectableId;
    if (!nodeId_.isEmpty()) engine->startSharing(nodeId_);
    update();
}

void StageWidget::initializeGL() {
    initializeOpenGLFunctions();
    if (auto* wgl = context()->nativeInterface<QNativeInterface::QWGLContext>())
        nativeContext_ = wgl->nativeContext();

    const uint32_t vs = compile(this, GL_VERTEX_SHADER, kVert);
    const uint32_t fs = compile(this, GL_FRAGMENT_SHADER, kFrag);
    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);
    glDeleteShader(vs);
    glDeleteShader(fs);
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

void StageWidget::paintGL() {
    glClearColor(0.055f, 0.059f, 0.075f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    FrameExchange* ex = nodeId_.isEmpty() ? nullptr : EngineBridge::instance()->frames(nodeId_);
    if (!ex) return;

    void* writeFence = nullptr;
    const int slot = ex->acquireReadSlot(&writeFence);
    if (slot < 0) return;
    if (writeFence) {
        glWaitSync(static_cast<GLsync>(writeFence), 0, GL_TIMEOUT_IGNORED);
        glDeleteSync(static_cast<GLsync>(writeFence));
        framesShown_.fetch_add(1);
    }

    glUseProgram(program_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ex->texture(slot));
    glUniform1i(locTex_, 0);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    GLsync readFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glFlush();
    if (void* old = ex->releaseReadSlot(slot, readFence)) glDeleteSync(static_cast<GLsync>(old));
}
