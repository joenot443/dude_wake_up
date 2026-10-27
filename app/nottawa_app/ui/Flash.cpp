#include "Flash.h"

#include <QPainter>
#include <QVariantAnimation>
#include <QWidget>

namespace {
class FlashWidget : public QWidget {
public:
    explicit FlashWidget(QWidget* parent) : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        // Only the painted white shows, not the app stylesheet's panel fill.
        setObjectName("Flash");
        setStyleSheet("#Flash { background: transparent; }");
        setGeometry(parent->rect());
        auto* fade = new QVariantAnimation(this);
        fade->setStartValue(0.6);
        fade->setEndValue(0.0);
        fade->setDuration(250);
        fade->setEasingCurve(QEasingCurve::OutCubic);
        connect(fade, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
            opacity_ = v.toReal();
            update();
        });
        connect(fade, &QVariantAnimation::finished, this, &QObject::deleteLater);
        show();
        raise();
        fade->start();
    }
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setOpacity(opacity_);
        p.fillRect(rect(), Qt::white);
    }
private:
    qreal opacity_ = 0.6;
};
} // anon

namespace Flash {
void over(QWidget* target) {
    if (target) new FlashWidget(target);
}
}
