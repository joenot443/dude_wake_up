#include "NodeEditorView.h"

#include "EngineBridge.h"
#include "NodeItems.h"
#include "PreviewWindow.h"
#include "Theme.h"

#include <QApplication>
#include <QTimer>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QOpenGLFramebufferObject>
#include <QOpenGLWidget>
#include <QScreen>
#include <QScrollBar>
#include <QSet>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

namespace {
constexpr qreal kMinZoom   = 0.2;
constexpr qreal kMaxZoom   = 3.0;
constexpr qreal kGrid      = 40.0;
constexpr qreal kSnap      = 30.0;   // screen px
constexpr int   kClickSlop = 4;      // screen px
constexpr qreal kChainStep = 300.0;
constexpr qreal kStackStep = 228.0;
} // anon

NodeEditorView::NodeEditorView(QWidget* parent) : QGraphicsView(parent) {
    // Live previews are drawn with GL straight into the viewport, which
    // shares textures with the engine through Qt's global share context.
    setViewport(new QOpenGLWidget);
    auto* scene = new QGraphicsScene(this);
    scene->setSceneRect(-50000, -50000, 100000, 100000);
    setScene(scene);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setDragMode(QGraphicsView::RubberBandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::StrongFocus);
    setAcceptDrops(true);
    centerOn(0, 0);

    connect(scene, &QGraphicsScene::selectionChanged, this, [this] {
        if (syncing_) return;
        const QStringList ids = selectedNodeIds();
        emit nodeSelected(ids.isEmpty() ? QString() : ids.first());
        emit selectionChanged(ids);
    });

    repaintTimer_.setTimerType(Qt::PreciseTimer);
    connect(&repaintTimer_, &QTimer::timeout, this, [this] { if (isVisible()) viewport()->update(); });
    // Audio on/off doesn't come through graphChanged, so poll it.
    auto* audioPoll = new QTimer(this);
    connect(audioPoll, &QTimer::timeout, this, [this] {
        const bool audio = EngineBridge::instance()->isAudioActive();
        if (audio == audioActive_) return;
        audioActive_ = audio;
        for (NodeItem* n : std::as_const(nodes_)) n->setAudioHint(!audio && audioReactive_.contains(n->id()));
    });
    audioPoll->start(500);

    connect(EngineBridge::instance(), &EngineBridge::sharedStrandFetched, this,
            [this](const QString& slug, bool ok, const QString& jsonOrError) {
        const bool dropped = sharedDrops_.contains(slug);
        const QPointF at = sharedDrops_.take(slug);
        if (!ok) { emit strandLoadFailed(jsonOrError); return; }
        placeLoadedStrand(EngineBridge::instance()->loadStrandJson(jsonOrError), dropped ? &at : nullptr);
    });
    const qreal hz = screen() ? screen()->refreshRate() : 60.0;
    repaintTimer_.start(qMax(1, int(1000.0 / (hz > 0 ? hz : 60.0))));
}

NodeEditorView::~NodeEditorView() {
    // Deleting the scene removes the selected items, which would announce
    // a selection change to listeners that may already be gone.
    scene()->disconnect(this);
    for (const auto& w : std::as_const(previews_)) if (w) delete w.data();
    for (const QString& id : nodes_.keys()) EngineBridge::instance()->releaseFrames(id);
    if (auto* gl = qobject_cast<QOpenGLWidget*>(viewport())) {
        gl->makeCurrent();
        presenter_.destroy();
        gl->doneCurrent();
    }
}

// ------------------------------------------------------------------ sync

