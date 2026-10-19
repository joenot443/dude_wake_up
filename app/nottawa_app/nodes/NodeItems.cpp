#include "NodeItems.h"

#include "Theme.h"

#include "EngineBridge.h"
#include "NodeEditorView.h"

#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPathStroker>

namespace {
const QColor& kAccent      = Theme::accent;
const QColor& kSuccess     = Theme::success;
const QColor& kWarning     = Theme::warning;
const QColor& kSourceDot   = Theme::success;
const QColor& kInactiveDot = Theme::textTertiary;
const QColor& kBody        = Theme::backgroundTertiary;
constexpr qreal kRadius     = 14.0;
constexpr qreal kTitleH     = 30.0;
constexpr qreal kPortDot    = 3.0;   // radius of the 6 px dot
constexpr qreal kPortHit    = 7.0;   // radius of the 14 px hit area
} // anon

// ---------------------------------------------------------------- PortItem

PortItem::PortItem(QGraphicsItem* parent, Kind kind, int slot)
    : QGraphicsItem(parent), kind_(kind), slot_(slot) {
    setAcceptHoverEvents(true);
    setZValue(2);
}

QString PortItem::nodeId() const { return static_cast<const NodeItem*>(parentItem())->id(); }

void PortItem::setConnected(bool on) { if (connected_ != on) { connected_ = on; update(); } }
void PortItem::setDragState(int state) { if (dragState_ != state) { dragState_ = state; update(); } }

QRectF PortItem::boundingRect() const {
    return QRectF(-kPortHit - 6, -kPortHit, 2 * kPortHit + 12, 2 * kPortHit);
}

void PortItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->setRenderHint(QPainter::Antialiasing);
    // Pill backing straddling the node edge.
    p->setPen(Qt::NoPen);
    QColor backing = Theme::background;
    backing.setAlpha(200);
    p->setBrush(backing);
    p->drawRoundedRect(QRectF(-kPortHit, -kPortHit, 2 * kPortHit, 2 * kPortHit), kPortHit, kPortHit);

    const QColor color = dragState_ == 2 ? kSuccess : (slot_ == kAuxOutputSlot ? kWarning : kAccent);
    const qreal r = dragState_ == 1 ? kPortDot + 1.5 : kPortDot;
    if (connected_ || dragState_ == 2) {
        p->setBrush(color);
        p->drawEllipse(QPointF(0, 0), r, r);
    } else {
        p->setBrush(Qt::NoBrush);
        p->setPen(QPen(color, 1.5));
        p->drawEllipse(QPointF(0, 0), r, r);
    }
    if (slot_ == kAuxOutputSlot) {
        p->setPen(kWarning);
        QFont f = p->font(); f.setPointSizeF(7); f.setBold(true); p->setFont(f);
        p->drawText(QRectF(kPortHit, -kPortHit, 10, 2 * kPortHit), Qt::AlignCenter, "A");
    }
}

// ---------------------------------------------------------------- NodeItem

NodeItem::NodeItem(NodeEditorView* view, const QString& id) : view_(view), id_(id) {
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setZValue(1);
}

void NodeItem::setInfo(const QString& name, bool isShader, bool active, int inputCount, bool hasAux) {
    name_ = name;
    isShader_ = isShader;
    active_ = active;
    if (inputCount != inputCount_ || hasAux != hasAux_) rebuildPorts(inputCount, hasAux);
    update();
}

void NodeItem::setNeedsInput(bool on) { if (needsInput_ != on) { needsInput_ = on; update(); } }
void NodeItem::setAudioHint(bool on) { if (audioHint_ != on) { audioHint_ = on; update(); } }

void NodeItem::setDownload(float progress, bool paused) {
    if (progress == download_ && paused == downloadPaused_) return;
    download_ = progress;
    downloadPaused_ = paused;
    update();
}

void NodeItem::rebuildPorts(int inputCount, bool hasAux) {
    inputCount_ = inputCount;
    hasAux_ = hasAux;
    qDeleteAll(ports_);
    ports_.clear();
    const QRectF b = body();
    for (int i = 0; i < inputCount; ++i) {
        auto* port = new PortItem(this, PortItem::Input, i);
        port->setPos(b.left(), b.top() + b.height() * (i + 1) / (inputCount + 1));
        ports_.push_back(port);
    }
    auto* out = new PortItem(this, PortItem::Output, 0);
    out->setPos(b.right(), hasAux ? -22 : 0);
    ports_.push_back(out);
    if (hasAux) {
        auto* aux = new PortItem(this, PortItem::Output, kAuxOutputSlot);
        aux->setPos(b.right(), 22);
        ports_.push_back(aux);
    }
}

