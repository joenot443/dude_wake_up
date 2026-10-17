#pragma once

#include <QObject>
#include <QThread>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFunctions_3_3_Core>
#include <memory>

class EngineWidget;

// EngineThread — the "engine" side of the dual-context setup. Lives on
// its own QThread. Owns a QOpenGLContext created with the widget's
// context as the share-context, plus a QOffscreenSurface so the engine
// context has somewhere to be current.
//
// On a 16ms timer (effective 60Hz), it renders a procedural shader into
// a 1024x576 FBO, glFinish()es, then tells the EngineWidget "new frame,
// texture id = X". Because AA_ShareOpenGLContexts is on, the widget's
// context sees exactly that texture.
class EngineThread : public QThread {
    Q_OBJECT
public:
    // The offscreen surface MUST be created on the GUI thread (Qt rule).
    // Owner creates it, passes ownership here.
    EngineThread(EngineWidget* widget, QOffscreenSurface* surface);
    ~EngineThread() override;

    void requestStop();

    // Live stats the UI samples at 1Hz for the FPS overlay.
    double currentFps()      const;
    quint64 currentFrame()   const;
    double currentP99FrameMs() const;

protected:
    void run() override;

private:
    void initGL_();
    void renderFrame_(float t);
    void teardownGL_();

    EngineWidget* widget_ = nullptr;

    QOpenGLContext*    ctx_      = nullptr;
    QOffscreenSurface* surface_  = nullptr;   // owned by us but allocated on GUI thread

    // Our tiny GL subset.
    std::unique_ptr<QOpenGLFunctions_3_3_Core> gl_;

    uint32_t fbo_        = 0;
    uint32_t fboColor_   = 0;      // the shared texture the UI reads
    int      fboW_       = 1920;
    int      fboH_       = 1080;

    uint32_t program_    = 0;
    uint32_t vao_        = 0;
    uint32_t vbo_        = 0;
    int      locTime_    = -1;
    int      locRes_     = -1;

    std::atomic<bool>     stop_{false};
    std::atomic<uint64_t> frameCount_{0};

    // Rolling stats — written by engine thread, read by UI.
    std::atomic<double>   emaFps_{0.0};
    std::atomic<double>   p99FrameMs_{0.0};
};
