#include "Toast.h"

#include "Theme.h"

#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>

Toast::Toast(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    opacity_ = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacity_);
    fade_ = new QPropertyAnimation(opacity_, "opacity", this);
    fade_->setDuration(250);
    hideTimer_.setSingleShot(true);
    connect(&hideTimer_, &QTimer::timeout, this, [this] {
        fade_->stop();
        fade_->setStartValue(opacity_->opacity());
        fade_->setEndValue(0.0);
        fade_->start();
    });
    connect(fade_, &QPropertyAnimation::finished, this, [this] { if (opacity_->opacity() == 0.0) hide(); });
    parent->installEventFilter(this);
    hide();
}

void Toast::show(const QString& message, int ms) {
    message_ = message;
    place();
    raise();
    QWidget::show();
    fade_->stop();
    fade_->setStartValue(opacity_->opacity() > 0 && isVisible() ? opacity_->opacity() : 0.0);
    fade_->setEndValue(1.0);
    fade_->start();
    hideTimer_.start(ms);
    update();
}

void Toast::place() {
    QFont f = font();
    f.setPixelSize(13);
    f.setWeight(QFont::Medium);
    setFont(f);
    const int w = fontMetrics().horizontalAdvance(message_) + 40;
    const int h = 36;
    setGeometry((parentWidget()->width() - w) / 2, parentWidget()->height() - h - 32, w, h);
}

bool Toast::eventFilter(QObject* watched, QEvent* e) {
    if (watched == parentWidget() && e->type() == QEvent::Resize && isVisible()) place();
    return false;
}

void Toast::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath pill;
    pill.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), height() / 2.0, height() / 2.0);
    p.fillPath(pill, Theme::backgroundSecondary);
    p.setPen(QPen(Theme::border, 1));
    p.drawPath(pill);
    p.setPen(Theme::textPrimary);
    p.drawText(rect(), Qt::AlignCenter, message_);
}
