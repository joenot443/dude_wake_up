#include "PreviewWindow.h"
#include "Prefs.h"

#include "EngineBridge.h"
#include "Screenshots.h"
#include "StageWidget.h"
#include "ui/Flash.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QSettings>
#include <QTimer>

PreviewWindow::PreviewWindow(const QString& connectableId, const QString& name, Capture capture, QWidget* parent)
    : QWidget(parent, Qt::Window), id_(connectableId), capture_(std::move(capture)) {
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName("PreviewWindow");
    setStyleSheet("#PreviewWindow { background: black; }");
    setMinimumSize(240, 135);
    const QSize res = EngineBridge::instance()->resolution();
    resize(960, res.isEmpty() ? 540 : 960 * res.height() / res.width());
    view_ = new StageWidget(this, FramePresenter::Preview);
    // Let double-clicks reach this window for the fullscreen toggle.
    view_->setAttribute(Qt::WA_TransparentForMouseEvents);
    view_->setSourceNode(connectableId);
    connect(EngineBridge::instance(), &EngineBridge::resolutionChanged, this, &PreviewWindow::layoutView);
    connect(Prefs::instance(), &Prefs::changed, this, [this](const QString& id) {
        if (id == "output.stretchPreviews") layoutView();
    });
    notice_ = new QLabel(this);
    notice_->setStyleSheet("QLabel { background: rgba(0,0,0,170); color: white; border-radius: 6px; padding: 6px 10px; }");
    notice_->hide();
    // Hold Shift for the engine's and this window's frame rates, as on macOS.
    fps_ = new QLabel(this);
    fps_->setStyleSheet("QLabel { background: rgba(30,36,51,220); color: white; border-radius: 6px; padding: 4px 8px;"
                        " font-family: Consolas; font-size: 8pt; }");
    fps_->move(8, 8);
    fps_->hide();
    fpsTimer_ = new QTimer(this);
    fpsTimer_->setInterval(1000);
    connect(fpsTimer_, &QTimer::timeout, this, [this] {
        const quint64 frames = view_->framesShown();
        fps_->setText(QString("Engine %1 FPS\nPreview %2 FPS").arg(int(EngineBridge::instance()->fps())).arg(frames - lastFrames_));
        fps_->adjustSize();
        lastFrames_ = frames;
    });
    setNodeName(name);
}

bool PreviewWindow::stretchToFill() { return Prefs::boolean("output.stretchPreviews"); }

// Open preview windows follow the setting (see the constructor).
void PreviewWindow::setStretchToFill(bool on) { Prefs::set("output.stretchPreviews", on ? 1.0 : 0.0); }

void PreviewWindow::layoutView() {
    if (stretchToFill()) { view_->setGeometry(rect()); return; }
    const QSize res = EngineBridge::instance()->resolution();
    const QSize fit = (res.isEmpty() ? QSize(16, 9) : res).scaled(size(), Qt::KeepAspectRatio);
    view_->setGeometry((width() - fit.width()) / 2, (height() - fit.height()) / 2, fit.width(), fit.height());
}

void PreviewWindow::resizeEvent(QResizeEvent*) { layoutView(); }

void PreviewWindow::setNodeName(const QString& name) {
    setWindowTitle(QString("%1 — Nottawa Preview").arg(name));
}

void PreviewWindow::setSource(const QString& connectableId, const QString& name) {
    id_ = connectableId;
    view_->setSourceNode(connectableId);
    setNodeName(name);
}

int PreviewWindow::fadeMs() {
    return int(Prefs::number("output.advanceFade") * 1000);
}

void PreviewWindow::advanceTo(const QString& connectableId, const QString& name) {
    id_ = connectableId;
    view_->crossfadeTo(connectableId, fadeMs());
    setNodeName(name);
}

void PreviewWindow::toggleFullscreen() {
    isFullScreen() ? showNormal() : showFullScreen();
}

void PreviewWindow::flash(const QString& text) {
    notice_->setText(text);
    notice_->adjustSize();
    notice_->move(12, height() - notice_->height() - 12);
    notice_->show();
    notice_->raise();
    QTimer::singleShot(1800, notice_, &QWidget::hide);
}

void PreviewWindow::keyReleaseEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Shift && !e->isAutoRepeat()) {
        fps_->hide();
        fpsTimer_->stop();
    }
    QWidget::keyReleaseEvent(e);
}

void PreviewWindow::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Shift && !e->isAutoRepeat()) {
        lastFrames_ = view_->framesShown();
        fps_->setText(QString("Engine %1 FPS\nPreview -- FPS").arg(int(EngineBridge::instance()->fps())));
        fps_->adjustSize();
        fps_->show();
        fps_->raise();
        fpsTimer_->start();
        return;
    }
    if (e->key() == Qt::Key_F11) { toggleFullscreen(); return; }
    if (e->key() == Qt::Key_Escape) {
        if (isFullScreen()) showNormal();
        else close();
        return;
    }
    if (e->key() == Qt::Key_S && !(e->modifiers() & (Qt::ControlModifier | Qt::AltModifier))) {
        if ((e->modifiers() & Qt::ShiftModifier) && Screenshots::chooseFolder(this).isEmpty()) return;
        const QString file = Screenshots::save(capture_(id_), this);
        if (!file.isEmpty()) Flash::over(this);
        flash(file.isEmpty() ? "Couldn't save the screenshot"
                             : "Saved to " + QDir::toNativeSeparators(QFileInfo(file).absolutePath()));
        return;
    }
    if (e->key() == Qt::Key_C && e->modifiers() == Qt::NoModifier) {
        const QImage image = capture_(id_);
        if (image.isNull()) return;
        QApplication::clipboard()->setImage(image);
        flash("Copied");
        return;
    }
    QWidget::keyPressEvent(e);
}

void PreviewWindow::mouseDoubleClickEvent(QMouseEvent*) {
    toggleFullscreen();
}
