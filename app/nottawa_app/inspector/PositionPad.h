#pragma once

#include "FramePresenter.h"

#include <QOpenGLWidget>
#include <QTimer>

// A 16:9 pad for dragging an X/Y pair, after the macOS PositionPadView: the
// node's live output (dimmed) behind a grid, crosshair and handle.
// Positions are normalised 0..1 with y = 0 at the top.
class PositionPad : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit PositionPad(const QString& connectableId, QWidget* parent = nullptr);
    ~PositionPad() override;

    void setPosition(float x, float y);  // ignored mid-drag
    bool isDragging() const { return dragging_; }

    bool hasHeightForWidth() const override { return true; }
    int  heightForWidth(int w) const override { return w * 9 / 16; }
    QSize sizeHint() const override { return QSize(240, 135); }

signals:
    void moved(float x, float y);

protected:
    void initializeGL() override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    void setFromMouse(QPointF pos);

    QString        nodeId_;
    FramePresenter presenter_;
    QTimer         repaintTimer_;
    float          x_ = 0.5f, y_ = 0.5f;
    bool           dragging_ = false;
};