void NodeEditorView::syncFromEngine() {
    auto* engine = EngineBridge::instance();
    if (!engine->isRunning()) return;
    syncing_ = true;

    const auto graphNodes = engine->nodes();
    const auto graphEdges = engine->edges();

    QSet<QString> connectedInputs;  // "<nodeId>/<slot>"
    QSet<QString> connectedOutputs;
    for (const GraphEdge& e : graphEdges) {
        connectedInputs.insert(e.endId + '/' + QString::number(e.inputSlot));
        connectedOutputs.insert(e.startId + '/' + QString::number(e.outputSlot));
    }

    QSet<QString> liveNodes;
    for (const GraphNode& n : graphNodes) {
        liveNodes.insert(n.id);
        NodeItem* item = nodes_.value(n.id);
        if (!item) {
            item = new NodeItem(this, n.id);
            scene()->addItem(item);
            nodes_.insert(n.id, item);
            item->setPos(n.position);
            engine->retainFrames(n.id);
        } else if (!item->isSelected() || QApplication::mouseButtons() == Qt::NoButton) {
            // Don't fight an in-progress drag; otherwise follow the engine
            // (undo, paste and blend creation move nodes).
            if (item->pos() != n.position) item->setPos(n.position);
        }
        item->setInfo(n.name, n.isShader, n.active, n.inputCount, n.hasAuxOutput);
        item->setTypeRaw(n.typeRaw);
        bool anyInput = false;
        for (PortItem* port : item->ports()) {
            const QString key = n.id + '/' + QString::number(port->slot());
            const bool on = port->kind() == PortItem::Input ? connectedInputs.contains(key)
                                                            : connectedOutputs.contains(key);
            port->setConnected(on);
            if (port->kind() == PortItem::Input) anyInput |= on;
        }
        item->setNeedsInput(n.isShader && n.inputCount > 0 && !anyInput);
        if (n.audioReactive) audioReactive_.insert(n.id); else audioReactive_.remove(n.id);
        item->setAudioHint(n.audioReactive && !audioActive_);
    }
    for (auto it = nodes_.begin(); it != nodes_.end();) {
        if (liveNodes.contains(it.key())) { ++it; continue; }
        if (PreviewWindow* w = previews_.take(it.key())) w->close();
        audioReactive_.remove(it.key());
        engine->releaseFrames(it.key());
        delete it.value();
        it = nodes_.erase(it);
    }
    for (auto it = previews_.begin(); it != previews_.end();) {
        if (!it.value()) { it = previews_.erase(it); continue; }
        if (NodeItem* n = nodes_.value(it.key())) it.value()->setNodeName(n->name());
        ++it;
    }

    QSet<QString> liveEdges;
    for (const GraphEdge& e : graphEdges) {
        NodeItem* from = nodes_.value(e.startId);
        NodeItem* to   = nodes_.value(e.endId);
        PortItem* out = from ? from->port(PortItem::Output, e.outputSlot) : nullptr;
        PortItem* in  = to ? to->port(PortItem::Input, e.inputSlot) : nullptr;
        if (!out || !in) continue;
        liveEdges.insert(e.id);
        EdgeItem* edge = edges_.value(e.id);
        if (!edge) {
            edge = new EdgeItem(e.id, e.outputSlot == kAuxOutputSlot);
            scene()->addItem(edge);
            edges_.insert(e.id, edge);
        }
        edge->setEnds(out, in);
    }
    for (auto it = edges_.begin(); it != edges_.end();) {
        if (liveEdges.contains(it.key())) { ++it; continue; }
        delete it.value();
        it = edges_.erase(it);
    }
    syncing_ = false;

    if (!fitted_ && !nodes_.isEmpty()) {
        fitted_ = true;
        zoomToFit();
    }
}

void NodeEditorView::nodeGeometryChanged(NodeItem* node) {
    for (EdgeItem* edge : std::as_const(edges_)) {
        if ((edge->from() && edge->from()->parentItem() == node) ||
            (edge->to() && edge->to()->parentItem() == node))
            edge->updatePath();
    }
}

void NodeEditorView::zoomToFit() {
    if (nodes_.isEmpty()) return;
    QRectF bounds;
    for (NodeItem* n : std::as_const(nodes_)) bounds |= n->sceneBoundingRect();
    bounds.adjust(-60, -60, 60, 60);
    const qreal scaleX = viewport()->width() / bounds.width();
    const qreal scaleY = viewport()->height() / bounds.height();
    const qreal s = qBound(kMinZoom, qMin(scaleX, scaleY), 1.5);
    setTransform(QTransform::fromScale(s, s));
    centerOn(bounds.center());
}

// --------------------------------------------------------------- selection

void NodeEditorView::selectNode(const QString& id) {
    NodeItem* item = nodes_.value(id);
    if (!item || (item->isSelected() && selectedNodeIds().size() == 1)) return;
    syncing_ = true;
    scene()->clearSelection();
    item->setSelected(true);
    syncing_ = false;
}

QStringList NodeEditorView::selectedNodeIds() const {
    QStringList ids;
    for (QGraphicsItem* item : scene()->selectedItems())
        if (auto* node = dynamic_cast<NodeItem*>(item)) ids << node->id();
    return ids;
}

// ---------------------------------------------------------------- previews

