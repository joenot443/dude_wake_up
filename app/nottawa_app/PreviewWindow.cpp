#include "PreviewWindow.h"

#include "StageWidget.h"

#include <QKeyEvent>
#include <QVBoxLayout>

PreviewWindow::PreviewWindow(const QString& connectableId, const QString& name, QWidget* parent)
    : QWidget(parent, Qt::Window) {
    setAttribute(Qt::WA_DeleteOnClose);
    resize(960, 540);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    view_ = new StageWidget(this, FramePresenter::Preview);
    // Let double-clicks reach this window for the fullscreen toggle.
    view_->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(view_);
    view_->setSourceNode(connectableId);
    setNodeName(name);
}

void PreviewWindow::setNodeName(const QString& name) {
    setWindowTitle(QString("%1 — Nottawa Preview").arg(name));
}

void PreviewWindow::toggleFullscreen() {
    isFullScreen() ? showNormal() : showFullScreen();
}

void PreviewWindow::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_F11) { toggleFullscreen(); return; }
    if (e->key() == Qt::Key_Escape && isFullScreen()) { showNormal(); return; }
    QWidget::keyPressEvent(e);
}

void PreviewWindow::mouseDoubleClickEvent(QMouseEvent*) {
    toggleFullscreen();
}
