#include "Icons.h"

#include "Theme.h"

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
    case ChevronUp:
        p->drawPolyline(QPolygonF({QPointF(4, 10), QPointF(8, 6), QPointF(12, 10)}));
        break;
    case Note:
        p->drawLine(QPointF(6, 12), QPointF(6, 3));
        p->drawLine(QPointF(6, 3), QPointF(13, 1.8));
        p->drawLine(QPointF(13, 1.8), QPointF(13, 10.5));
        p->setBrush(color);
        p->drawEllipse(QPointF(4.3, 12.4), 2.1, 1.7);
        p->drawEllipse(QPointF(11.3, 10.9), 2.1, 1.7);
        break;
    case Keys:
        p->drawRoundedRect(QRectF(1.5, 3, 13, 10), 1.2, 1.2);
        for (const qreal x : {4.75, 8.0, 11.25}) p->drawLine(QPointF(x, 8.5), QPointF(x, 13));
        p->setBrush(color);
        p->setPen(Qt::NoPen);
        for (const qreal x : {4.75, 8.0, 11.25}) p->drawRect(QRectF(x - 0.9, 3, 1.8, 5.5));
        break;
    case Reset:
        p->drawArc(QRectF(2.5, 2.5, 11, 11), 120 * 16, 290 * 16);
        p->setBrush(color);
        p->drawPolygon(QPolygonF({QPointF(2.0, 2.4), QPointF(5.6, 3.6), QPointF(3.0, 6.4)}));
        break;
    case Play:
        p->setBrush(color);
        p->drawPolygon(QPolygonF({QPointF(4.5, 2.5), QPointF(13, 8), QPointF(4.5, 13.5)}));
        break;
    case Pause:
        p->setBrush(color);
        p->drawRoundedRect(QRectF(4, 3, 2.8, 10), 0.8, 0.8);
        p->drawRoundedRect(QRectF(9.2, 3, 2.8, 10), 0.8, 0.8);
        break;
    case Undo:
    case Redo: {
        p->save();
        if (kind == Redo) { p->translate(16, 0); p->scale(-1, 1); }
        QPainterPath arc;
        arc.moveTo(4, 6);
        arc.lineTo(10, 6);
        arc.cubicTo(14.5, 6, 14.5, 13, 10, 13);
        arc.lineTo(6, 13);
        p->drawPath(arc);
        p->drawPolyline(QPolygonF({QPointF(6.5, 3.2), QPointF(3.6, 6), QPointF(6.5, 8.8)}));
        p->restore();
        break;
    }
    case Sidebar:
        p->drawRoundedRect(QRectF(1.5, 2.5, 13, 11), 2, 2);
        p->drawLine(QPointF(6, 2.5), QPointF(6, 13.5));
        break;
    case Sliders:
        p->drawLine(QPointF(2, 5), QPointF(14, 5));
        p->drawLine(QPointF(2, 11), QPointF(14, 11));
        p->setBrush(Theme::backgroundSecondary);
        p->drawEllipse(QPointF(10, 5), 2, 2);
        p->drawEllipse(QPointF(6, 11), 2, 2);
        break;
    case Trash:
    case Clear:
        p->drawLine(QPointF(2.5, 4), QPointF(13.5, 4));
        p->drawLine(QPointF(6.2, 2), QPointF(9.8, 2));
        p->drawPolyline(QPolygonF({QPointF(4, 4), QPointF(4.8, 14), QPointF(11.2, 14), QPointF(12, 4)}));
        if (kind == Clear) p->drawLine(QPointF(2, 15), QPointF(14, 1));
        else { p->drawLine(QPointF(6.7, 6.5), QPointF(6.9, 11.5)); p->drawLine(QPointF(9.3, 6.5), QPointF(9.1, 11.5)); }
        break;
    case Camera:
        p->drawRoundedRect(QRectF(1.5, 4.5, 13, 9), 1.8, 1.8);
        p->drawPolyline(QPolygonF({QPointF(5.5, 4.5), QPointF(6.5, 2.5), QPointF(9.5, 2.5), QPointF(10.5, 4.5)}));
        p->drawEllipse(QPointF(8, 9), 2.6, 2.6);
        break;
    case Help:
        p->drawEllipse(QPointF(8, 8), 6.5, 6.5);
        p->drawArc(QRectF(5.8, 3.8, 4.4, 4.4), 180 * 16, -250 * 16);
        p->drawLine(QPointF(8, 8.4), QPointF(8, 9.6));
        p->setBrush(color);
        p->drawEllipse(QPointF(8, 11.8), 0.6, 0.6);
        break;
    case Fit:
        for (const int sx : {-1, 1})
            for (const int sy : {-1, 1}) {
                const QPointF c(8 + sx * 2, 8 + sy * 2), far(8 + sx * 6.5, 8 + sy * 6.5);
                p->drawLine(far, c);
                p->drawPolyline(QPolygonF({c + QPointF(sx * 3, 0), c, c + QPointF(0, sy * 3)}));
            }
        break;
    case ChevronLeft:
        p->drawPolyline(QPolygonF({QPointF(10, 4), QPointF(6, 8), QPointF(10, 12)}));
        break;
    case Lock:
    case Unlock:
        p->setBrush(color);
        p->drawRoundedRect(QRectF(3.5, 7.5, 9, 7), 1.2, 1.2);
        p->setBrush(Qt::NoBrush);
        if (kind == Lock) p->drawArc(QRectF(5, 2.5, 6, 10), 0, 180 * 16);
        else p->drawArc(QRectF(8, 1.5, 6, 10), 20 * 16, 160 * 16);
        if (kind == Lock) { p->drawLine(QPointF(5, 7.5), QPointF(5, 7)); p->drawLine(QPointF(11, 7.5), QPointF(11, 7)); }
        break;
    case Stop:
        p->setBrush(color);
        p->drawRoundedRect(QRectF(3.5, 3.5, 9, 9), 1.5, 1.5);
        break;
    case Folder:
        p->drawPolyline(QPolygonF({QPointF(1.5, 13.5), QPointF(1.5, 3), QPointF(6, 3), QPointF(7.5, 4.8),
                                   QPointF(14.5, 4.8), QPointF(14.5, 13.5), QPointF(1.5, 13.5)}));
        p->drawLine(QPointF(1.5, 6.8), QPointF(14.5, 6.8));
        break;
    case NewSquare: {
        QPen dashed = p->pen();
        dashed.setDashPattern({1.6, 1.4});
        p->setPen(dashed);
        p->drawRoundedRect(QRectF(1.5, 1.5, 13, 13), 2, 2);
        dashed.setStyle(Qt::SolidLine);
        p->setPen(dashed);
        p->drawLine(QPointF(8, 5), QPointF(8, 11));
        p->drawLine(QPointF(5, 8), QPointF(11, 8));
        break;
    }
    case Plus:
        p->drawLine(QPointF(8, 3.5), QPointF(8, 12.5));
        p->drawLine(QPointF(3.5, 8), QPointF(12.5, 8));
        break;
    case Minus:
        p->drawLine(QPointF(3.5, 8), QPointF(12.5, 8));
        break;
    case Film:
        p->drawRoundedRect(QRectF(2.5, 1.5, 11, 13), 1.2, 1.2);
        p->drawLine(QPointF(5, 1.5), QPointF(5, 14.5));
        p->drawLine(QPointF(11, 1.5), QPointF(11, 14.5));
        p->drawLine(QPointF(5, 8), QPointF(11, 8));
        for (const qreal y : {4.0, 8.0, 12.0}) {
            p->drawLine(QPointF(2.5, y), QPointF(5, y));
            p->drawLine(QPointF(11, y), QPointF(13.5, y));
        }
        break;
    case Cloud: {
        QPainterPath cloud;
        cloud.moveTo(4.5, 11.5);
        cloud.cubicTo(1.5, 11.5, 1.2, 7.4, 4.2, 7.0);
        cloud.cubicTo(4.6, 3.4, 9.6, 2.6, 10.8, 5.8);
        cloud.cubicTo(14.4, 5.6, 15.4, 11.5, 11.8, 11.5);
        p->drawPath(cloud);
        p->drawLine(QPointF(8, 7.5), QPointF(8, 14.5));
        p->drawPolyline(QPolygonF({QPointF(6, 12.5), QPointF(8, 14.5), QPointF(10, 12.5)}));
        break;
    }
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