PortItem* NodeItem::port(PortItem::Kind kind, int slot) const {
    for (PortItem* p : ports_) if (p->kind() == kind && p->slot() == slot) return p;
    return nullptr;
}

QRectF NodeItem::boundingRect() const {
    return body().adjusted(-10, -10, 10, 14);  // room for the selection glow and shadow
}

void NodeItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->setRenderHint(QPainter::Antialiasing);
    const QRectF b = body();
    QPainterPath rounded;
    rounded.addRoundedRect(b, kRadius, kRadius);

    // A soft drop shadow (the macOS node's 40% black, 8 radius, 4 down):
    // stacked translucent outlines whose opacity builds up towards the card.
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(0, 0, 0, 15));
    for (int spread = 8; spread >= 1; --spread)
        p->drawRoundedRect(b.adjusted(-spread, -spread + 4, spread, spread + 4), kRadius + spread, kRadius + spread);

    p->fillPath(rounded, kBody);
    view_->drawNodePreview(p, this, b, kRadius, active_ ? 1.0f : 0.4f);

    // The hints follow the macOS NodeView: a scrim of the background colour
    // with secondary text.
    QColor scrim = Theme::background;
    scrim.setAlphaF(0.6);
    if (needsInput_) {
        p->fillPath(rounded, scrim);
        p->setPen(Theme::textSecondary);
        QFont f = p->font(); f.setPointSizeF(9); p->setFont(f);
        p->drawText(b.adjusted(0, 0, 0, -kTitleH), Qt::AlignCenter, "Connect a source");
    } else if (audioHint_) {
        p->fillPath(rounded, scrim);
        p->setPen(Theme::textSecondary);
        QFont f = p->font(); f.setPointSizeF(9); p->setFont(f);
        p->drawText(b.adjusted(0, 0, 0, -kTitleH), Qt::AlignCenter, "Enable audio from the Audio menu");
    }

    // Download progress, after the macOS NodeDownloadOverlay.
    if (download_ >= 0) {
        p->save();
        QColor scrim = Theme::background;
        scrim.setAlphaF(0.8);
        p->fillPath(rounded, scrim);
        const QPointF c = b.adjusted(0, 0, 0, -kTitleH).center() - QPointF(0, 8);
        const QRectF ring(c.x() - 25, c.y() - 25, 50, 50);
        QColor track = Theme::textTertiary;
        track.setAlphaF(0.3);
        p->setBrush(Qt::NoBrush);
        p->setPen(QPen(track, 4));
        p->drawEllipse(ring);
        p->setPen(QPen(downloadPaused_ ? QColor(0xff, 0x95, 0x00) : Theme::accent, 4, Qt::SolidLine, Qt::RoundCap));
        p->drawArc(ring, 90 * 16, -int(std::clamp(download_, 0.0f, 1.0f) * 360 * 16));
        QFont f = p->font();
        f.setFamily("Consolas");
        f.setPointSizeF(10);
        f.setBold(true);
        p->setFont(f);
        p->setPen(Theme::textPrimary);
        p->drawText(ring, Qt::AlignCenter, QString("%1%").arg(int(download_ * 100)));
        p->restore();
        p->save();
        f = p->font();
        f.setPointSizeF(8);
        p->setFont(f);
        p->setPen(Theme::textSecondary);
        p->drawText(QRectF(b.left(), ring.bottom() + 4, b.width(), 18), Qt::AlignHCenter | Qt::AlignTop,
                    downloadPaused_ ? "Paused" : "Downloading...");
        p->restore();
    }

    // Title bar along the bottom.
    const QRectF bar(b.left(), b.bottom() - kTitleH, b.width(), kTitleH);
    QLinearGradient grad(bar.topLeft(), bar.bottomLeft());
    const int alpha = isSelected() ? 255 : 153;
    grad.setColorAt(0, QColor(0, 0, 0, 0));
    grad.setColorAt(0.45, QColor(0, 0, 0, alpha));
    grad.setColorAt(1, QColor(0, 0, 0, alpha));
    QPainterPath barPath;
    barPath.addRect(bar);
    p->fillPath(barPath.intersected(rounded), grad);

    const QColor dot = !active_ ? kInactiveDot : (isShader_ ? kAccent : kSourceDot);
    p->setPen(Qt::NoPen);
    p->setBrush(dot);
    p->drawEllipse(QPointF(bar.left() + 14, bar.center().y() + 2), 3, 3);
    p->setPen(Qt::white);
    QFont f = p->font(); f.setPointSizeF(9); f.setBold(true); p->setFont(f);
    const qreal textRight = isSelected() ? buttonRect(ToggleActive).left() - 6 : bar.right() - 12;
    const QRectF textRect(bar.left() + 24, bar.top() + 4, textRight - bar.left() - 24, bar.height() - 4);
    p->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
                p->fontMetrics().elidedText(name_, Qt::ElideRight, int(textRect.width())));
    if (isSelected())
        for (Button b : {ToggleActive, Swap, Delete}) paintButton(p, b);

    p->setBrush(Qt::NoBrush);
    if (isSelected()) {
        p->setPen(QPen(QColor(kAccent.red(), kAccent.green(), kAccent.blue(), 70), 8));
        p->drawPath(rounded);
        p->setPen(QPen(kAccent, 2.5));
    } else {
        // A hairline at any zoom, like the macOS 1 pt border.
        QPen hairline(QColor(255, 255, 255, 102), 1);
        hairline.setCosmetic(true);
        p->setPen(hairline);
    }
    p->drawPath(rounded);
    if (dropHover_) {  // a browser drop would chain onto this node
        QColor ring = Theme::success;
        ring.setAlphaF(0.3);
        p->setPen(QPen(ring, 8));
        p->drawPath(rounded);
        p->setPen(QPen(Theme::success, 2.5));
        p->drawPath(rounded);
    }
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionHasChanged) view_->nodeGeometryChanged(this);
    if (change == ItemSelectedHasChanged) update();
    return QGraphicsObject::itemChange(change, value);
}

