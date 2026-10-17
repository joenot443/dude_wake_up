#include "StageWidget.h"

#include "EngineBridge.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QScreen>
#include <QtGui/qopenglcontext_platform.h>

StageWidget::StageWidget(QWidget* parent) : QOpenGLWidget(parent) {
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
    setMinimumSize(320, 180);
    repaintTimer_.setTimerType(Qt::PreciseTimer);
    connect(&repaintTimer_, &QTimer::timeout, this, [this] { if (isVisible()) update(); });
    const qreal hz = screen() ? screen()->refreshRate() : 60.0;
    // Round the interval down so no engine frame is skipped.
    repaintTimer_.start(qMax(1, int(1000.0 / (hz > 0 ? hz : 60.0))));
}

StageWidget::~StageWidget() {
    if (!nodeId_.isEmpty()) EngineBridge::instance()->releaseFrames(nodeId_);
    makeCurrent();
    presenter_.destroy();
    doneCurrent();
}

void StageWidget::setSourceNode(const QString& connectableId) {
    if (connectableId == nodeId_) return;
    auto* engine = EngineBridge::instance();
    if (!nodeId_.isEmpty()) engine->releaseFrames(nodeId_);
    nodeId_ = connectableId;
    if (!nodeId_.isEmpty()) engine->retainFrames(nodeId_);
    update();
}

void StageWidget::initializeGL() {
    if (auto* wgl = context()->nativeInterface<QNativeInterface::QWGLContext>())
        nativeContext_ = wgl->nativeContext();
    presenter_.initialize();
    emit glReady();
}

void StageWidget::paintGL() {
    QOpenGLFunctions* gl = context()->functions();
    gl->glClearColor(0.055f, 0.059f, 0.075f, 1.f);
    gl->glClear(GL_COLOR_BUFFER_BIT);
    if (nodeId_.isEmpty()) return;
    bool isNew = false;
    presenter_.draw(EngineBridge::instance()->frames(nodeId_), FramePresenter::Stage, 1.0f, &isNew);
    if (isNew) framesShown_.fetch_add(1);
}
