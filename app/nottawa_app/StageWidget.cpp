#include "StageWidget.h"

#include "EngineBridge.h"

#include <cmath>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QScreen>

StageWidget::StageWidget(QWidget* parent, FramePresenter::Reader reader)
    : QOpenGLWidget(parent), reader_(reader) {
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
    repaintTimer_.setTimerType(Qt::PreciseTimer);
    connect(&repaintTimer_, &QTimer::timeout, this, [this] { if (isVisible()) update(); });
    const qreal hz = screen() ? screen()->refreshRate() : 60.0;
    // Round the interval down so no engine frame is skipped.
    repaintTimer_.start(qMax(1, int(1000.0 / (hz > 0 ? hz : 60.0))));
}

StageWidget::~StageWidget() {
    if (!nodeId_.isEmpty()) EngineBridge::instance()->releaseFrames(nodeId_);
    if (!outgoing_.isEmpty()) EngineBridge::instance()->releaseFrames(outgoing_);
    makeCurrent();
    presenter_.destroy();
    doneCurrent();
}

void StageWidget::setSourceNode(const QString& connectableId) {
    if (connectableId == nodeId_) return;
    auto* engine = EngineBridge::instance();
    if (!outgoing_.isEmpty()) { engine->releaseFrames(outgoing_); outgoing_.clear(); }
    if (!nodeId_.isEmpty()) engine->releaseFrames(nodeId_);
    nodeId_ = connectableId;
    if (!nodeId_.isEmpty()) engine->retainFrames(nodeId_);
    update();
}

void StageWidget::crossfadeTo(const QString& connectableId, int ms) {
    if (connectableId == nodeId_) return;
    if (ms <= 0 || nodeId_.isEmpty()) { setSourceNode(connectableId); return; }
    auto* engine = EngineBridge::instance();
    if (!outgoing_.isEmpty()) engine->releaseFrames(outgoing_);
    outgoing_ = nodeId_;  // keeps its retain until the fade ends
    nodeId_ = connectableId;
    if (!nodeId_.isEmpty()) engine->retainFrames(nodeId_);
    fadeMs_ = ms;
    fade_.start();
    update();
}

void StageWidget::initializeGL() {
    presenter_.initialize();
}

void StageWidget::paintGL() {
    QOpenGLFunctions* gl = context()->functions();
    gl->glClearColor(0.055f, 0.059f, 0.075f, 1.f);
    gl->glClear(GL_COLOR_BUFFER_BIT);
    if (nodeId_.isEmpty()) return;
    const qreal dpr = devicePixelRatioF();
    presenter_.setViewport(0, 0, int(std::lround(width() * dpr)), int(std::lround(height() * dpr)));
    bool isNew = false;
    presenter_.draw(EngineBridge::instance()->frames(nodeId_), reader_, 1.0f, 0.0f, &isNew);
    if (isNew) framesShown_.fetch_add(1);
    if (!outgoing_.isEmpty()) {
        const float t = float(fade_.elapsed()) / float(fadeMs_);
        if (t >= 1.0f) {
            EngineBridge::instance()->releaseFrames(outgoing_);
            outgoing_.clear();
        } else {
            presenter_.draw(EngineBridge::instance()->frames(outgoing_), reader_, 1.0f - t);
        }
    }
}