void NodeEditorView::drawNodePreview(QPainter* p, const NodeItem* node, const QRectF& bodyLocal, qreal cornerRadius, float opacity) {
    FrameExchange* ex = EngineBridge::instance()->frames(node->id());
    if (!ex) return;
    const QRectF vr = node->deviceTransform(viewportTransform()).mapRect(bodyLocal);
    if (!vr.intersects(viewport()->rect())) return;

    // Round each edge inward so the preview never spills past the node body
    // (the rounded mask and border cover the sub-pixel remainder).
    const qreal dpr = viewport()->devicePixelRatioF();
    const int left   = int(std::ceil(vr.left() * dpr));
    const int right  = int(std::floor(vr.right() * dpr));
    const int top    = int(std::ceil(vr.top() * dpr));
    const int bottom = int(std::floor(vr.bottom() * dpr));
    if (right <= left || bottom <= top) return;
    const int fbHeight = int(std::lround(viewport()->height() * dpr));

    p->beginNativePainting();
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (!presenter_.isInitialized()) presenter_.initialize();
    GLint saved[4];
    gl->glGetIntegerv(GL_VIEWPORT, saved);
    gl->glViewport(left, fbHeight - bottom, right - left, bottom - top);
    const qreal radiusPx = cornerRadius * node->deviceTransform(viewportTransform()).m11() * dpr;
    presenter_.draw(ex, FramePresenter::NodeEditor, opacity, float(radiusPx));
    gl->glViewport(saved[0], saved[1], saved[2], saved[3]);
    p->endNativePainting();
}

// ------------------------------------------------------------- hit testing

PortItem* NodeEditorView::dragFrom() const {
    const NodeItem* node = dragNodeId_.isEmpty() ? nullptr : nodes_.value(dragNodeId_);
    return node ? node->port(PortItem::Kind(dragKind_), dragSlot_) : nullptr;
}

NodeItem* NodeEditorView::nodeAt(const QPoint& viewPos) const {
    for (QGraphicsItem* item : items(viewPos)) {
        if (auto* node = dynamic_cast<NodeItem*>(item)) return node;
        if (auto* port = dynamic_cast<PortItem*>(item)) return static_cast<NodeItem*>(port->parentItem());
    }
    return nullptr;
}

EdgeItem* NodeEditorView::edgeAt(const QPoint& viewPos) const {
    for (QGraphicsItem* item : items(viewPos))
        if (auto* edge = dynamic_cast<EdgeItem*>(item)) return edge;
    return nullptr;
}

bool NodeEditorView::isValidTarget(const PortItem* from, const PortItem* to) const {
    if (!from || !to || from->nodeId() == to->nodeId()) return false;
    if (from->kind() != to->kind()) return true;
    // Output onto output creates a Blend between the two nodes.
    return from->kind() == PortItem::Output && from->slot() == 0 && to->slot() == 0;
}

PortItem* NodeEditorView::portNear(QPointF scenePos, const PortItem* from) const {
    const qreal radius = kSnap / transform().m11();
    PortItem* best = nullptr;
    qreal bestDist = radius;
    for (NodeItem* node : std::as_const(nodes_)) {
        for (PortItem* port : node->ports()) {
            if (from && !isValidTarget(from, port)) continue;
            const qreal d = QLineF(port->anchor(), scenePos).length();
            if (d < bestDist) { bestDist = d; best = port; }
        }
    }
    return best;
}

void NodeEditorView::setDragHighlights(const PortItem* from, const PortItem* snapped) {
    for (NodeItem* node : std::as_const(nodes_))
        for (PortItem* port : node->ports())
            port->setDragState(!from ? 0 : port == snapped ? 2 : isValidTarget(from, port) ? 1 : 0);
    for (EdgeItem* edge : std::as_const(edges_)) edge->setDimmed(from != nullptr);
}

// ------------------------------------------------------------------- mouse

void NodeEditorView::wheelEvent(QWheelEvent* e) {
    const qreal current = transform().m11();
    const qreal target = qBound(kMinZoom, current * std::pow(1.0015, e->angleDelta().y()), kMaxZoom);
    scale(target / current, target / current);
}

void NodeEditorView::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::RightButton) {
        panning_ = true;
        rightMoved_ = false;
        rightPress_ = panLast_ = e->pos();
        return;
    }
    if (e->button() == Qt::LeftButton) {
        leftPress_ = e->pos();
        const QPointF scenePos = mapToScene(e->pos());
        if (PortItem* port = portNear(scenePos, nullptr)) {
            dragNodeId_ = port->nodeId();
            dragKind_ = port->kind();
            dragSlot_ = port->slot();
            dragEdge_ = new QGraphicsPathItem;
            dragEdge_->setPen(QPen(Theme::accent, 2.5, Qt::DashLine));
            dragEdge_->setZValue(3);
            scene()->addItem(dragEdge_);
            setDragHighlights(port, nullptr);
            return;
        }
        if ((e->modifiers() & Qt::AltModifier) && !nodeAt(e->pos())) {
            cutLine_ = scene()->addLine(QLineF(scenePos, scenePos), QPen(Theme::destructive, 2, Qt::DashLine));
            cutLine_->setZValue(3);
            return;
        }
        leftOnNode_ = nodeAt(e->pos()) != nullptr;
        if (!leftOnNode_ && !edgeAt(e->pos()) && !(e->modifiers() & Qt::ControlModifier))
            EngineBridge::instance()->selectNode({});
    }
    QGraphicsView::mousePressEvent(e);
}