QRectF NodeItem::buttonRect(Button b) const {
    constexpr qreal size = 20, gap = 4;
    const QRectF bar(body().left(), body().bottom() - kTitleH, kWidth, kTitleH);
    const int fromRight = b == Delete ? 0 : b == Swap ? 1 : 2;
    const qreal right = bar.right() - 8 - fromRight * (size + gap);
    return QRectF(right - size, bar.center().y() + 2 - size / 2, size, size);
}

NodeItem::Button NodeItem::buttonAt(QPointF pos) const {
    if (!isSelected()) return NoButton;
    for (Button b : {ToggleActive, Swap, Delete})
        if (buttonRect(b).contains(pos)) return b;
    return NoButton;
}

void NodeItem::paintButton(QPainter* p, Button b) const {
    const QRectF r = buttonRect(b);
    const QColor fg = b == Delete ? Theme::destructive : Theme::textPrimary;
    p->setPen(Qt::NoPen);
    p->setBrush(hovered_ == b ? QColor(255, 255, 255, 40) : QColor(255, 255, 255, 18));
    p->drawRoundedRect(r, 5, 5);
    const QPointF c = r.center();
    p->setPen(QPen(fg, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p->setBrush(Qt::NoBrush);
    switch (b) {
    case ToggleActive:
        if (active_) {  // pause
            p->drawLine(c + QPointF(-2.5, -4.5), c + QPointF(-2.5, 4.5));
            p->drawLine(c + QPointF(2.5, -4.5), c + QPointF(2.5, 4.5));
        } else {        // play
            p->setBrush(fg);
            p->drawPolygon(QPolygonF({c + QPointF(-3, -5), c + QPointF(5, 0), c + QPointF(-3, 5)}));
        }
        break;
    case Swap:
        p->drawLine(c + QPointF(-5, -2.5), c + QPointF(5, -2.5));
        p->drawLine(c + QPointF(2.5, -5), c + QPointF(5, -2.5));
        p->drawLine(c + QPointF(5, 2.5), c + QPointF(-5, 2.5));
        p->drawLine(c + QPointF(-2.5, 5), c + QPointF(-5, 2.5));
        break;
    case Delete:
        p->drawLine(c + QPointF(-5, -3.5), c + QPointF(5, -3.5));
        p->drawLine(c + QPointF(-1.5, -5.5), c + QPointF(1.5, -5.5));
        p->drawRect(QRectF(c + QPointF(-3.5, -3.5), c + QPointF(3.5, 5.5)));
        break;
    case NoButton:
        break;
    }
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        if (const Button b = buttonAt(e->pos()); b != NoButton) {
            e->accept();
            view_->nodeButtonClicked(this, b, e->screenPos());
            return;
        }
    }
    QGraphicsObject::mousePressEvent(e);
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) {
    if (e->button() == Qt::LeftButton && buttonAt(e->pos()) == NoButton) view_->openPreview(id_);
}

void NodeItem::hoverMoveEvent(QGraphicsSceneHoverEvent* e) {
    const Button b = buttonAt(e->pos());
    if (b != hovered_) { hovered_ = b; update(); }
}

void NodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
    if (hovered_ != NoButton) { hovered_ = NoButton; update(); }
}

