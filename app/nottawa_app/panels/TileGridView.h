#pragma once

#include "EngineBridge.h"
#include "FramePresenter.h"

#include <QAbstractScrollArea>
#include <QSet>
#include <QTimer>

class PreviewPool;

struct TileSection {
    QString title;  // empty: no header (a single flat grid)
    QVector<CatalogItem> items;
};

// The browser's grid of catalog tiles: a live preview (or still snapshot)
// per tile, name underneath, favourite star in the corner, grouped under
// collapsible headers that stay pinned while their section scrolls.
// Tiles drag onto the node editor; double-click adds one.
class TileGridView : public QAbstractScrollArea {
    Q_OBJECT
public:
    TileGridView(PreviewPool* pool, QWidget* parent = nullptr);
    ~TileGridView() override;

    void setSections(const QVector<TileSection>& sections);
    void setLivePreview(bool on);
    bool livePreview() const { return live_; }
    // Re-captures still snapshots (live previews refresh themselves).
    void refreshSnapshots();
    void setCollapsed(const QSet<QString>& titles);
    QSet<QString> collapsed() const { return collapsed_; }

signals:
    void activated(const CatalogItem& item);
    void favoriteToggled(const CatalogItem& item);
    void collapsedChanged();

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void scrollContentsBy(int dx, int dy) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;
    bool viewportEvent(QEvent*) override;

private:
    struct Header { QRect rect; int section; };
    struct Tile   { QRect rect; int section; int index; };

    void relayout();
    void updateWanted();
    const CatalogItem* itemAt(const Tile& t) const { return &sections_[t.section].items[t.index]; }
    int  tileAt(QPoint viewPos) const;
    int  headerAt(QPoint viewPos) const;
    QRect previewRect(const QRect& tile) const;
    QRect starRect(const QRect& tile) const;
    QRect toView(const QRect& content) const;
    void paintTile(QPainter& p, const Tile& t, bool hovered);
    void paintHeader(QPainter& p, const QRect& r, int section);
    bool drawPreview(QPainter& p, const QString& previewId, const QRect& viewRect);
    QImage capture(const QString& previewId, QSize size);
    void startDrag(const CatalogItem& item);

    PreviewPool*         pool_;
    FramePresenter       presenter_;
    QTimer               repaintTimer_;
    QVector<TileSection> sections_;
    QVector<Header>      headers_;
    QVector<Tile>        tiles_;
    QSet<QString>        collapsed_;
    QHash<QString, qint64> liveSince_;  // preview key -> ms it first showed a frame
    bool  live_ = true;
    int   contentHeight_ = 0;
    int   hovered_ = -1;
    int   pressed_ = -1;
    QPoint pressPos_;
};