void NodeEditorView::mouseMoveEvent(QMouseEvent* e) {
    if (panning_) {
        const QPoint d = e->pos() - panLast_;
        panLast_ = e->pos();
        if ((e->pos() - rightPress_).manhattanLength() > kClickSlop) rightMoved_ = true;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - d.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - d.y());
        return;
    }
    if (dragEdge_) {
        PortItem* from = dragFrom();
        if (!from) return;
        const QPointF scenePos = mapToScene(e->pos());
        PortItem* snapped = portNear(scenePos, from);
        setDragHighlights(from, snapped);
        const QPointF end = snapped ? snapped->anchor() : scenePos;
        dragEdge_->setPath(from->kind() == PortItem::Output ? EdgeItem::curve(from->anchor(), end)
                                                            : EdgeItem::curve(end, from->anchor()));
        return;
    }
    if (cutLine_) {
        cutLine_->setLine(QLineF(cutLine_->line().p1(), mapToScene(e->pos())));
        return;
    }
    QGraphicsView::mouseMoveEvent(e);
}

void NodeEditorView::mouseReleaseEvent(QMouseEvent* e) {
    if (panning_ && e->button() == Qt::RightButton) {
        panning_ = false;
        if (!rightMoved_) showContextMenu(e->pos());
        return;
    }
    if (dragEdge_) { finishPortDrag(e->pos()); return; }
    if (cutLine_)  { finishCut(); return; }
    QGraphicsView::mouseReleaseEvent(e);
    if (e->button() == Qt::LeftButton && leftOnNode_ && (e->pos() - leftPress_).manhattanLength() > kClickSlop)
        commitMovedNodes();
    leftOnNode_ = false;
}

void NodeEditorView::finishPortDrag(const QPoint& viewPos) {
    delete dragEdge_;
    dragEdge_ = nullptr;
    PortItem* from = dragFrom();
    dragNodeId_.clear();
    PortItem* to = from ? portNear(mapToScene(viewPos), from) : nullptr;
    setDragHighlights(nullptr, nullptr);
    if (!from || !to) return;
    auto* engine = EngineBridge::instance();
    if (from->kind() == PortItem::Output && to->kind() == PortItem::Output) {
        const QString blendId = engine->blendBetween(from->nodeId(), to->nodeId());
        const NodeItem* a = nodes_.value(from->nodeId());
        const NodeItem* b = nodes_.value(to->nodeId());
        if (!blendId.isEmpty() && a && b)
            engine->setNodePosition(blendId, QPointF(qMax(a->x(), b->x()) + kChainStep, (a->y() + b->y()) / 2));
        return;
    }
    const PortItem* out = from->kind() == PortItem::Output ? from : to;
    const PortItem* in  = from->kind() == PortItem::Input ? from : to;
    engine->connectNodes(out->nodeId(), in->nodeId(), out->slot(), in->slot());
}

void NodeEditorView::finishCut() {
    QPainterPath cut(cutLine_->line().p1());
    cut.lineTo(cutLine_->line().p2());
    delete cutLine_;
    cutLine_ = nullptr;
    QStringList hit;
    for (EdgeItem* edge : std::as_const(edges_))
        if (edge->path().intersects(cut)) hit << edge->id();
    for (const QString& id : hit) EngineBridge::instance()->removeConnection(id);
}

void NodeEditorView::commitMovedNodes() {
    for (QGraphicsItem* item : scene()->selectedItems())
        if (auto* node = dynamic_cast<NodeItem*>(item))
            EngineBridge::instance()->setNodePosition(node->id(), node->pos());
}

// -------------------------------------------------------------------- keys