// ---------------------------------------------------------------- EdgeItem

EdgeItem::EdgeItem(const QString& id, bool aux) : id_(id), aux_(aux) {
    setFlags(ItemIsSelectable);
    setZValue(0);
}

void EdgeItem::setEnds(PortItem* from, PortItem* to) {
    from_ = from;
    to_ = to;
    updatePath();
}

QPainterPath EdgeItem::curve(QPointF a, QPointF b) {
    const qreal off = qMax(qMax(qAbs(b.x() - a.x()) * 0.5, qAbs(b.y() - a.y()) * 0.25), 60.0);
    QPainterPath path(a);
    path.cubicTo(a + QPointF(off, 0), b - QPointF(off, 0), b);
    return path;
}

void EdgeItem::updatePath() {
    if (from_ && to_) setPath(curve(from_->anchor(), to_->anchor()));
}

// The glow and hit area are sized in screen pixels; this covers them down to
// the minimum zoom (6 px half-width at 0.2x is 30 scene units).
QRectF EdgeItem::boundingRect() const { return path().boundingRect().adjusted(-40, -40, 40, 40); }

QPainterPath EdgeItem::shape() const {
    // A 12 px wide target on screen, whatever the zoom.
    qreal zoom = 1;
    if (scene() && !scene()->views().isEmpty()) zoom = scene()->views().first()->transform().m11();
    QPainterPathStroker stroker;
    stroker.setWidth(12 / std::max(zoom, 0.05));
    return stroker.createStroke(path());
}

void EdgeItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    p->setRenderHint(QPainter::Antialiasing);
    const QColor base = aux_ ? kWarning : kAccent;
    const qreal fade = dimmed_ ? 0.3 : 1.0;
    QColor glow = base, line = base;
    const bool sel = isSelected();
    glow.setAlphaF((sel ? 0.25 : 0.12) * fade);
    line.setAlphaF((sel ? 1.0 : 0.8) * fade);
    p->setBrush(Qt::NoBrush);
    // Screen-pixel widths, as on macOS: cables keep their weight at any zoom.
    QPen glowPen(glow, sel ? 12 : 8, Qt::SolidLine, Qt::RoundCap);
    glowPen.setCosmetic(true);
    p->setPen(glowPen);
    p->drawPath(path());
    QPen pen(line, sel ? 4 : 2.5, Qt::SolidLine, Qt::RoundCap);
    pen.setCosmetic(true);
    if (aux_) pen.setDashPattern({6.0 / pen.widthF(), 4.0 / pen.widthF()});
    p->setPen(pen);
    p->drawPath(path());
    if (dropHover_) {  // the macOS insertion highlight
        QColor warm = Theme::warning;
        warm.setAlphaF(0.25);
        QPen hoverGlow(warm, 10, Qt::SolidLine, Qt::RoundCap);
        hoverGlow.setCosmetic(true);
        p->setPen(hoverGlow);
        p->drawPath(path());
        warm.setAlphaF(0.9);
        QPen hoverLine(warm, 3, Qt::SolidLine, Qt::RoundCap);
        hoverLine.setCosmetic(true);
        p->setPen(hoverLine);
        p->drawPath(path());
    }
}
