#include "FramePresenter.h"

#include "FrameExchange.h"

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
uniform float uOpacity;
uniform vec4  uRect;    // viewport x, y, w, h in framebuffer pixels
uniform float uRadius;  // corner radius in pixels; 0 = square
out vec4 outColor;
void main() {
    float coverage = 1.0;
    if (uRadius > 0.0) {
        vec2 half_ = uRect.zw * 0.5;
        vec2 p = gl_FragCoord.xy - uRect.xy - half_;
        vec2 q = abs(p) - (half_ - vec2(uRadius));
        float d = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - uRadius;
        coverage = clamp(0.5 - d, 0.0, 1.0);
    }
    outColor = vec4(texture(uTex, vUV).rgb, uOpacity * coverage);
}
)";
} // anon

void FramePresenter::initialize() {
    initializeOpenGLFunctions();
    auto compile = [this](uint32_t type, const char* src) {
        uint32_t s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        int ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[1024]{}; glGetShaderInfoLog(s, sizeof(log), nullptr, log);
            qWarning() << "[presenter] compile fail:" << log;
        }
        return s;
    };
    const uint32_t vs = compile(GL_VERTEX_SHADER, kVert);
    const uint32_t fs = compile(GL_FRAGMENT_SHADER, kFrag);
    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);
    glDeleteShader(vs);
    glDeleteShader(fs);
    locTex_ = glGetUniformLocation(program_, "uTex");
    locOpacity_ = glGetUniformLocation(program_, "uOpacity");
    locRect_ = glGetUniformLocation(program_, "uRect");
    locRadius_ = glGetUniformLocation(program_, "uRadius");

    // Texture row 0 at the top, as macOS shows engine frames (its IOSurface
    // goes to Core Animation, whose origin is top-left); in GL terms that's
    // a vertical flip.
    const float verts[] = {
        -1.f,-1.f, 0.f,1.f,
         1.f,-1.f, 1.f,1.f,
        -1.f, 1.f, 0.f,0.f,
         1.f, 1.f, 1.f,0.f,
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
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void FramePresenter::destroy() {
    if (program_) glDeleteProgram(program_);
    if (vbo_)     glDeleteBuffers(1, &vbo_);
    if (vao_)     glDeleteVertexArrays(1, &vao_);
    program_ = vao_ = vbo_ = 0;
}

bool FramePresenter::draw(FrameExchange* ex, Reader reader, float opacity, float cornerRadiusPx, bool* newFrame) {
    if (newFrame) *newFrame = false;
    if (!ex || !program_) return false;

    void* writeFence = nullptr;
    bool isNew = false;
    const int slot = ex->acquireReadSlot(reader, &writeFence, &isNew);
    if (slot < 0) return false;
    if (isNew && writeFence) glWaitSync(static_cast<GLsync>(writeFence), 0, GL_TIMEOUT_IGNORED);
    if (newFrame) *newFrame = isNew;

    const bool blend = opacity < 1.0f || cornerRadiusPx > 0.0f;
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    if (blend) {
        glEnable(GL_BLEND);
        // Leave destination alpha alone: the widget's framebuffer is
        // composited, so lowering its alpha would let the backdrop through.
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    }
    glUseProgram(program_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ex->texture(slot));
    glUniform1i(locTex_, 0);
    glUniform1f(locOpacity_, opacity);
    glUniform4f(locRect_, float(vp[0]), float(vp[1]), float(vp[2]), float(vp[3]));
    glUniform1f(locRadius_, cornerRadiusPx);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    if (blend) glDisable(GL_BLEND);

    GLsync readFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glFlush();
    if (void* old = ex->releaseReadSlot(reader, slot, readFence)) glDeleteSync(static_cast<GLsync>(old));
    return true;
}
