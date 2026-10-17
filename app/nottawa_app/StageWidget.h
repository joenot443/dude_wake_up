#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QString>
#include <QTimer>
#include <atomic>
#include <cstdint>

// Shows the output of one engine node. Frames arrive through the engine's
// FrameExchange for that node; the widget repaints at the display's refresh rate.
class StageWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    explicit StageWidget(QWidget* parent = nullptr);
    ~StageWidget() override;

    void setSourceNode(const QString& connectableId);
    QString sourceNode() const { return nodeId_; }

    // Native GL context (HGLRC) captured in initializeGL; valid after glReady.
    void* nativeContext() const { return nativeContext_; }

    // Count of distinct engine frames drawn so far.
    quint64 framesShown() const { return framesShown_.load(); }

signals:
    void glReady();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QTimer   repaintTimer_;
    QString  nodeId_;
    void*    nativeContext_ = nullptr;
    uint32_t program_ = 0;
    uint32_t vao_     = 0;
    uint32_t vbo_     = 0;
    int      locTex_  = -1;
    std::atomic<quint64> framesShown_{0};
};
