#include "PreviewWindow.h"

#include "Screenshots.h"
#include "StageWidget.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

PreviewWindow::PreviewWindow(const QString& connectableId, const QString& name, Capture capture, QWidget* parent)
    : QWidget(parent, Qt::Window), id_(connectableId), capture_(std::move(capture)) {
    setAttribute(Qt::WA_DeleteOnClose);
    resize(960, 540);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    view_ = new StageWidget(this, FramePresenter::Preview);
    view_->setMinimumSize(320, 180);
    // Let double-clicks reach this window for the fullscreen toggle.
    view_->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(view_);
    view_->setSourceNode(connectableId);
    notice_ = new QLabel(this);
    notice_->setStyleSheet("QLabel { background: rgba(0,0,0,170); color: white; border-radius: 6px; padding: 6px 10px; }");
    notice_->hide();
    setNodeName(name);
}

void PreviewWindow::setNodeName(const QString& name) {
    setWindowTitle(QString("%1 — Nottawa Preview").arg(name));
}

void PreviewWindow::setSource(const QString& connectableId, const QString& name) {
    id_ = connectableId;
    view_->setSourceNode(connectableId);
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

void PreviewWindow::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_F11) { toggleFullscreen(); return; }
    if (e->key() == Qt::Key_Escape && isFullScreen()) { showNormal(); return; }
    if (e->key() == Qt::Key_S && !(e->modifiers() & (Qt::ControlModifier | Qt::AltModifier))) {
        if ((e->modifiers() & Qt::ShiftModifier) && Screenshots::chooseFolder(this).isEmpty()) return;
        const QString file = Screenshots::save(capture_(id_), this);
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