void NodeEditorView::keyPressEvent(QKeyEvent* e) {
    auto* engine = EngineBridge::instance();
    const bool ctrl = e->modifiers() & Qt::ControlModifier;
    if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) { deleteSelection(); return; }
    if (e->key() == Qt::Key_Space) {
        for (const QString& id : selectedNodeIds()) engine->setNodeActive(id, !nodes_.value(id)->isActive());
        return;
    }
    if (ctrl && e->key() == Qt::Key_A) {
        for (NodeItem* n : std::as_const(nodes_)) n->setSelected(true);
        return;
    }
    if (ctrl && e->key() == Qt::Key_C) { engine->copyNodes(selectedNodeIds()); return; }
    if (ctrl && e->key() == Qt::Key_V) {
        const QStringList pasted = engine->pasteNodes();
        syncFromEngine();
        scene()->clearSelection();
        for (const QString& id : pasted) if (NodeItem* n = nodes_.value(id)) n->setSelected(true);
        return;
    }
    if (ctrl && e->key() == Qt::Key_Z) { (e->modifiers() & Qt::ShiftModifier) ? engine->redo() : engine->undo(); return; }
    if (ctrl && e->key() == Qt::Key_Y) { engine->redo(); return; }
    if (ctrl && (e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab)) {
        cycleSelection(e->key() == Qt::Key_Backtab || (e->modifiers() & Qt::ShiftModifier));
        return;
    }
    QGraphicsView::keyPressEvent(e);
}

void NodeEditorView::deleteSelection() {
    auto* engine = EngineBridge::instance();
    QStringList edgeIds;
    for (QGraphicsItem* item : scene()->selectedItems())
        if (auto* edge = dynamic_cast<EdgeItem*>(item)) edgeIds << edge->id();
    if (!edgeIds.isEmpty()) {
        for (const QString& id : edgeIds) engine->removeConnection(id);
        return;
    }
    for (const QString& id : selectedNodeIds()) engine->removeNode(id);
}

// ----------------------------------------------------------- context menu

void NodeEditorView::showContextMenu(const QPoint& viewPos) {
    auto* engine = EngineBridge::instance();
    QMenu menu(this);
    const QPointF scenePos = mapToScene(viewPos);

    if (PortItem* port = portNear(scenePos, nullptr)) {
        for (EdgeItem* edge : std::as_const(edges_)) {
            const bool mine = port->kind() == PortItem::Output ? edge->from() == port : edge->to() == port;
            if (!mine) continue;
            const PortItem* other = port->kind() == PortItem::Output ? edge->to() : edge->from();
            const QString name = static_cast<NodeItem*>(other->parentItem())->name();
            const QString id = edge->id();
            menu.addAction(QString("Break Connection %1 %2").arg(port->kind() == PortItem::Output ? "→" : "←", name),
                           this, [engine, id] { engine->removeConnection(id); });
        }
    } else if (NodeItem* node = nodeAt(viewPos)) {
        if (!node->isSelected()) { scene()->clearSelection(); node->setSelected(true); }
        const QString id = node->id();
        const bool isSource = !node->isShader();
        menu.addAction(node->isActive() ? "Pause" : "Resume", this, [engine, id, node] { engine->setNodeActive(id, !node->isActive()); });
        menu.addAction(QString("Delete %1").arg(node->name()), this, [engine, id] { engine->removeNode(id); });
        fillCatalogMenu(menu.addMenu(isSource ? "Swap Source" : "Swap Effect"), isSource,
                        [this, id, isSource](int type) { swapNode(id, isSource, type); });
        menu.addAction("Open Preview Window", this, [this, id] { openPreview(id); });
        if (node->isShader() || node->typeRaw() == 4)  // effects and shader-generated sources
            menu.addAction("Edit Shader Code…", this, [this, id, node] { emit editShaderRequested(id, node->name()); });
        menu.addAction("Save Strand...", this, [this, id] { saveStrand(id); });
        const QString name = node->name();
        menu.addAction("Share Strand...", this, [this, id, name] { emit shareRequested(id, name); });
        menu.addSeparator();
        QList<EdgeItem*> nodeEdges;
        for (EdgeItem* edge : std::as_const(edges_))
            if (edge->from()->parentItem() == node || edge->to()->parentItem() == node) nodeEdges << edge;
        if (!nodeEdges.isEmpty()) {
            QMenu* breakMenu = menu.addMenu("Break Connections");
            for (EdgeItem* edge : nodeEdges) {
                const bool outgoing = edge->from()->parentItem() == node;
                const PortItem* other = outgoing ? edge->to() : edge->from();
                const QString edgeId = edge->id();
                breakMenu->addAction(QString("%1 %2").arg(outgoing ? "→" : "←", static_cast<NodeItem*>(other->parentItem())->name()),
                                     this, [engine, edgeId] { engine->removeConnection(edgeId); });
            }
            if (nodeEdges.size() > 1) {
                QStringList ids;
                for (EdgeItem* edge : nodeEdges) ids << edge->id();
                breakMenu->addSeparator();
                breakMenu->addAction("Break All", this, [engine, ids] { for (const QString& i : ids) engine->removeConnection(i); });
            }
        }
        const QStringList selected = selectedNodeIds();
        if (selected.size() == 2) {
            menu.addAction("Blend Selected", this, [this, engine, selected] {
                const QString blendId = engine->blendBetween(selected[0], selected[1]);
                const NodeItem* a = nodes_.value(selected[0]);
                const NodeItem* b = nodes_.value(selected[1]);
                if (!blendId.isEmpty() && a && b)
                    engine->setNodePosition(blendId, QPointF(qMax(a->x(), b->x()) + kChainStep, (a->y() + b->y()) / 2));
            });
        }
    } else {
        fillCatalogMenu(menu.addMenu("Add Effect"), false,
                        [this, scenePos](int type) { addFromCatalog(type, false, {}, &scenePos); });
        fillCatalogMenu(menu.addMenu("Add Source"), true,
                        [this, scenePos](int type) { addFromCatalog(type, true, {}, &scenePos); });
    }
    if (!menu.isEmpty()) menu.exec(mapToGlobal(viewPos));
}

