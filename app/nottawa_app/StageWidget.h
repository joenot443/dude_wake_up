#pragma once

#include "FrameExchange.h"

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <atomic>
#include <cstdint>

// Displays the engine's newest finished frame. The engine publishes frames
// into exchange() from its own thread and calls notifyFrameReady().
class StageWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    explicit StageWidget(QWidget* parent = nullptr);
    ~StageWidget() override;

    FrameExchange& exchange() { return exchange_; }
    void notifyFrameReady();

    // Count of distinct engine frames actually drawn by the UI.
    quint64 framesShown() const { return framesShown_.load(); }

signals:
    void glReady();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    FrameExchange exchange_;
    uint32_t program_ = 0;
    uint32_t vao_     = 0;
    uint32_t vbo_     = 0;
    int      locTex_  = -1;
    std::atomic<bool>    updatePending_{false};
    std::atomic<quint64> framesShown_{0};
};
