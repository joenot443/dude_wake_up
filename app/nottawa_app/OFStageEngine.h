#pragma once

#include <QThread>
#include <QObject>
#include <QString>
#include <atomic>
#include <cstdint>

class StageWidget;
struct GLFWwindow;
namespace of_impl { struct State; }

// Set once at startup — directory holding Nottawa .vert/.frag files.
// Simple module-level setter so main.cpp can resolve it from the exe
// path without the engine class growing a construction parameter.
void setNottawaShadersDir(const QString& absPath);

// OFStageEngine — Phase A.2 engine thread. The dual-context pattern
// from the spike + OpenFrameworks on top:
//
//   - Main thread: Qt's QOpenGLContext (owned by StageWidget).
//   - Engine thread: owns a HIDDEN GLFW window + its WGL context so
//     OpenFrameworks has a native GL context it understands. We call
//     `wglShareLists(qtHglrc, glfwHglrc)` so that OF-created textures
//     (ofFbo attachments) are visible from Qt's context — same zero-copy
//     handoff proven in spike/qt-of/engine-fbo/, just with GLFW instead
//     of a second QOpenGLContext.
//
// Phase A.2.a goal: prove this plumbing works end-to-end with ONE OF
// render primitive (ofFbo + ofShader) producing a visible frame in Qt.
// A.2.b wires EngineLoop + the Nottawa services on top.
class OFStageEngine : public QThread {
    Q_OBJECT
public:
    // qtHglrc is HGLRC captured on the main thread while Qt's context
    // was current (wglGetCurrentContext()). Needed so we can call
    // wglShareLists from the engine thread.
    OFStageEngine(StageWidget* widget, void* qtHglrc);
    ~OFStageEngine() override;

    void requestStop();

    double  currentFps()   const { return emaFps_.load();     }
    quint64 currentFrame() const { return frameCount_.load(); }
    double  currentP99Ms() const { return p99FrameMs_.load(); }

signals:
    // Emitted once with the OF/GLFW version string so the UI can show
    // "OpenFrameworks 0.12.1 loaded" in the log on startup. Confirms
    // the lib really linked.
    void engineInfo(QString msg);

    // Fatal error message if OF/GLFW init blew up. UI shows this in
    // the log; the stage will stay dark.
    void engineError(QString msg);

protected:
    void run() override;

private:
    bool initGLFWContext();
    bool shareWithQt();
    bool initOF();
    void renderFrame(float t);
    void teardown();

    StageWidget*  widget_   = nullptr;
    void*         qtHglrc_  = nullptr;

    GLFWwindow*   window_   = nullptr;
    std::unique_ptr<of_impl::State> of_;

    std::atomic<bool>     stop_{false};
    std::atomic<uint64_t> frameCount_{0};
    std::atomic<double>   emaFps_{0.0};
    std::atomic<double>   p99FrameMs_{0.0};
};

// Forward decl needs the full type to compile the unique_ptr dtor — the
// out-of-line destructor in OFStageEngine.cpp sees the full definition.
#include <memory>