void NodeEditorView::fillCatalogMenu(QMenu* menu, bool sources, const std::function<void(int)>& pick) {
    auto* engine = EngineBridge::instance();
    QHash<QString, QMenu*> categories;
    for (const CatalogItem& item : sources ? engine->sources() : engine->effects()) {
        QMenu* target = menu;
        if (!sources) {
            QMenu*& sub = categories[item.category];
            if (!sub) sub = menu->addMenu(item.category);
            target = sub;
        }
        const int type = item.typeRaw;
        target->addAction(item.name, this, [pick, type] { pick(type); });
    }
}

// --------------------------------------------------------- node operations

void NodeEditorView::loadStrand(const QString& strandId, const QPointF* scenePos) {
    placeLoadedStrand(EngineBridge::instance()->loadStrand(strandId), scenePos);
}

void NodeEditorView::loadSharedStrand(const QString& slug, const QPointF* scenePos) {
    if (scenePos) sharedDrops_.insert(slug, *scenePos);
    else sharedDrops_.remove(slug);
    EngineBridge::instance()->fetchSharedStrand(slug);
}

void NodeEditorView::placeLoadedStrand(const QStringList& ids, const QPointF* scenePos) {
    auto* engine = EngineBridge::instance();
    if (ids.isEmpty()) return;
    if (scenePos) {
        // Move the strand so its top-left node lands on the drop point.
        QPointF topLeft(qInf(), qInf());
        QHash<QString, QPointF> positions;
        for (const GraphNode& n : engine->nodes()) {
            if (!ids.contains(n.id)) continue;
            positions.insert(n.id, n.position);
            topLeft = QPointF(qMin(topLeft.x(), n.position.x()), qMin(topLeft.y(), n.position.y()));
        }
        for (auto it = positions.cbegin(); it != positions.cend(); ++it)
            engine->setNodePosition(it.key(), it.value() - topLeft + *scenePos);
    }
    syncFromEngine();
    if (!scenePos) {
        // Saved positions can land on existing nodes (e.g. the graph the
        // strand was saved from); then put the strand below the graph.
        QRectF loaded, existing;
        for (auto it = nodes_.cbegin(); it != nodes_.cend(); ++it)
            (ids.contains(it.key()) ? loaded : existing) |= it.value()->sceneBoundingRect();
        bool overlaps = false;
        for (const QString& id : ids) {
            const NodeItem* n = nodes_.value(id);
            if (!n) continue;
            for (auto it = nodes_.cbegin(); it != nodes_.cend() && !overlaps; ++it)
                overlaps = !ids.contains(it.key()) && it.value()->sceneBoundingRect().intersects(n->sceneBoundingRect());
        }
        if (overlaps) {
            const QPointF shift(existing.left() - loaded.left(), existing.bottom() + 60 - loaded.top());
            for (const QString& id : ids)
                if (const NodeItem* n = nodes_.value(id)) engine->setNodePosition(id, n->pos() + shift);
            syncFromEngine();
        }
    }
    scene()->clearSelection();
    for (const QString& id : ids) if (NodeItem* n = nodes_.value(id)) n->setSelected(true);
    if (!scenePos) {
        // Saved positions may be anywhere; bring the new nodes into view.
        QRectF bounds;
        for (const QString& id : ids) if (NodeItem* n = nodes_.value(id)) bounds |= n->sceneBoundingRect();
        if (!bounds.isEmpty() && !mapToScene(viewport()->rect()).boundingRect().contains(bounds)) zoomToFit();
    }
}

