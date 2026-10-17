#include "Icons.h"

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QtMath>

namespace Icons {

namespace {

QPainterPath starPath(QPointF c, qreal outer, qreal inner) {
    QPainterPath path;
    for (int i = 0; i < 10; ++i) {
        const qreal r = (i % 2) ? inner : outer;
        const qreal a = qDegreesToRadians(-90.0 + i * 36.0);
        const QPointF pt(c.x() + r * qCos(a), c.y() + r * qSin(a));
        i == 0 ? path.moveTo(pt) : path.lineTo(pt);
    }
    path.closeSubpath();
    return path;
}

QPainterPath sparkle(QPointF c, qreal r) {
    QPainterPath path;
    const qreal k = r * 0.22;
    path.moveTo(c.x(), c.y() - r);
    path.quadTo(c.x() + k, c.y() - k, c.x() + r, c.y());
    path.quadTo(c.x() + k, c.y() + k, c.x(), c.y() + r);
    path.quadTo(c.x() - k, c.y() + k, c.x() - r, c.y());
    path.quadTo(c.x() - k, c.y() - k, c.x(), c.y() - r);
    return path;
}

} // anon

void paint(QPainter* p, const QRectF& rect, Kind kind, const QColor& color) {
    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    const qreal s = qMin(rect.width(), rect.height()) / 16.0;
    p->translate(rect.center() - QPointF(8 * s, 8 * s));
    p->scale(s, s);
    QPen pen(color, 1.3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);

    switch (kind) {
    case Video:
        p->drawRoundedRect(QRectF(1.5, 4, 9, 8), 1.5, 1.5);
        p->drawPolygon(QPolygonF({QPointF(10.5, 7), QPointF(14.5, 4.8), QPointF(14.5, 11.2), QPointF(10.5, 9)}));
        break;
    case Sparkles:
        p->setPen(Qt::NoPen);
        p->setBrush(color);
        p->drawPath(sparkle(QPointF(6.5, 9), 5.5));
        p->drawPath(sparkle(QPointF(12.5, 3.8), 2.6));
        p->drawPath(sparkle(QPointF(12.8, 12.6), 1.9));
        break;
    case Waveform: {
        QPainterPath path(QPointF(1, 8));
        for (int i = 0; i <= 28; ++i) {
            const qreal x = 1 + i * 0.5;
            const qreal env = qSin(M_PI * i / 28.0);
            path.lineTo(x, 8 - 5.5 * env * qSin(i * 0.9));
        }
        p->drawPath(path);
        break;
    }
    case Books:
        p->drawRect(QRectF(2, 3, 3, 11));
        p->drawRect(QRectF(6, 2, 3, 12));
        p->save();
        p->translate(11.5, 14);
        p->rotate(-14);
        p->drawRect(QRectF(-1.5, -11, 3, 11));
        p->restore();
        break;
    case Stack:
        p->drawRoundedRect(QRectF(2, 6, 12, 8), 1.5, 1.5);
        p->drawLine(QPointF(3.5, 4), QPointF(12.5, 4));
        p->drawLine(QPointF(5, 2), QPointF(11, 2));
        break;
    case Search:
        p->drawEllipse(QPointF(6.8, 6.8), 4.3, 4.3);
        p->drawLine(QPointF(10, 10), QPointF(14, 14));
        break;
    case Star:
        p->drawPath(starPath(QPointF(8, 8.6), 6.6, 2.9));
        break;
    case StarFilled:
        p->setBrush(color);
        p->drawPath(starPath(QPointF(8, 8.6), 6.6, 2.9));
        break;
    case Refresh: {
        p->drawArc(QRectF(2.5, 2.5, 11, 11), 60 * 16, 290 * 16);
        p->setBrush(color);
        p->drawPolygon(QPolygonF({QPointF(10.2, 0.8), QPointF(13.6, 3.2), QPointF(9.8, 4.9)}));
        break;
    }
    case Close:
        p->drawLine(QPointF(4, 4), QPointF(12, 12));
        p->drawLine(QPointF(12, 4), QPointF(4, 12));
        break;
    case Text: {
        QPen thick = pen; thick.setWidthF(1.6); p->setPen(thick);
        p->drawLine(QPointF(3, 3.5), QPointF(13, 3.5));
        p->drawLine(QPointF(8, 3.5), QPointF(8, 13.5));
        p->drawLine(QPointF(6, 13.5), QPointF(10, 13.5));
        break;
    }
    case Smiley:
        p->drawEllipse(QPointF(8, 8), 6.3, 6.3);
        p->drawArc(QRectF(4.8, 5.5, 6.4, 5.6), 200 * 16, 140 * 16);
        p->setBrush(color);
        p->setPen(Qt::NoPen);
        p->drawEllipse(QPointF(5.9, 6.4), 0.9, 0.9);
        p->drawEllipse(QPointF(10.1, 6.4), 0.9, 0.9);
        break;
    case Photo:
        p->drawRoundedRect(QRectF(1.5, 2.5, 13, 11), 1.5, 1.5);
        p->drawPolyline(QPolygonF({QPointF(2, 12.5), QPointF(6, 8), QPointF(9, 11), QPointF(11, 9), QPointF(14, 12)}));
        p->drawEllipse(QPointF(11, 5.6), 1.1, 1.1);
        break;
    case Webcam:
        p->drawEllipse(QPointF(8, 6.5), 4.8, 4.8);
        p->drawEllipse(QPointF(8, 6.5), 1.8, 1.8);
        p->drawLine(QPointF(5, 14.5), QPointF(11, 14.5));
        p->drawLine(QPointF(8, 11.3), QPointF(8, 14.5));
        break;
    case ChevronDown:
        p->drawPolyline(QPolygonF({QPointF(4, 6), QPointF(8, 10), QPointF(12, 6)}));
        break;
    case ChevronRight:
        p->drawPolyline(QPolygonF({QPointF(6, 4), QPointF(10, 8), QPointF(6, 12)}));
        break;
    }
    p->restore();
}

QIcon icon(Kind kind, const QColor& color, const QColor& activeColor) {
    QIcon out;
    for (const qreal dpr : {1.0, 2.0}) {
        auto render = [&](const QColor& c) {
            QPixmap pm(QSize(32, 32) * dpr);
            pm.setDevicePixelRatio(dpr);
            pm.fill(Qt::transparent);
            QPainter p(&pm);
            paint(&p, QRectF(0, 0, 32, 32), kind, c);
            return pm;
        };
        out.addPixmap(render(color), QIcon::Normal, QIcon::Off);
        if (activeColor.isValid()) out.addPixmap(render(activeColor), QIcon::Normal, QIcon::On);
    }
    return out;
}

} // namespace Icons
