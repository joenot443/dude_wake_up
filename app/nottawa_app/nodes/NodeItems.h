#pragma once

#include <QGraphicsObject>
#include <QGraphicsPathItem>
#include <QVector>

class NodeEditorView;

class PortItem : public QGraphicsItem {
public:
    enum Kind { Input, Output };
    PortItem(QGraphicsItem* parent, Kind kind, int slot);

    Kind kind() const { return kind_; }
    int  slot() const { return slot_; }
    QString nodeId() const;
    QPointF anchor() const { return scenePos(); }

    void setConnected(bool on);
    // During a connection drag: 0 = normal, 1 = valid target, 2 = snapped.
    void setDragState(int state);

    QRectF boundingRect() const override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
    Kind kind_;
    int  slot_;
    bool connected_ = false;
    int  dragState_ = 0;
};

// A node centred on its engine position: a live preview of the node's
// output filling the body, a title bar along the bottom, ports on the edges.
class NodeItem : public QGraphicsObject {
    Q_OBJECT
public:
    // Buttons shown in the title bar while the node is selected.
    enum Button { NoButton, ToggleActive, Swap, Delete };

    NodeItem(NodeEditorView* view, const QString& id);

    void setInfo(const QString& name, bool isShader, bool active, int inputCount, bool hasAux);
    void setNeedsInput(bool on);

    const QString& id() const { return id_; }
    const QString& name() const { return name_; }
    bool isShader() const { return isShader_; }
    bool isActive() const { return active_; }
    const QVector<PortItem*>& ports() const { return ports_; }
    PortItem* port(PortItem::Kind kind, int slot) const;

    QRectF boundingRect() const override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override;

    static constexpr qreal kWidth  = 260.0;
    static constexpr qreal kHeight = 146.0;
    static QRectF body() { return QRectF(-kWidth / 2, -kHeight / 2, kWidth, kHeight); }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* e) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* e) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* e) override;

private:
    void rebuildPorts(int inputCount, bool hasAux);
    QRectF buttonRect(Button b) const;
    Button buttonAt(QPointF pos) const;
    void paintButton(QPainter* p, Button b) const;

    NodeEditorView*    view_;
    QString            id_;
    QString            name_;
    bool               isShader_  = false;
    bool               active_    = true;
    bool               needsInput_ = false;
    int                inputCount_ = -1;
    bool               hasAux_    = false;
    Button             hovered_   = NoButton;
    QVector<PortItem*> ports_;
};

class EdgeItem : public QGraphicsPathItem {
public:
    EdgeItem(const QString& id, bool aux);
    const QString& id() const { return id_; }
    void setEnds(PortItem* from, PortItem* to);
    void updatePath();
    PortItem* from() const { return from_; }
    PortItem* to() const { return to_; }
    void setDimmed(bool on) { dimmed_ = on; update(); }

    static QPainterPath curve(QPointF from, QPointF to);

    QPainterPath shape() const override;
    void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) override;

private:
    QString   id_;
    bool      aux_    = false;
    bool      dimmed_ = false;
    PortItem* from_ = nullptr;
    PortItem* to_   = nullptr;
};
