#pragma once

#include "FramePresenter.h"

#include <QOpenGLWidget>
#include <QTimer>

// Base for the 16:9 transform pads: a GL widget that can draw an engine
// frame into any rect of itself and repaints at 30 fps.
class FramePad : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit FramePad(QWidget* parent = nullptr);
    ~FramePad() override;

    // Shares (and shows) the given connectable's frames; empty for none.
    void setFrameSource(const QString& connectableId);

    bool hasHeightForWidth() const override { return true; }
    int  heightForWidth(int w) const override { return w * 9 / 16; }
    QSize sizeHint() const override { return QSize(240, 135); }
    QSize minimumSizeHint() const override { return QSize(64, 36); }

protected:
    void initializeGL() override;
    // Draws the frame into `rect` (widget coordinates); needs a QPainter
    // active on this widget.
    bool drawFrame(QPainter& p, const QRectF& rect);
    static void paintHandle(QPainter& p, QPointF at);

    QString frameSource_;

private:
    FramePresenter presenter_;
    QTimer         repaintTimer_;
};

// The crop rectangle over the transform's input (after the macOS
// CropRectPadView): drag the body, edges or corners. Normalised, y down.
class CropPad : public FramePad {
    Q_OBJECT
public:
    explicit CropPad(QWidget* parent = nullptr);
    void setCrop(float minX, float maxX, float minY, float maxY);  // ignored mid-drag
    bool isDragging() const { return mode_ != None; }

signals:
    void cropChanged(float minX, float maxX, float minY, float maxY);

protected:
    void paintGL() override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    enum Mode { None, Body, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };
    QRectF rectInWidget() const;
    Mode   hitTest(QPointF p) const;

    float minX_ = 0, maxX_ = 1, minY_ = 0, maxY_ = 1;
    Mode  mode_ = None;
    QPointF pressPos_;
    float startMinX_ = 0, startMaxX_ = 1, startMinY_ = 0, startMaxY_ = 1;
};

// Where the cropped image lands in the output (after the macOS
// TransformPlacePadView): drag to move, corners to scale.
class PlacePad : public FramePad {
    Q_OBJECT
public:
    explicit PlacePad(QWidget* parent = nullptr);
    void setPlacement(float x, float y, float scale, float cropW, float cropH);  // ignored mid-drag
    bool isDragging() const { return mode_ != None; }

signals:
    void placementChanged(float x, float y, float scale);

protected:
    void paintGL() override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    enum Mode { None, Body, TopLeft, TopRight, BottomLeft, BottomRight };
    QRectF rectInWidget() const;

    float x_ = 0, y_ = 0, scale_ = 1, cropW_ = 1, cropH_ = 1;
    Mode  mode_ = None;
    QPointF pressPos_;
    float startX_ = 0, startY_ = 0, startScale_ = 1;
};
