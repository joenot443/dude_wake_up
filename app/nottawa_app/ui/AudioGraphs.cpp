#include "AudioGraphs.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <algorithm>

namespace {
const QColor kCard(20, 20, 20);  // white 0.08, as on macOS

QColor alpha(QColor c, qreal a) { c.setAlphaF(a); return c; }

QPainterPath card(const QRectF& r) {
    QPainterPath path;
    path.addRoundedRect(r, 6, 6);
    return path;
}
}

AudioGraph::AudioGraph(Mode mode, QWidget* parent) : QWidget(parent), mode_(mode) {
    setMinimumHeight(40);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void AudioGraph::setSeries(const QVector<QVector<float>>& series, const QVector<QColor>& colors) {
    series_ = series;
    colors_ = colors;
    update();
}

void AudioGraph::setValues(const QVector<float>& values, const QColor& color) {
    series_ = {values};
    colors_ = {color};
    update();
}

void AudioGraph::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r = rect();
    const QPainterPath shape = card(r);
    p.fillPath(shape, kCard);
    p.setClipPath(shape);
    const qreal w = r.width(), h = r.height();
    p.setPen(QPen(alpha(Qt::white, 0.06), 0.5));
    for (int i = 1; i < 4; ++i) p.drawLine(QPointF(0, h * i / 4), QPointF(w, h * i / 4));

    if (mode_ == Bars) {
        if (series_.isEmpty() || series_[0].isEmpty()) return;
        const QVector<float>& v = series_[0];
        const QColor c = colors_.value(0, Qt::cyan);
        const int n = int(v.size());
        const qreal barW = std::max(2.0, (w - (n + 1) * 3.0) / n);
        p.setPen(Qt::NoPen);
        for (int i = 0; i < n; ++i) {
            const qreal barH = std::clamp(double(v[i]), 0.0, 1.0) * (h - 4);
            const qreal x = 3 + i * (barW + 3);
            QLinearGradient g(x, h - barH, x, h);
            g.setColorAt(0, c);
            g.setColorAt(1, alpha(c, 0.3));
            QPainterPath bar;
            bar.addRoundedRect(QRectF(x, h - barH, barW, barH + 3), 2, 2);
            p.fillPath(bar, g);
        }
        return;
    }

    if (mode_ == Waveform) {
        p.setPen(QPen(alpha(Qt::white, 0.12), 0.5));
        p.drawLine(QPointF(0, h / 2), QPointF(w, h / 2));
        if (series_.isEmpty() || series_[0].size() < 2) return;
        const QVector<float>& v = series_[0];
        const QColor c = colors_.value(0, QColor(0x00, 0xc7, 0xbe));
        QPainterPath line;
        const qreal step = w / (v.size() - 1);
        for (int i = 0; i < v.size(); ++i) {
            const QPointF pt(i * step, std::clamp(h / 2 - v[i] * h / 2, 0.0, h));
            i == 0 ? line.moveTo(pt) : line.lineTo(pt);
        }
        QPainterPath fill = line;
        fill.lineTo((v.size() - 1) * step, h / 2);
        fill.lineTo(0, h / 2);
        fill.closeSubpath();
        QLinearGradient g(0, 0, 0, h);
        g.setColorAt(0, alpha(c, 0.25));
        g.setColorAt(0.5, alpha(c, 0.0));
        g.setColorAt(1, alpha(c, 0.25));
        p.fillPath(fill, g);
        p.setPen(QPen(alpha(c, 0.3), 5));
        p.drawPath(line);
        p.setPen(QPen(c, 2));
        p.drawPath(line);
        return;
    }

    // Lines, with a gradient fill under the first series.
    for (int s = 0; s < series_.size(); ++s) {
        const QVector<float>& v = series_[s];
        if (v.size() < 2) continue;
        const QColor c = colors_.value(s, Qt::green);
        QPainterPath line;
        const qreal step = w / (v.size() - 1);
        for (int i = 0; i < v.size(); ++i) {
            const QPointF pt(i * step, std::clamp(h - v[i] * h, 0.0, h));
            i == 0 ? line.moveTo(pt) : line.lineTo(pt);
        }
        if (series_.size() == 1) {
            QPainterPath fill = line;
            fill.lineTo((v.size() - 1) * step, h);
            fill.lineTo(0, h);
            fill.closeSubpath();
            QLinearGradient g(0, 0, 0, h);
            g.setColorAt(0, alpha(c, 0.35));
            g.setColorAt(1, alpha(c, 0.0));
            p.fillPath(fill, g);
        }
        p.setPen(QPen(alpha(c, 0.3), 5));
        p.drawPath(line);
        p.setPen(QPen(c, 2));
        p.drawPath(line);
    }
}

BeatIndicator::BeatIndicator(QWidget* parent) : QWidget(parent) {
    setMinimumSize(80, 60);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void BeatIndicator::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r = rect();
    const QPainterPath shape = card(r);
    p.fillPath(shape, kCard);
    p.setClipPath(shape);
    const QColor orange(0xff, 0x95, 0x00);
    const qreal f = flash_;
    if (f > 0.01) p.fillPath(shape, alpha(orange, f * 0.08));
    const QPointF c = r.center();
    const qreal side = std::min(r.width(), r.height());
    if (f > 0.01) {
        const qreal glow = side * 0.48 + f * 20;
        QRadialGradient g(c, glow);
        g.setColorAt(0, alpha(orange, f * 0.5));
        g.setColorAt(0.5, alpha(orange, f * 0.15));
        g.setColorAt(1, alpha(orange, 0));
        p.setPen(Qt::NoPen);
        p.setBrush(g);
        p.drawEllipse(c, glow, glow);
    }
    const qreal ring = side * 0.35 * (1 + f * 0.08);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(alpha(orange, 0.25 + f * 0.6), 2.5 + f * 2));
    p.drawEllipse(c, ring, ring);

    QFont big = font();
    big.setPixelSize(std::max(10, int(side * 0.3)));
    big.setBold(true);
    p.setFont(big);
    p.setPen(alpha(Qt::white, 0.7 + f * 0.3));
    p.drawText(QRectF(r.left(), c.y() - side * 0.25, r.width(), side * 0.38), Qt::AlignCenter, QString::number(int(bpm_)));
    QFont small = font();
    small.setPixelSize(10);
    small.setWeight(QFont::Medium);
    p.setFont(small);
    p.setPen(alpha(Qt::white, 0.3 + f * 0.2));
    p.drawText(QRectF(r.left(), c.y() + side * 0.1, r.width(), 14), Qt::AlignCenter, "BPM");
}
