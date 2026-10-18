#pragma once

#include "FramePresenter.h"
#include "Icons.h"

#include <QAbstractScrollArea>
#include <QHash>
#include <QImage>
#include <QSet>
#include <QTimer>

class PreviewPool;

// One tile. Its picture is, in order of preference: a live engine preview
// (`live`: the payload is a PreviewPool key), a thumbnail from `imagePath`,
// or `icon` on a tinted background.
struct TileItem {
    QString     name;
    QString     payload;    // identity and drag payload, e.g. "shader:12"
    bool        live = false;
    QString     imagePath;
    Icons::Kind icon = Icons::Sparkles;
    QColor      tint;       // placeholder background; invalid: neutral
    bool        starrable = false;
    bool        favorite  = false;
    float       progress  = -1;     // >= 0: download in progress (0..1)
    bool        cloud     = false;  // not downloaded yet
};

struct TileSection {
    QString title;  // empty: no header (a single flat grid)
    QVector<TileItem> items;
};

// The browser's tile grid: a picture per tile, name underneath, an optional
// favourite star, grouped under collapsible headers that stay pinned while
// their section scrolls. Tiles drag onto the node editor (their payload);
// a click or double-click (see setActivateOnClick) activates one.
class TileGridView : public QAbstractScrollArea {
    Q_OBJECT
public:
    // `pool` may be null when no tile is live.
    TileGridView(PreviewPool* pool, QWidget* parent = nullptr);
    ~TileGridView() override;

    void setSections(const QVector<TileSection>& sections);
    void setActivateOnClick(bool on) { activateOnClick_ = on; }
    // Shown centred when there are no tiles.
    void setEmptyText(const QString& text) { emptyText_ = text; viewport()->update(); }
    void setLivePreview(bool on);
    bool livePreview() const { return live_; }
    // Re-captures still snapshots (live previews refresh themselves).
    void refreshSnapshots();
    void setCollapsed(const QSet<QString>& titles);
    QSet<QString> collapsed() const { return collapsed_; }

signals:
    void activated(const TileItem& item);
    void favoriteToggled(const TileItem& item);
    void contextMenuRequested(const TileItem& item, QPoint globalPos);
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
    const TileItem* itemAt(const Tile& t) const { return &sections_[t.section].items[t.index]; }
    int  tileAt(QPoint viewPos) const;
    int  headerAt(QPoint viewPos) const;
    QRect previewRect(const QRect& tile) const;
    QRect starRect(const QRect& tile) const;
    QRect toView(const QRect& content) const;
    void paintTile(QPainter& p, const Tile& t, bool hovered);
    void paintHeader(QPainter& p, const QRect& r, int section);
    bool drawPreview(QPainter& p, const QString& previewId, const QRect& viewRect);
    QImage capture(const QString& previewId, QSize size);
    void startDrag(const TileItem& item);
    QImage thumbnail(const QString& path);  // async; null until loaded

    PreviewPool*         pool_;
    FramePresenter       presenter_;
    QTimer               repaintTimer_;
    QVector<TileSection> sections_;
    QVector<Header>      headers_;
    QVector<Tile>        tiles_;
    QSet<QString>        collapsed_;
    QHash<QString, qint64> liveSince_;  // preview key -> ms it first showed a frame
    QHash<QString, QImage> thumbnails_; // image path -> scaled image (null while loading)
    QString emptyText_;
    bool  live_ = true;
    bool  activateOnClick_ = false;
    int   contentHeight_ = 0;
    int   hovered_ = -1;
    int   pressed_ = -1;
    QPoint pressPos_;
};
