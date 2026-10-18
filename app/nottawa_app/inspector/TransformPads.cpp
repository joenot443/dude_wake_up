#include "TransformPads.h"

#include "EngineBridge.h"
#include "Theme.h"

#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <cmath>

namespace {
constexpr qreal kRadius = 6;
constexpr qreal kHandle = 10;
constexpr qreal kSlop = kHandle / 2 + 8;
constexpr float kMinCrop = 0.02f;
constexpr float kMinScale = 0.05f;

float clampf(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }
}

// ---------------------------------------------------------------- FramePad

FramePad::FramePad(QWidget* parent) : QOpenGLWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    repaintTimer_.setInterval(33);
    connect(&repaintTimer_, &QTimer::timeout, this, [this] { if (isVisible()) update(); });
    repaintTimer_.start();
}

FramePad::~FramePad() {
    makeCurrent();
    presenter_.destroy();
    doneCurrent();
    if (!frameSource_.isEmpty()) EngineBridge::instance()->releaseFrames(frameSource_);
}

void FramePad::setFrameSource(const QString& id) {
    if (id == frameSource_) return;
    if (!frameSource_.isEmpty()) EngineBridge::instance()->releaseFrames(frameSource_);
    frameSource_ = id;
    if (!id.isEmpty()) EngineBridge::instance()->retainFrames(id);
}

void FramePad::initializeGL() { presenter_.initialize(); }

bool FramePad::drawFrame(QPainter& p, const QRectF& rect) {
    if (frameSource_.isEmpty() || rect.width() < 1 || rect.height() < 1) return false;
    FrameExchange* ex = EngineBridge::instance()->frames(frameSource_);
    if (!ex) return false;
    const qreal dpr = devicePixelRatioF();
    p.beginNativePainting();
    QOpenGLFunctions* gl = context()->functions();
    // Clip to the widget (the rect may hang off it).
    gl->glEnable(GL_SCISSOR_TEST);
    gl->glScissor(0, 0, int(width() * dpr), int(height() * dpr));
    gl->glViewport(int(std::lround(rect.left() * dpr)), int(std::lround((height() - rect.bottom()) * dpr)),
                   int(std::lround(rect.width() * dpr)), int(std::lround(rect.height() * dpr)));
    const bool ok = presenter_.draw(ex, FramePresenter::InspectorPad);
    gl->glDisable(GL_SCISSOR_TEST);
    gl->glViewport(0, 0, int(width() * dpr), int(height() * dpr));
    p.endNativePainting();
    return ok;
}

void FramePad::paintHandle(QPainter& p, QPointF at) {
    const QRectF r(at.x() - kHandle / 2, at.y() - kHandle / 2, kHandle, kHandle);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 110));
    p.drawRect(r.translated(0, 1));
    p.setBrush(Theme::accent);
    p.setPen(QPen(Qt::white, 1));
    p.drawRect(r);
}

// ----------------------------------------------------------------- CropPad

CropPad::CropPad(QWidget* parent) : FramePad(parent) {}

void CropPad::setCrop(float minX, float maxX, float minY, float maxY) {
    if (mode_ != None) return;
    minX_ = minX; maxX_ = maxX; minY_ = minY; maxY_ = maxY;
}

QRectF CropPad::rectInWidget() const {
    return QRectF(minX_ * width(), minY_ * height(), std::max(0.0f, maxX_ - minX_) * width(),
                  std::max(0.0f, maxY_ - minY_) * height());
}

