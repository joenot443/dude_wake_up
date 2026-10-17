#pragma once

#include "FramePresenter.h"

#include <QOpenGLWidget>
#include <QString>
#include <QTimer>
#include <atomic>

// Shows the output of one engine node, repainting at the display's refresh
// rate (the app runs with swap interval 0; see main.cpp).
class StageWidget : public QOpenGLWidget {
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
    void paintGL() override;

private:
    QTimer         repaintTimer_;
    FramePresenter presenter_;
    QString        nodeId_;
    void*          nativeContext_ = nullptr;
    std::atomic<quint64> framesShown_{0};
};
