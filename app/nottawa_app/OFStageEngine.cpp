// IMPORTANT include order: ofMain MUST precede anything that pulls in
// <GL/gl.h> (via Qt or windows.h), because OF pulls in <GL/glew.h>
// which refuses to compile if gl.h was seen first.
#include "ofMain.h"
#include "ofAppGLFWWindow.h"
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

// A tiny shim letting us pass a RAW GLFWwindow* into OF's
// `shareContextWith` setting. OF reads it via
// `getWindowContext()` and uses it as the `share` parameter when it
// internally calls glfwCreateWindow. The other virtuals are stubs
// — OF never invokes them on a share-only handle.
class BareShareHandle : public ofAppBaseWindow {
public:
    explicit BareShareHandle(GLFWwindow* w) : raw_(w) {}
    void* getWindowContext() override { return raw_; }

    // Pure-virtual stubs. OF only reads getWindowContext() from the
    // share-handle; these never run.
    void  setup(const ofWindowSettings&) override {}
    void  update() override {}
    void  draw() override {}
    ofCoreEvents& events() override { return events_; }
    std::shared_ptr<ofBaseRenderer>& renderer() override { return renderer_; }
private:
    GLFWwindow*                      raw_;
    ofCoreEvents                     events_;
    std::shared_ptr<ofBaseRenderer>  renderer_;
};

// Engine state. The pivotal order is:
//   1) create bareGLFW (empty)
//   2) wglShareLists(qtHglrc, bareGLFW) — WGL rule says both must be
//      empty at share time, and they are
//   3) ofCreateWindow with shareContextWith = bareShim(bareGLFW) —
//      OF's new window B shares with bareGLFW, so Qt sees everything OF
//      creates in B via the bareGLFW bridge.
struct State {
    GLFWwindow* bareGLFW   = nullptr;      // the empty share-root
    std::shared_ptr<BareShareHandle> shim;
    std::shared_ptr<ofAppBaseWindow> ofWindow;

    ofFbo      fbo;
    ofShader   shader;
    ofMesh     quad;
    ofTexture  inputTex;   // 1x1 white dummy for the shader's `tex` uniform

    static constexpr int W = 1920;
    static constexpr int H = 1080;
};

} // namespace of_impl

namespace {

// A.2.c — load an actual Nottawa shader from the repo's shaders/
// directory via ofShader. Confirms Nottawa's shader content runs
// through our OF+Qt pipeline unchanged.
constexpr const char* kShaderName = "Accretion";

// Repo-root shaders dir, discovered relative to the exe. Set by main()
// before engine starts.
QString& shadersDir() { static QString s; return s; }

} // anon

void setNottawaShadersDir(const QString& absPath) { shadersDir() = absPath; }

OFStageEngine::OFStageEngine(StageWidget* widget, void* qtHglrc)
  : widget_(widget), qtHglrc_(qtHglrc) {}

OFStageEngine::~OFStageEngine() {
    requestStop();
    wait(2000);
}

void OFStageEngine::requestStop() { stop_.store(true); }

