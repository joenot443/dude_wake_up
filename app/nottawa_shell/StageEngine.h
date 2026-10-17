#pragma once

#include <QObject>
#include <QThread>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFunctions_3_3_Core>
#include <atomic>
#include <memory>

class StageWidget;

// Placeholder stage engine for Phase A.1 — same dual-context + shared
// texture pattern proven in the Qt spike (spike/qt-of/engine-fbo/).
// Renders a plasma shader into a 1920x1080 FBO on its own thread.
//
// Phase A.2 replaces this with the real Nottawa engine (EngineLoop::tick()
// driving ShaderChainerService::processFrame()). The StageWidget side
// doesn't change — it just reads a different texture id.
class StageEngine : public QThread {
    Q_OBJECT
public:
    StageEngine(StageWidget* widget, QOffscreenSurface* surface);
    ~StageEngine() override;

    void requestStop();

    double  currentFps()      const { return emaFps_.load();     }
    quint64 currentFrame()    const { return frameCount_.load(); }
    double  currentP99Ms()    const { return p99FrameMs_.load(); }

protected:
    void run() override;

private:
    void initGL();
    void renderFrame(float t);
    void teardownGL();

    StageWidget* widget_ = nullptr;

    QOpenGLContext*    ctx_     = nullptr;
    QOffscreenSurface* surface_ = nullptr;
    std::unique_ptr<QOpenGLFunctions_3_3_Core> gl_;

    uint32_t fbo_      = 0;
    uint32_t fboColor_ = 0;
    int      fboW_     = 1920;
    int      fboH_     = 1080;

    uint32_t program_  = 0;
    uint32_t vao_      = 0;
    uint32_t vbo_      = 0;
    int      locTime_  = -1;
    int      locRes_   = -1;

    std::atomic<bool>     stop_{false};
    std::atomic<uint64_t> frameCount_{0};
    std::atomic<double>   emaFps_{0.0};
    std::atomic<double>   p99FrameMs_{0.0};
};