void CropPad::paintGL() {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Theme::backgroundSecondary);
    QPainterPath pad;
    pad.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), kRadius, kRadius);
    p.fillPath(pad, Theme::surface);
    drawFrame(p, rect());

    const QRectF r = rectInWidget();
    QPainterPath outside = pad;
    outside.addRect(r);
    outside.setFillRule(Qt::OddEvenFill);
    p.fillPath(outside, QColor(0, 0, 0, 140));
    if (r.width() > 4 && r.height() > 4) {
        p.setPen(QPen(QColor(255, 255, 255, 46), 0.5));
        for (int i = 1; i < 3; ++i) {
            p.drawLine(QPointF(r.left() + r.width() * i / 3, r.top()), QPointF(r.left() + r.width() * i / 3, r.bottom()));
            p.drawLine(QPointF(r.left(), r.top() + r.height() * i / 3), QPointF(r.right(), r.top() + r.height() * i / 3));
        }
    }
    p.setPen(QPen(Theme::accent, 1.5));
    p.setBrush(Qt::NoBrush);
    p.drawRect(r);
    p.setRenderHint(QPainter::Antialiasing, false);
    for (const QPointF& at : {r.topLeft(), r.topRight(), r.bottomLeft(), r.bottomRight(),
                              QPointF(r.left(), r.center().y()), QPointF(r.right(), r.center().y()),
                              QPointF(r.center().x(), r.top()), QPointF(r.center().x(), r.bottom())})
        paintHandle(p, at);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Theme::border, 0.5));
    p.setBrush(Qt::NoBrush);
    p.drawPath(pad);
}

CropPad::Mode CropPad::hitTest(QPointF pt) const {
    const QRectF r = rectInWidget();
    const struct { Mode mode; QPointF at; } corners[] = {
        {TopLeft, r.topLeft()}, {TopRight, r.topRight()}, {BottomLeft, r.bottomLeft()}, {BottomRight, r.bottomRight()},
    };
    for (const auto& c : corners)
        if (std::hypot(c.at.x() - pt.x(), c.at.y() - pt.y()) <= kSlop) return c.mode;
    const bool inY = pt.y() > r.top() - kSlop && pt.y() < r.bottom() + kSlop;
    const bool inX = pt.x() > r.left() - kSlop && pt.x() < r.right() + kSlop;
    if (std::abs(pt.x() - r.left()) <= kSlop && inY) return Left;
    if (std::abs(pt.x() - r.right()) <= kSlop && inY) return Right;
    if (std::abs(pt.y() - r.top()) <= kSlop && inX) return Top;
    if (std::abs(pt.y() - r.bottom()) <= kSlop && inX) return Bottom;
    return Body;
}

void CropPad::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;
    mode_ = hitTest(e->position());
    pressPos_ = e->position();
    startMinX_ = minX_; startMaxX_ = maxX_; startMinY_ = minY_; startMaxY_ = maxY_;
}

void CropPad::mouseMoveEvent(QMouseEvent* e) {
    if (mode_ == None) {
        static const Qt::CursorShape cursors[] = {Qt::ArrowCursor, Qt::SizeAllCursor, Qt::SizeHorCursor, Qt::SizeHorCursor,
                                                  Qt::SizeVerCursor, Qt::SizeVerCursor, Qt::SizeFDiagCursor,
                                                  Qt::SizeBDiagCursor, Qt::SizeBDiagCursor, Qt::SizeFDiagCursor};
        setCursor(cursors[hitTest(e->position())]);
        return;
    }
    const float dx = float((e->position().x() - pressPos_.x()) / std::max(1, width()));
    const float dy = float((e->position().y() - pressPos_.y()) / std::max(1, height()));
    float minX = startMinX_, maxX = startMaxX_, minY = startMinY_, maxY = startMaxY_;
    const bool left = mode_ == Left || mode_ == TopLeft || mode_ == BottomLeft;
    const bool right = mode_ == Right || mode_ == TopRight || mode_ == BottomRight;
    const bool top = mode_ == Top || mode_ == TopLeft || mode_ == TopRight;
    const bool bottom = mode_ == Bottom || mode_ == BottomLeft || mode_ == BottomRight;
    if (mode_ == Body) {
        const float w = maxX - minX, h = maxY - minY;
        minX = clampf(startMinX_ + dx, 0, 1 - w); maxX = minX + w;
        minY = clampf(startMinY_ + dy, 0, 1 - h); maxY = minY + h;
    }
    if (left) minX = clampf(startMinX_ + dx, 0, startMaxX_ - kMinCrop);
    if (right) maxX = clampf(startMaxX_ + dx, startMinX_ + kMinCrop, 1);
    if (top) minY = clampf(startMinY_ + dy, 0, startMaxY_ - kMinCrop);
    if (bottom) maxY = clampf(startMaxY_ + dy, startMinY_ + kMinCrop, 1);
    minX_ = minX; maxX_ = maxX; minY_ = minY; maxY_ = maxY;
    update();
    emit cropChanged(minX, maxX, minY, maxY);
}

