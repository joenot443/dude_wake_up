#pragma once

#include "FramePresenter.h"

#include <QOpenGLWidget>
#include <QString>
#include <QTimer>
#include <atomic>

// Shows the output of one engine node, repainting at the display's refresh
// rate (the app runs with swap interval 0; see main.cpp). `reader` picks the
// node's FrameExchange reader slot; each concurrently visible view of the
// same node needs its own.
class StageWidget : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit StageWidget(QWidget* parent = nullptr, FramePresenter::Reader reader = FramePresenter::Stage);
    ~StageWidget() override;

    void setSourceNode(const QString& connectableId);
    // Slower repaints for thumbnails (default: the display's refresh rate).
    void setRefreshInterval(int ms) { repaintTimer_.setInterval(ms); }
    QString sourceNode() const { return nodeId_; }

    // Count of distinct engine frames drawn so far.
    quint64 framesShown() const { return framesShown_.load(); }

protected:
    void initializeGL() override;
    void paintGL() override;

private:
    QTimer         repaintTimer_;
    FramePresenter presenter_;
    FramePresenter::Reader reader_;
    QString        nodeId_;
    std::atomic<quint64> framesShown_{0};
};