void NodeEditorView::saveStrand(const QString& nodeId) {
    auto* engine = EngineBridge::instance();
    const NodeItem* node = nodes_.value(nodeId);
    const QString folder = engine->strandsFolder();
    if (!node || folder.isEmpty()) return;
    QDir().mkpath(folder);
    // A unique file name: "<node>.json", "<node> 2.json", ...
    const QString base = node->name().isEmpty() ? QStringLiteral("Strand") : node->name();
    QString name = base;
    for (int n = 2; QFileInfo::exists(QDir(folder).filePath(name + ".json")); ++n)
        name = QString("%1 %2").arg(base).arg(n);
    const QString path = QDir(folder).filePath(name + ".json");
    QString preview = QDir(folder).filePath(name + ".png");
    if (!captureNode(nodeId).save(preview)) preview.clear();
    if (!engine->saveStrand(nodeId, path, name, preview)) QFile::remove(preview);
    else emit strandSaved(name);
}

QImage NodeEditorView::captureNode(const QString& id, QSize size) {
    FrameExchange* ex = EngineBridge::instance()->frames(id);
    auto* gl = qobject_cast<QOpenGLWidget*>(viewport());
    if (!ex || !gl) return {};
    gl->makeCurrent();
    if (!presenter_.isInitialized()) presenter_.initialize();
    QOpenGLFramebufferObject fbo(size);
    fbo.bind();
    QOpenGLContext::currentContext()->functions()->glViewport(0, 0, size.width(), size.height());
    const bool ok = presenter_.draw(ex, FramePresenter::NodeEditor);
    QImage image = ok ? fbo.toImage().convertToFormat(QImage::Format_RGB32) : QImage();
    fbo.release();
    gl->doneCurrent();
    return image;
}

void NodeEditorView::nodeButtonClicked(NodeItem* node, int button, QPoint screenPos) {
    auto* engine = EngineBridge::instance();
    const QString id = node->id();
    switch (button) {
    case NodeItem::ToggleActive:
        engine->setNodeActive(id, !node->isActive());
        break;
    case NodeItem::Swap: {
        const bool isSource = !node->isShader();
        QMenu menu(this);
        fillCatalogMenu(&menu, isSource, [this, id, isSource](int type) { swapNode(id, isSource, type); });
        menu.exec(screenPos);
        break;
    }
    case NodeItem::Delete:
        engine->removeNode(id);
        break;
    }
}

void NodeEditorView::swapNode(const QString& id, bool isSource, int typeRaw) {
    const QString newId = EngineBridge::instance()->replaceNode(id, typeRaw, isSource);
    if (newId.isEmpty()) return;
    syncFromEngine();
    selectNode(newId);
    emit nodeSelected(newId);
}

void NodeEditorView::openPreview(const QString& id) {
    NodeItem* node = nodes_.value(id);
    if (!node) return;
    QPointer<PreviewWindow>& window = previews_[id];
    if (!window) window = new PreviewWindow(id, node->name(), this->window());
    window->show();
    window->raise();
    window->activateWindow();
}

void NodeEditorView::cycleSelection(bool backwards) {
    if (nodes_.isEmpty()) return;
    QList<NodeItem*> order = nodes_.values();
    std::sort(order.begin(), order.end(), [](const NodeItem* a, const NodeItem* b) {
        return a->y() != b->y() ? a->y() < b->y() : a->x() < b->x();
    });
    const QStringList selected = selectedNodeIds();
    int index = -1;
    if (!selected.isEmpty())
        for (int i = 0; i < order.size(); ++i) if (order[i]->id() == selected.first()) index = i;
    const int n = int(order.size());
    const int next = index < 0 ? (backwards ? n - 1 : 0) : (index + (backwards ? n - 1 : 1)) % n;
    selectNode(order[next]->id());
    emit nodeSelected(order[next]->id());
    ensureVisible(order[next], 40, 40);
}

// ------------------------------------------------------------- adding nodes

QString NodeEditorView::addFromCatalog(int typeRaw, bool isSource, const QString& connectFrom, const QPointF* scenePos) {
    auto* engine = EngineBridge::instance();
    const NodeItem* anchor = connectFrom.isEmpty() || isSource ? nullptr : nodes_.value(connectFrom);
    const QString id = isSource ? engine->addSource(typeRaw)
                                : engine->addEffect(typeRaw, anchor ? connectFrom : QString());
    if (id.isEmpty()) return id;
    QPointF pos;
    if (scenePos) pos = *scenePos;
    else if (anchor) pos = anchor->pos() + QPointF(kChainStep, 0);
    else pos = nextFreePosition();
    placeNew(id, pos);
    return id;
}