bool OFStageEngine::initGLFWContext() {
    // Step 1 — create a BARE GLFW context, no objects in it yet.
    // This is the share-root that both Qt and OF will link against.
    of_ = std::make_unique<of_impl::State>();

    if (!glfwInit()) {
        emit engineError("glfwInit failed");
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    of_->bareGLFW = glfwCreateWindow(16, 16, "nottawa-share-root",
                                     nullptr, nullptr);
    if (!of_->bareGLFW) {
        emit engineError("bare glfwCreateWindow failed");
        return false;
    }
    // Make current so wglGetCurrentContext inside shareWithQt resolves
    // to our HGLRC via glfwGetWGLContext(bareGLFW).
    glfwMakeContextCurrent(of_->bareGLFW);
    window_ = of_->bareGLFW;
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
    // Step 3 — hand OF a shareContextWith pointing at the bare GLFW
    // window we just linked to Qt. OF's new window B shares with it,
    // so B's objects (including ofFbo textures) are visible to Qt.
    of_->shim = std::make_shared<of_impl::BareShareHandle>(of_->bareGLFW);

    ofGLFWWindowSettings settings;
    settings.setGLVersion(3, 3);
    settings.setSize(64, 64);
    settings.visible         = false;
    settings.decorated       = false;
    settings.resizable       = false;
    settings.shareContextWith = of_->shim;

    of_->ofWindow = ofCreateWindow(settings);
    if (!of_->ofWindow) {
        emit engineError("ofCreateWindow failed");
        return false;
    }
    auto* glfwWin = dynamic_cast<ofAppGLFWWindow*>(of_->ofWindow.get());
    if (glfwWin && glfwWin->getGLFWWindow()) {
        glfwHideWindow(glfwWin->getGLFWWindow());
        glfwSetWindowRefreshCallback(glfwWin->getGLFWWindow(), nullptr);
    }

    ofDisableArbTex();  // Force GL_TEXTURE_2D — matches Nottawa's main app.

    ofFboSettings fboSettings;
    fboSettings.width          = of_impl::State::W;
    fboSettings.height         = of_impl::State::H;
    fboSettings.internalformat = GL_RGBA8;
    fboSettings.textureTarget  = GL_TEXTURE_2D;
    fboSettings.useDepth       = false;
    fboSettings.useStencil     = false;
    of_->fbo.allocate(fboSettings);
    if (!of_->fbo.isAllocated()) {
        emit engineError("ofFbo::allocate failed");
        return false;
    }

    // Load the real Nottawa shader from shaders/<name>.vert + .frag.
    // Can't use ofShader::load() because OF prepends the data path; we
    // use the project's shader dir directly via ofBufferFromFile +
    // setupShaderFromSource.
    const QString vertPath = shadersDir() + "/" + kShaderName + ".vert";
    const QString fragPath = shadersDir() + "/" + kShaderName + ".frag";

    ofBuffer vertBuf = ofBufferFromFile(vertPath.toStdString(), false);
    ofBuffer fragBuf = ofBufferFromFile(fragPath.toStdString(), false);
    if (vertBuf.size() == 0 || fragBuf.size() == 0) {
        emit engineError(QString("shader files missing: %1 / %2").arg(vertPath, fragPath));
        return false;
    }

    if (!of_->shader.setupShaderFromSource(GL_VERTEX_SHADER,   vertBuf.getText())) {
        emit engineError(QString("vert compile failed: %1").arg(vertPath));
        return false;
    }
    if (!of_->shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragBuf.getText())) {
        emit engineError(QString("frag compile failed: %1").arg(fragPath));
        return false;
    }
    of_->shader.bindDefaults();   // Binds `position`+`texcoord` so ofMesh.draw() routes correctly.
    if (!of_->shader.linkProgram()) {
        emit engineError("ofShader::linkProgram failed");
        return false;
    }

    // 1x1 white dummy input texture — Accretion samples it but its
    // output doesn't depend on content. A.2.d will wire a real source.
    unsigned char whitePx[4] = {255, 255, 255, 255};
    of_->inputTex.allocate(1, 1, GL_RGBA);
    of_->inputTex.loadData(whitePx, 1, 1, GL_RGBA);

    // Fullscreen quad in CLIP SPACE (-1..1). With Accretion.vert's
    // `gl_Position = modelViewProjectionMatrix * position`, if MVP is
    // identity (what you get when calling mesh.draw() with no renderer-
    // managed matrices), clip-space in == clip-space out → fullscreen.
    // Nottawa's engine normally pushes an ortho projection matching
    // the FBO, but that lives in services we haven't bootstrapped yet;
    // clip-space is the right fit for A.2.c.
    of_->quad.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    of_->quad.addVertex(glm::vec3(-1,-1, 0)); of_->quad.addTexCoord(glm::vec2(0, 0));
    of_->quad.addVertex(glm::vec3( 1,-1, 0)); of_->quad.addTexCoord(glm::vec2(1, 0));
    of_->quad.addVertex(glm::vec3(-1, 1, 0)); of_->quad.addTexCoord(glm::vec2(0, 1));
    of_->quad.addVertex(glm::vec3( 1, 1, 0)); of_->quad.addTexCoord(glm::vec2(1, 1));

    emit engineInfo(QString("OF ready — renderer=%1 ofFbo=%2x%3 texId=%4 shader=%5 (real Nottawa shader)")
                    .arg(reinterpret_cast<const char*>(glGetString(GL_RENDERER)))
                    .arg(of_impl::State::W)
                    .arg(of_impl::State::H)
                    .arg(of_->fbo.getTexture().getTextureData().textureID)
                    .arg(kShaderName));
    return true;
}

void OFStageEngine::renderFrame(float t) {
    frameCount_.fetch_add(1);
    // Default OF_FBOMODE_PERSPECTIVE|MATRIXFLIP: OF pushes an ortho
    // projection matching the FBO size, so ofTexture::draw(0, 0, W, H)
    // becomes a fullscreen quad with correct MVP for Nottawa's
    // vertex shader (`gl_Position = modelViewProjectionMatrix * position`).
    of_->fbo.begin();
    ofClear(0, 0, 0, 255);
    of_->shader.begin();
    of_->shader.setUniform1f("time",       t);
    of_->shader.setUniform2f("dimensions",
                             static_cast<float>(of_impl::State::W),
                             static_cast<float>(of_impl::State::H));
    of_->shader.setUniform1f("brightness", 400.0f);   // matches Nottawa's AccretionSettings default
    of_->shader.setUniformTexture("tex", of_->inputTex, 0);
    // Mimic Nottawa's shade() pattern — use ofTexture::draw to produce
    // a fullscreen textured quad through the active shader.
    of_->inputTex.draw(0, 0, of_impl::State::W, of_impl::State::H);
    of_->shader.end();
    of_->fbo.end();
    glFlush();
    widget_->setEngineTexture(of_->fbo.getTexture().getTextureData().textureID);
}

void OFStageEngine::teardown() {
    if (of_) {
        of_->shader.unload();
        of_->fbo.clear();
        of_->quad.clear();
        of_->ofWindow.reset();   // destroys OF's GLFW window B
        if (of_->bareGLFW) {
            glfwDestroyWindow(of_->bareGLFW);
            of_->bareGLFW = nullptr;
        }
        of_.reset();
    }
    window_ = nullptr;
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
