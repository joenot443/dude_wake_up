#include "ActionBar.h"

#include "Icons.h"
#include "Theme.h"

#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QSettings>
#include <QToolButton>

namespace {
QFrame* separator(QWidget* parent) {
    auto* line = new QFrame(parent);
    line->setFixedSize(1, 20);
    QColor c = Theme::border;
    c.setAlphaF(0.6);
    line->setStyleSheet(QString("background: %1;").arg(c.name(QColor::HexArgb)));
    return line;
}
}

ActionBar::ActionBar(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    layout_ = new QHBoxLayout(this);
    layout_->setContentsMargins(8, 6, 8, 6);
    layout_->setSpacing(4);
    toggle_ = button(Icons::ChevronRight, "Collapse action bar");
    connect(toggle_, &QToolButton::clicked, this, [this] { setExpanded(!isExpanded_); });
    layout_->addWidget(toggle_);

    expanded_ = new QWidget(this);
    expanded_->setAttribute(Qt::WA_TranslucentBackground);
    auto* row = new QHBoxLayout(expanded_);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(4);
    row->addWidget(separator(expanded_));
    undo_ = button(Icons::Undo, "Undo (Ctrl+Z)");
    redo_ = button(Icons::Redo, "Redo (Ctrl+Shift+Z)");
    auto* shot = button(Icons::Camera, "Capture screenshot");
    help_ = button(Icons::Help, "Toggle help overlay");
    connect(help_, &QToolButton::clicked, this, &ActionBar::helpToggled);
    auto* clear = button(Icons::Clear, "Clear all nodes");
    auto* fit = button(Icons::Fit, "Zoom to fit all nodes");
    connect(undo_, &QToolButton::clicked, this, &ActionBar::undo);
    connect(redo_, &QToolButton::clicked, this, &ActionBar::redo);
    connect(shot, &QToolButton::clicked, this, &ActionBar::screenshot);
    connect(clear, &QToolButton::clicked, this, &ActionBar::clearAll);
    connect(fit, &QToolButton::clicked, this, &ActionBar::zoomToFit);
    const QList<QWidget*> items = {undo_, redo_, separator(expanded_), shot, help_, separator(expanded_), clear, fit};
    for (QWidget* w : items) {
        w->setParent(expanded_);
        row->addWidget(w);
    }
    layout_->addWidget(expanded_);

    parent->installEventFilter(this);
    setExpanded(QSettings().value("ui/actionBarExpanded", true).toBool());
}

QToolButton* ActionBar::button(int icon, const QString& tip) {
    auto* b = new QToolButton(this);
    b->setIcon(Icons::icon(Icons::Kind(icon), Theme::textPrimary));
    b->setIconSize(QSize(16, 16));
    b->setFixedSize(28, 28);
    b->setToolTip(tip);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(QString("QToolButton { background: transparent; border: none; border-radius: 6px; }"
                             "QToolButton:hover { background: %1; }"
                             "QToolButton:pressed { background: %2; }"
                             "QToolButton:disabled { background: transparent; }")
                         .arg(Theme::surfaceHover.name(), Theme::surfacePressed.name()));
    return b;
}

void ActionBar::setExpanded(bool on) {
    isExpanded_ = on;
    expanded_->setVisible(on);
    toggle_->setIcon(Icons::icon(on ? Icons::ChevronRight : Icons::ChevronLeft, Theme::textPrimary));
    toggle_->setToolTip(on ? "Collapse action bar" : "Expand action bar");
    layout_->setContentsMargins(on ? 8 : 6, 6, on ? 8 : 6, 6);
    QSettings().setValue("ui/actionBarExpanded", on);
    place();
}

void ActionBar::setUndoRedo(bool canUndo, bool canRedo) {
    undo_->setEnabled(canUndo);
    redo_->setEnabled(canRedo);
    undo_->setIcon(Icons::icon(Icons::Undo, canUndo ? Theme::textPrimary : Theme::textTertiary));
    redo_->setIcon(Icons::icon(Icons::Redo, canRedo ? Theme::textPrimary : Theme::textTertiary));
}

void ActionBar::setHelpActive(bool on) {
    help_->setIcon(Icons::icon(Icons::Help, on ? Theme::accent : Theme::textPrimary));
}

void ActionBar::place() {
    adjustSize();
    if (QWidget* p = parentWidget()) move(p->width() - width() - 16, p->height() - height() - 16);
    raise();
}

bool ActionBar::eventFilter(QObject* watched, QEvent* e) {
    if (watched == parentWidget() && e->type() == QEvent::Resize) place();
    return false;
}

void ActionBar::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath shape;
    shape.addRoundedRect(QRectF(rect()), 12, 12);
    QColor bg = Theme::surface;
    bg.setAlphaF(0.85);
    p.fillPath(shape, bg);
}
