#pragma once

#include <QMutex>
#include <QString>
#include <QStringList>
#include <QThread>
#include <atomic>
#include <cstdint>
#include <memory>

class StageWidget;
namespace of_impl { struct State; }

void setNottawaShadersDir(const QString& absPath);
QString nottawaShadersDir();
// Every shader in the shaders dir that has both a .vert and a .frag.
QStringList nottawaShaderNames();
// NOTTAWA_SHADER env var, else "Accretion".
QString nottawaDefaultShader();

// Measured over the most recent one-second window.
struct EngineStats {
    double  fps           = 0;
    double  intervalP99Ms = 0;
    double  intervalMaxMs = 0;
    double  gpuAvgMs      = 0;   // GL_TIME_ELAPSED around the shader pass
    double  gpuP99Ms      = 0;
    quint64 hitches       = 0;   // cumulative frame intervals > 33.3 ms
    quint64 frames        = 0;
};

// Renders Nottawa shaders with OpenFrameworks on its own thread and hands
// finished frames to StageWidget through FrameExchange.
//
// Context setup: a bare GLFW context is created first and joined to Qt's via
// wglShareLists while both are still empty (WGL refuses otherwise), then
// OF's window is created with shareContextWith pointing at it.
//
// Env: NOTTAWA_UNCAPPED=1 disables 60 Hz pacing; NOTTAWA_BENCH_ALL=1 cycles
// every shader for 1.5 s, reports per-shader GPU cost, then emits
// benchFinished().
class OFStageEngine : public QThread {
    Q_OBJECT
public:
    OFStageEngine(StageWidget* widget, void* qtHglrc);
    ~OFStageEngine() override;

    void requestStop();
    EngineStats stats() const;

public slots:
    void requestShader(const QString& name);
    void setBrightness(double value);

signals:
    void engineInfo(QString msg);
    void engineError(QString msg);
    void benchFinished();

protected:
    void run() override;

private:
    bool createShareRoot();
    bool shareWithQt();
    bool initOF();
    bool switchShader(const QString& name);
    void applyPendingShader();
    void renderFrame(float t);
    void collectGpuTime();
    void teardown();

    StageWidget* widget_  = nullptr;
    void*        qtHglrc_ = nullptr;
    std::unique_ptr<of_impl::State> of_;

    const bool uncapped_;
    const bool benchAll_;

    std::atomic<bool>   stop_{false};
    std::atomic<double> brightness_{400.0};

    QMutex  pendingMutex_;
    QString pendingShader_;
    bool    pendingSet_ = false;

    mutable QMutex statsMutex_;
    EngineStats    stats_;
};
