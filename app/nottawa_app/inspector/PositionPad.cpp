#include "PositionPad.h"

#include "EngineBridge.h"
#include "Theme.h"

#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

PositionPad::PositionPad(const QString& connectableId, QWidget* parent)
    : QOpenGLWidget(parent), nodeId_(connectableId) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setCursor(Qt::CrossCursor);
    EngineBridge::instance()->retainFrames(nodeId_);
    repaintTimer_.setInterval(33);
    connect(&repaintTimer_, &QTimer::timeout, this, [this] { if (isVisible()) update(); });
    repaintTimer_.start();
}

PositionPad::~PositionPad() {
    makeCurrent();
    presenter_.destroy();
    doneCurrent();
    EngineBridge::instance()->releaseFrames(nodeId_);
}

void PositionPad::setPosition(float x, float y) {
    if (dragging_) return;
    x_ = x; y_ = y;
}

void PositionPad::initializeGL() { presenter_.initialize(); }

void PositionPad::paintGL() {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath frame;
    frame.addRoundedRect(r, 6, 6);
    p.fillRect(rect(), Theme::backgroundSecondary);
    p.fillPath(frame, Theme::surface);

    if (FrameExchange* ex = EngineBridge::instance()->frames(nodeId_)) {
        p.beginNativePainting();
        const qreal dpr = devicePixelRatioF();
        context()->functions()->glViewport(0, 0, int(width() * dpr), int(height() * dpr));
        presenter_.draw(ex, FramePresenter::InspectorPad, 1.0f, float(6 * dpr));
        p.endNativePainting();
        p.fillPath(frame, QColor(0, 0, 0, 77));  // dim so the handle stands out
    }

    p.setClipPath(frame);
    p.setPen(QPen(QColor(255, 255, 255, 20), 0.5));
    for (int i = 1; i < 4; ++i) p.drawLine(QPointF(r.left() + r.width() * i / 4, r.top()), QPointF(r.left() + r.width() * i / 4, r.bottom()));
    for (int i = 1; i < 3; ++i) p.drawLine(QPointF(r.left(), r.top() + r.height() * i / 3), QPointF(r.right(), r.top() + r.height() * i / 3));
    const QPointF pt(r.left() + x_ * r.width(), r.top() + y_ * r.height());
    p.setPen(QPen(QColor(255, 255, 255, 90), 0.5));
    p.drawLine(QPointF(pt.x(), r.top()), QPointF(pt.x(), r.bottom()));
    p.drawLine(QPointF(r.left(), pt.y()), QPointF(r.right(), pt.y()));
    p.setClipping(false);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 128));
    p.drawEllipse(pt + QPointF(0, 1), 7, 7);
    p.setBrush(Theme::accent);
    p.drawEllipse(pt, 6, 6);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(Theme::borderSubtle, 1));
    p.drawPath(frame);
}

void PositionPad::setFromMouse(QPointF pos) {
    x_ = std::clamp(float(pos.x() / std::max(1, width())), 0.0f, 1.0f);
    y_ = std::clamp(float(pos.y() / std::max(1, height())), 0.0f, 1.0f);
    update();
    emit moved(x_, y_);
}

void PositionPad::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;
    dragging_ = true;
    setFromMouse(e->position());
}

void PositionPad::mouseMoveEvent(QMouseEvent* e) {
    if (dragging_) setFromMouse(e->position());
}

void PositionPad::mouseReleaseEvent(QMouseEvent*) { dragging_ = false; }
