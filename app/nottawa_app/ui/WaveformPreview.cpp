#include "WaveformPreview.h"

#include "Icons.h"
#include "Theme.h"

#include <QDateTime>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>

namespace {
double posMod(double a, double b) {
    const double r = std::fmod(a, b);
    return r >= 0 ? r : r + b;
}
}

WaveformPreview::WaveformPreview(QWidget* parent)
    : QWidget(parent), randOffset_(QRandomGenerator::global()->generateDouble() * M_PI * 2) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(80);
    timer_.setInterval(33);  // 30 fps, as on macOS
    connect(&timer_, &QTimer::timeout, this, qOverload<>(&QWidget::update));
}

QStringList WaveformPreview::shapeNames() {
    return {"Sine", "Square", "Sawtooth", "Triangle", "Pulse Train", "Exp Sine", "Harmonic",
            "Rectified", "Noise Sine", "Bitcrushed", "Moire"};
}

void WaveformPreview::setOn(bool on) {
    enabled_ = on;
    if (on && isVisible()) timer_.start(); else timer_.stop();
    update();
}

void WaveformPreview::showEvent(QShowEvent*) { if (enabled_) timer_.start(); }
void WaveformPreview::hideEvent(QHideEvent*) { timer_.stop(); }

double WaveformPreview::value(int shape, double freqT, double t, double freq, double randOffset) {
    switch (shape) {
    case 0: return std::sin(freqT);
    case 1: return std::sin(freqT) >= 0 ? 1.0 : -1.0;
    case 2: return 2.0 * posMod(freqT / (M_PI * 2), 1.0) - 1.0;
    case 3: return 2.0 * std::abs(2.0 * posMod(freqT / (M_PI * 2), 1.0) - 1.0) - 1.0;
    case 4: return posMod(freqT / (M_PI * 2), 1.0) < 0.1 ? 1.0 : -1.0;
    case 5: {
        const double s = std::sin(freqT);
        return std::clamp((s * std::exp(s) - 1.175) / 1.55, -1.0, 1.0);
    }
    case 6: return (std::sin(freqT) + 0.5 * std::sin(2 * freqT) + 0.25 * std::sin(3 * freqT)) / 1.75;
    case 7: return 2.0 * std::abs(std::sin(freqT)) - 1.0;
    case 8: return std::sin(freqT + (std::sin(t * 0.73) * 0.5 + std::sin(t * 1.31) * 0.3) * 5.0);
    case 9: {
        const double steps = 8.0;
        double v = std::floor(std::sin(freqT) * steps) / steps;
        return std::clamp(v * steps / (steps - 1.0), -1.0, 1.0);
    }
    case 10: return 0.5 * std::sin(freqT) + 0.5 * std::sin(freq * 1.05 * t + randOffset);
    }
    return 0;
}

void WaveformPreview::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath frame;
    frame.addRoundedRect(r, 6, 6);

    if (!enabled_) {
        QColor bg = Theme::backgroundSecondary; bg.setAlphaF(0.5);
        p.fillPath(frame, bg);
        p.setPen(QPen(Theme::borderSubtle, 1));
        p.drawPath(frame);
        Icons::paint(&p, QRectF(r.center().x() - 8, r.center().y() - 16, 16, 16), Icons::Waveform, Theme::textTertiary);
        QFont f = font(); f.setPixelSize(10); p.setFont(f);
        p.setPen(Theme::textTertiary);
        p.drawText(QRectF(r.left(), r.center().y() + 2, r.width(), 16), Qt::AlignCenter, "Oscillator Off");
        return;
    }

    p.fillPath(frame, Theme::backgroundSecondary);
    p.setClipPath(frame);
    const qreal w = r.width(), h = r.height();
    QColor grid = Theme::border; grid.setAlphaF(0.3);
    p.setPen(QPen(grid, 0.5));
    for (int i = 1; i < 4; ++i) p.drawLine(QPointF(r.left(), r.top() + h * i / 4), QPointF(r.right(), r.top() + h * i / 4));

    const double actualMin = std::min(min_, max_), actualMax = std::max(min_, max_);
    const double span = double(paramMax_) - paramMin_;
    if (span > 0) {
        auto yPos = [&](double v) { return r.top() + (1.0 - (v - paramMin_) / span) * h; };
        const bool inBounds = actualMin >= paramMin_ && actualMax <= paramMax_;
        QColor line = inBounds ? Theme::success : Theme::warning;
        line.setAlphaF(0.6);
        QPen dashed(line, 1, Qt::CustomDashLine);
        dashed.setDashPattern({4, 3});
        p.setPen(dashed);
        const qreal minY = yPos(actualMin), maxY = yPos(actualMax);
        p.drawLine(QPointF(r.left(), minY), QPointF(r.right(), minY));
        p.drawLine(QPointF(r.left(), maxY), QPointF(r.right(), maxY));
        QFont mono("Consolas"); mono.setPixelSize(9); p.setFont(mono);
        p.setPen(line);
        p.drawText(QRectF(r.right() - 40, minY + 1, 36, 12), Qt::AlignRight | Qt::AlignTop, QString::number(min_, 'f', 1));
        p.drawText(QRectF(r.right() - 40, maxY - 13, 36, 12), Qt::AlignRight | Qt::AlignBottom, QString::number(max_, 'f', 1));

        // Six seconds of output, newest at the right.
        const double phase = QDateTime::currentMSecsSinceEpoch() / 1000.0;
        const int samples = int(w);
        QPainterPath wave;
        for (int i = 0; i < samples; ++i) {
            const double frac = double(i) / std::max(1, samples - 1);
            const double t = phase - 6.0 + frac * 6.0;
            const double v = value(shape_, frequency_ * t + randOffset_, t, frequency_, randOffset_);
            const double out = std::clamp(actualMin + (v + 1.0) / 2.0 * (actualMax - actualMin), double(paramMin_), double(paramMax_));
            const QPointF pt(r.left() + frac * w, yPos(out));
            i == 0 ? wave.moveTo(pt) : wave.lineTo(pt);
        }
        QColor glow = Theme::accent; glow.setAlphaF(0.25);
        p.setPen(QPen(glow, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawPath(wave);
        p.setPen(QPen(Theme::accent, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawPath(wave);
    }
    p.setClipping(false);
    p.setPen(QPen(Theme::borderSubtle, 1));
    p.drawPath(frame);
}
