#include "PositionPad.h"

#include "EngineBridge.h"
#include "Theme.h"

#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPainterPath>
#include <QSurfaceFormat>
#include <algorithm>

PositionPad::PositionPad(const QString& connectableId, QWidget* parent)
    : QOpenGLWidget(parent), nodeId_(connectableId) {
    // Multisampled, so the painter's thin lines and the handle are
    // antialiased (without it 0.5 px lines drop out between pixels).
    QSurfaceFormat fmt = format();
    fmt.setSamples(4);
    setFormat(fmt);
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
        presenter_.setViewport(0, 0, int(width() * dpr), int(height() * dpr));
        presenter_.draw(ex, FramePresenter::InspectorPad, 1.0f, float(6 * dpr));
        p.endNativePainting();
        p.fillPath(frame, QColor(0, 0, 0, 77));  // dim so the handle stands out
    }

    // As the macOS PositionPadView: a 4x3 grid at 8% white, a 35% crosshair
    // through the handle, the accent handle with a shadow and white ring,
    // and a hairline border.
    p.setClipPath(frame);
    p.setPen(QPen(QColor(255, 255, 255, 20), 0.5));
    for (int i = 1; i < 4; ++i) p.drawLine(QPointF(r.left() + r.width() * i / 4, r.top()), QPointF(r.left() + r.width() * i / 4, r.bottom()));
    for (int i = 1; i < 3; ++i) p.drawLine(QPointF(r.left(), r.top() + r.height() * i / 3), QPointF(r.right(), r.top() + r.height() * i / 3));
    const QPointF pt(r.left() + x_ * r.width(), r.top() + y_ * r.height());
    p.setPen(QPen(QColor(255, 255, 255, 89), 0.5));
    p.drawLine(QPointF(pt.x(), r.top()), QPointF(pt.x(), r.bottom()));
    p.drawLine(QPointF(r.left(), pt.y()), QPointF(r.right(), pt.y()));
    p.setClipping(false);
    p.setPen(Qt::NoPen);
    for (int i = 3; i >= 1; --i) {  // soft shadow, 50% black, 1 px down
        p.setBrush(QColor(0, 0, 0, 40));
        p.drawEllipse(pt + QPointF(0, 1), 6.0 + i, 6.0 + i);
    }
    p.setBrush(Theme::accent);
    p.drawEllipse(pt, 6, 6);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(Qt::white, 1.5));
    p.drawEllipse(pt, 7.25, 7.25);
    p.setPen(QPen(Theme::border, 0.5));
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