void CropPad::mouseReleaseEvent(QMouseEvent*) { mode_ = None; }

// ---------------------------------------------------------------- PlacePad

PlacePad::PlacePad(QWidget* parent) : FramePad(parent) {}

void PlacePad::setPlacement(float x, float y, float scale, float cropW, float cropH) {
    cropW_ = cropW; cropH_ = cropH;
    if (mode_ != None) return;
    x_ = x; y_ = y; scale_ = scale;
}

QRectF PlacePad::rectInWidget() const {
    return QRectF(x_ * width(), y_ * height(), std::max(0.0f, cropW_ * scale_) * width(),
                  std::max(0.0f, cropH_ * scale_) * height());
}

void PlacePad::paintGL() {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Theme::backgroundSecondary);
    QPainterPath pad;
    pad.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), kRadius, kRadius);
    p.fillPath(pad, Theme::surface);
    const QRectF r = rectInWidget();
    drawFrame(p, r);
    p.setClipPath(pad);
    p.setPen(QPen(Theme::accent, 1.5));
    p.setBrush(Qt::NoBrush);
    p.drawRect(r);
    p.setRenderHint(QPainter::Antialiasing, false);
    for (const QPointF& at : {r.topLeft(), r.topRight(), r.bottomLeft(), r.bottomRight()}) paintHandle(p, at);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipping(false);
    p.setPen(QPen(Theme::border, 0.5));
    p.setBrush(Qt::NoBrush);
    p.drawPath(pad);
}

void PlacePad::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;
    const QRectF r = rectInWidget();
    const QPointF pt = e->position();
    mode_ = Body;
    const struct { Mode mode; QPointF at; } corners[] = {
        {TopLeft, r.topLeft()}, {TopRight, r.topRight()}, {BottomLeft, r.bottomLeft()}, {BottomRight, r.bottomRight()},
    };
    for (const auto& c : corners)
        if (std::hypot(c.at.x() - pt.x(), c.at.y() - pt.y()) <= kSlop) { mode_ = c.mode; break; }
    pressPos_ = pt;
    startX_ = x_; startY_ = y_; startScale_ = scale_;
}

// The macOS maths: corners scale uniformly (by whichever axis moved more)
// with the opposite corner pinned.
void PlacePad::mouseMoveEvent(QMouseEvent* e) {
    if (mode_ == None) return;
    const float dx = float((e->position().x() - pressPos_.x()) / std::max(1, width()));
    const float dy = float((e->position().y() - pressPos_.y()) / std::max(1, height()));
    const float cw = std::max(0.001f, cropW_), ch = std::max(0.001f, cropH_);
    const float origW = cw * startScale_, origH = ch * startScale_;
    auto scaleFor = [&](float newW, float newH) {
        return clampf(std::max(std::max(0.0f, newW) / cw, std::max(0.0f, newH) / ch), kMinScale, 5.0f);
    };
    switch (mode_) {
    case Body:
        x_ = clampf(startX_ + dx, -2, 2);
        y_ = clampf(startY_ + dy, -2, 2);
        break;
    case BottomRight:
        scale_ = scaleFor(origW + dx, origH + dy);
        break;
    case TopLeft:
        scale_ = scaleFor(origW - dx, origH - dy);
        x_ = startX_ + origW - cw * scale_;
        y_ = startY_ + origH - ch * scale_;
        break;
    case TopRight:
        scale_ = scaleFor(origW + dx, origH - dy);
        y_ = startY_ + origH - ch * scale_;
        break;
    case BottomLeft:
        scale_ = scaleFor(origW - dx, origH + dy);
        x_ = startX_ + origW - cw * scale_;
        break;
    case None:
        return;
    }
    update();
    emit placementChanged(x_, y_, scale_);
}

void PlacePad::mouseReleaseEvent(QMouseEvent*) { mode_ = None; }
