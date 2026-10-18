#pragma once

#include "FramePresenter.h"

#include <QGraphicsView>
#include <QHash>
#include <QSet>
#include <QPointer>
#include <functional>
#include <QTimer>

class EdgeItem;
class NodeItem;
class PreviewWindow;
class QMenu;
class PortItem;
class QGraphicsLineItem;
class QGraphicsPathItem;

// Node graph canvas over the engine's graph, mirroring NottawaApp's
// NodeEditorView. The engine is the source of truth: edits go through
// EngineBridge and the scene is reconciled from it (keyed by id) on every
// graph change. Node bodies show live previews of each node's output.
//
// Mouse: left-drag moves nodes / marquee-selects (Ctrl adds), drag between
// ports connects (output onto output blends), Alt-drag on empty canvas cuts
// cables, right-drag pans, right-click opens a context menu, wheel zooms.
// Double-click opens a preview window. Keys: Delete/Backspace, Space
// (toggle active), Ctrl+A/C/V, Ctrl+Z, Ctrl+Shift+Z / Ctrl+Y, Ctrl+Tab /
// Ctrl+Shift+Tab (cycle selection).
class NodeEditorView : public QGraphicsView {
    Q_OBJECT
public:
    // Payload of drags from the library: "shader:<type>" or "source:<type>".
    static constexpr const char* kCatalogMime = "application/x-nottawa-catalog";

    explicit NodeEditorView(QWidget* parent = nullptr);
    ~NodeEditorView() override;

    void syncFromEngine();
    void selectNode(const QString& id);
    QStringList selectedNodeIds() const;
    void zoomToFit();

    // Adds a library entry the way the macOS editor does: at `scenePos` if
    // given, else connected after `connectFrom` (300 to its right), else
    // below the last added node, else at the view centre.
    // Adds a text, icon, image (asks for the file) or webcam source.
    QString addInput(int videoSourceType, const QPointF* scenePos = nullptr);
    // Adds a library video source (downloading the video if needed).
    QString addLibrarySource(const QString& libraryFileId, const QPointF* scenePos = nullptr);
    QString addFromCatalog(int typeRaw, bool isSource, const QString& connectFrom = {},
                           const QPointF* scenePos = nullptr);

    // Opens (or raises) a separate window showing the node's output.
    void openPreview(const QString& id);
    // Adds a saved strand, its top-left node at `scenePos` when given.
    void loadStrand(const QString& strandId, const QPointF* scenePos = nullptr);
    // Downloads a community strand and adds it likewise.
    void loadSharedStrand(const QString& slug, const QPointF* scenePos = nullptr);
    // Saves the chain ending at a node as a strand, with a thumbnail.
    void saveStrand(const QString& nodeId);
    // The node's current output as an image (null if it has none yet).
    QImage captureNode(const QString& id);

    // Called by NodeItem.
    void drawNodePreview(QPainter* p, const NodeItem* node, const QRectF& bodyLocal, qreal cornerRadius, float opacity);
    void nodeGeometryChanged(NodeItem* node);
    void nodeButtonClicked(NodeItem* node, int button, QPoint screenPos);

signals:
    void strandSaved(const QString& name);
    void strandLoadFailed(const QString& error);
    void shareRequested(const QString& nodeId, const QString& nodeName);
    void nodeSelected(const QString& id);

protected:
    void wheelEvent(QWheelEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void drawBackground(QPainter* p, const QRectF& rect) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dropEvent(QDropEvent* e) override;

private:
    PortItem* portNear(QPointF scenePos, const PortItem* from) const;
    bool isValidTarget(const PortItem* from, const PortItem* to) const;
    void setDragHighlights(const PortItem* from, const PortItem* snapped);
    NodeItem* nodeAt(const QPoint& viewPos) const;
    EdgeItem* edgeAt(const QPoint& viewPos) const;
    void showContextMenu(const QPoint& viewPos);
    void deleteSelection();
    void commitMovedNodes();
    void finishPortDrag(const QPoint& viewPos);
    void finishCut();
    // Adds catalog entries of one kind to `menu`, grouped like the library.
    void fillCatalogMenu(QMenu* menu, bool sources, const std::function<void(int)>& pick);
    void swapNode(const QString& id, bool isSource, int typeRaw);
    void cycleSelection(bool backwards);
    // Positions, selects and reveals just-loaded strand nodes.
    void placeLoadedStrand(const QStringList& ids, const QPointF* scenePos);
    QPointF nextFreePosition() const;
    // Positions a just-added node, selects it and syncs.
    void placeNew(const QString& id, QPointF pos);

    QHash<QString, NodeItem*> nodes_;
    QHash<QString, EdgeItem*> edges_;
    QHash<QString, QPointer<PreviewWindow>> previews_;
    QString lastAddedId_;
    QHash<QString, QPointF> sharedDrops_;  // slug -> drop point, while downloading

    FramePresenter presenter_;
    QTimer repaintTimer_;

    // Port a connection drag started from, re-resolved on use because a
    // graph refresh mid-drag may rebuild or remove it.
    PortItem* dragFrom() const;
    QString            dragNodeId_;
    int                dragKind_ = 0;
    int                dragSlot_ = 0;
    QGraphicsPathItem* dragEdge_ = nullptr;
    QGraphicsLineItem* cutLine_  = nullptr;
    bool   panning_ = false;
    bool   rightMoved_ = false;
    QPoint rightPress_;
    QPoint panLast_;
    QPoint leftPress_;
    bool   leftOnNode_ = false;
    bool   syncing_ = false;
    bool   fitted_ = false;
    bool   audioActive_ = false;
    QSet<QString> audioReactive_;
};