QString NodeEditorView::addInput(int type, const QPointF* scenePos) {
    QString path;
    if (type == VideoSourceKind::Image) {
        path = QFileDialog::getOpenFileName(this, "Choose an image file", {},
                                            "Images (*.png *.jpg *.jpeg *.gif *.bmp *.tif *.tiff *.webp)");
        if (path.isEmpty()) return {};
    }
    const QString id = EngineBridge::instance()->addInput(type, path);
    if (id.isEmpty()) return id;
    placeNew(id, scenePos ? *scenePos : nextFreePosition());
    return id;
}

QString NodeEditorView::addLibrarySource(const QString& libraryFileId, const QPointF* scenePos) {
    const QString id = EngineBridge::instance()->addLibrarySource(libraryFileId);
    if (!id.isEmpty()) placeNew(id, scenePos ? *scenePos : nextFreePosition());
    return id;
}

QPointF NodeEditorView::nextFreePosition() const {
    if (const NodeItem* last = nodes_.value(lastAddedId_)) return last->pos() + QPointF(0, kStackStep);
    return mapToScene(viewport()->rect().center());
}

void NodeEditorView::placeNew(const QString& id, QPointF pos) {
    EngineBridge::instance()->setNodePosition(id, pos);
    lastAddedId_ = id;
    syncFromEngine();
    selectNode(id);
    emit nodeSelected(id);
}

// ------------------------------------------------------------ drag & drop

void NodeEditorView::dragEnterEvent(QDragEnterEvent* e) {
    if (e->mimeData()->hasFormat(kCatalogMime)) e->acceptProposedAction();
}

void NodeEditorView::dragMoveEvent(QDragMoveEvent* e) {
    if (e->mimeData()->hasFormat(kCatalogMime)) e->acceptProposedAction();
}

void NodeEditorView::dropEvent(QDropEvent* e) {
    const QString payload = QString::fromUtf8(e->mimeData()->data(kCatalogMime));
    const int colon = payload.indexOf(':');
    if (colon < 0) return;
    const QStringView kind = QStringView(payload).left(colon);
    const bool isSource = kind == u"source";
    const int type = payload.mid(colon + 1).toInt();
    const QPoint viewPos = e->position().toPoint();
    const QPointF scenePos = mapToScene(viewPos);
    e->acceptProposedAction();
    if (kind == u"library") {
        addLibrarySource(payload.mid(colon + 1), &scenePos);
        return;
    }
    if (kind == u"shared_strand") {
        loadSharedStrand(payload.mid(colon + 1), &scenePos);
        return;
    }
    if (kind == u"strand") {
        loadStrand(payload.mid(colon + 1), &scenePos);
        return;
    }
    if (kind == u"input") {
        // Return to the event loop first: the image file dialog is modal.
        QTimer::singleShot(0, this, [this, type, scenePos] { addInput(type, &scenePos); });
        return;
    }

    auto* engine = EngineBridge::instance();
    if (!isSource) {
        if (NodeItem* node = nodeAt(viewPos)) {
            addFromCatalog(type, false, node->id());
            return;
        }
        if (EdgeItem* edge = edgeAt(viewPos)) {
            // Insert into the cable, keeping its original slots.
            const QString startId = edge->from()->nodeId(), endId = edge->to()->nodeId();
            const int outSlot = edge->from()->slot(), inSlot = edge->to()->slot();
            engine->removeConnection(edge->id());
            const QString id = addFromCatalog(type, false, {}, &scenePos);
            if (!id.isEmpty()) {
                engine->connectNodes(startId, id, outSlot, 0);
                engine->connectNodes(id, endId, 0, inSlot);
            }
            return;
        }
    }
    addFromCatalog(type, isSource, {}, &scenePos);
}

// -------------------------------------------------------------- background

void NodeEditorView::drawForeground(QPainter*, const QRectF&) { ++framesPainted_; }

void NodeEditorView::drawBackground(QPainter* p, const QRectF& rect) {
    p->fillRect(rect, Theme::background);
    const qreal scale = transform().m11();
    if (kGrid * scale <= 4.0) return;
    QColor grid = Theme::textTertiary;
    grid.setAlphaF(0.3);
    QPen dot(grid, 3.0, Qt::SolidLine, Qt::RoundCap);
    dot.setCosmetic(true);
    p->setPen(dot);
    QVector<QPointF> points;
    const qreal left = std::floor(rect.left() / kGrid) * kGrid;
    const qreal top  = std::floor(rect.top() / kGrid) * kGrid;
    for (qreal x = left; x < rect.right(); x += kGrid)
        for (qreal y = top; y < rect.bottom(); y += kGrid)
            points.append(QPointF(x, y));
    p->drawPoints(points.constData(), int(points.size()));
}
