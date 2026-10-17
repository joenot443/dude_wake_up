#include "TileGridView.h"

#include "Icons.h"
#include "PreviewPool.h"
#include "Theme.h"
#include "nodes/NodeEditorView.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QDrag>
#include <QMenu>
#include <QMimeData>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QScrollBar>
#include <cmath>

namespace {
constexpr int kPad          = 6;   // grid padding
constexpr int kSpacing      = 4;   // between tiles
constexpr int kMinTileWidth = 100;
constexpr int kTilePad      = 6;   // inside a tile
constexpr int kNameHeight   = 14;
constexpr int kHeaderHeight = 22;
constexpr int kTileRadius   = 8;
constexpr int kPreviewRadius = 6;
constexpr int kMaxLive      = 24;  // previews kept rendering at once
constexpr qint64 kSettleMs  = 400; // let a new preview animate before a still capture
const QColor kInputTint(0x3b, 0x82, 0xf6);

Icons::Kind inputIcon(int videoSourceType) {
    switch (videoSourceType) {
    case VideoSourceKind::Text:   return Icons::Text;
    case VideoSourceKind::Icon:   return Icons::Smiley;
    case VideoSourceKind::Image:  return Icons::Photo;
    case VideoSourceKind::Webcam: return Icons::Webcam;
    }
    return Icons::Video;
}
} // anon

TileGridView::TileGridView(PreviewPool* pool, QWidget* parent) : QAbstractScrollArea(parent), pool_(pool) {
    setViewport(new QOpenGLWidget);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    verticalScrollBar()->setSingleStep(24);

    connect(pool_, &PreviewPool::changed, viewport(), qOverload<>(&QWidget::update));
    repaintTimer_.setTimerType(Qt::PreciseTimer);
    connect(&repaintTimer_, &QTimer::timeout, this, [this] { if (isVisible()) viewport()->update(); });
    const qreal hz = screen() ? screen()->refreshRate() : 60.0;
    repaintTimer_.setInterval(qMax(1, int(1000.0 / (hz > 0 ? hz : 60.0))));
}

TileGridView::~TileGridView() {
    if (auto* gl = qobject_cast<QOpenGLWidget*>(viewport())) {
        gl->makeCurrent();
        presenter_.destroy();
        gl->doneCurrent();
    }
}

// ---------------------------------------------------------------- content

void TileGridView::setSections(const QVector<TileSection>& sections) {
    sections_ = sections;
    hovered_ = pressed_ = -1;
    relayout();
}

void TileGridView::setCollapsed(const QSet<QString>& titles) {
    collapsed_ = titles;
    relayout();
}

void TileGridView::setLivePreview(bool on) {
    if (live_ == on) return;
    live_ = on;
    updateWanted();
    viewport()->update();
}

void TileGridView::refreshSnapshots() {
    pool_->clearSnapshots();
    liveSince_.clear();
    updateWanted();
    viewport()->update();
}

// ----------------------------------------------------------------- layout

void TileGridView::relayout() {
    headers_.clear();
    tiles_.clear();
    const int width = viewport()->width();
    const int columns = qMax(1, (width - 2 * kPad + kSpacing) / (kMinTileWidth + kSpacing));
    const int tileW = qMax(1, (width - 2 * kPad - (columns - 1) * kSpacing) / columns);
    const int previewH = int(std::lround((tileW - 2 * kTilePad) * 9.0 / 16.0));
    const int tileH = kTilePad + previewH + 4 + kNameHeight + kTilePad;

    int y = kPad;
    for (int s = 0; s < sections_.size(); ++s) {
        const TileSection& section = sections_[s];
        if (section.items.isEmpty()) continue;
        if (!section.title.isEmpty()) {
            headers_.push_back({QRect(0, y, width, kHeaderHeight), s});
            y += kHeaderHeight + kSpacing;
            if (collapsed_.contains(section.title)) continue;
        }
        for (int i = 0; i < section.items.size(); ++i) {
            const int col = i % columns, row = i / columns;
            tiles_.push_back({QRect(kPad + col * (tileW + kSpacing), y + row * (tileH + kSpacing), tileW, tileH), s, i});
        }
        const int rows = (int(section.items.size()) + columns - 1) / columns;
        y += rows * (tileH + kSpacing) + kSpacing;
    }
    contentHeight_ = y + kPad;
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0, qMax(0, contentHeight_ - viewport()->height()));
    updateWanted();
    viewport()->update();
}

QRect TileGridView::toView(const QRect& content) const {
    return content.translated(0, -verticalScrollBar()->value());
}

QRect TileGridView::previewRect(const QRect& tile) const {
    const int w = tile.width() - 2 * kTilePad;
    return QRect(tile.left() + kTilePad, tile.top() + kTilePad, w, int(std::lround(w * 9.0 / 16.0)));
}

QRect TileGridView::starRect(const QRect& tile) const {
    const QRect pr = previewRect(tile);
    return QRect(pr.right() - 16, pr.top() + 2, 14, 14);
}

// Previews for the tiles on screen (then just below/above), capped so the
// engine doesn't render too many at once. Still mode only wants previews
// for tiles still missing a snapshot.
void TileGridView::updateWanted() {
    const int top = verticalScrollBar()->value();
    const QRect visible(0, top, viewport()->width(), viewport()->height());
    const QRect nearby = visible.adjusted(0, -visible.height() / 2, 0, visible.height() / 2);
    QStringList wanted, later;
    for (const Tile& t : std::as_const(tiles_)) {
        const CatalogItem* item = itemAt(t);
        if (item->isInput) continue;
        const QString key = item->payload();
        if (!live_ && !pool_->snapshot(key).isNull()) continue;
        if (t.rect.intersects(visible)) wanted << key;
        else if (t.rect.intersects(nearby)) later << key;
    }
    wanted += later;
    if (wanted.size() > kMaxLive) wanted = wanted.mid(0, kMaxLive);
    pool_->setWanted(isVisible() ? wanted : QStringList());

    const bool animate = isVisible() && !wanted.isEmpty();
    if (animate && !repaintTimer_.isActive()) repaintTimer_.start();
    else if (!animate) repaintTimer_.stop();
}

// --------------------------------------------------------------- painting

void TileGridView::paintEvent(QPaintEvent*) {
    QPainter p(viewport());
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.fillRect(viewport()->rect(), Theme::backgroundSecondary);

    const int top = verticalScrollBar()->value();
    const QRect visible(0, top, viewport()->width(), viewport()->height());
    bool captured = false;
    for (int i = 0; i < tiles_.size(); ++i) {
        const Tile& t = tiles_[i];
        if (!t.rect.intersects(visible)) continue;
        paintTile(p, t, i == hovered_);

        // Still mode: grab a settled preview into the snapshot cache.
        const CatalogItem* item = itemAt(t);
        if (live_ || item->isInput) continue;
        const QString key = item->payload();
        const QString id = pool_->previewId(key);
        if (id.isEmpty() || !pool_->snapshot(key).isNull()) continue;
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        const qint64 since = liveSince_.value(key, 0);
        if (!since || now - since < kSettleMs) continue;
        p.beginNativePainting();
        const QImage image = capture(id, QSize(320, 180));
        p.endNativePainting();
        if (!image.isNull()) { pool_->setSnapshot(key, image); captured = true; }
    }

    for (const Header& h : std::as_const(headers_))
        if (h.rect.intersects(visible)) paintHeader(p, toView(h.rect), h.section);

    // The header of the section under the top edge stays pinned, pushed up
    // by the next header as it arrives.
    for (int i = int(headers_.size()) - 1; i >= 0; --i) {
        if (headers_[i].rect.top() > top) continue;
        int y = 0;
        if (i + 1 < headers_.size()) y = qMin(0, headers_[i + 1].rect.top() - top - kHeaderHeight);
        paintHeader(p, QRect(0, y, viewport()->width(), kHeaderHeight), headers_[i].section);
        break;
    }
    if (captured) QTimer::singleShot(0, this, &TileGridView::updateWanted);
}

void TileGridView::paintTile(QPainter& p, const Tile& t, bool hovered) {
    const CatalogItem* item = itemAt(t);
    const QRect r = toView(t.rect);
    QPainterPath body;
    body.addRoundedRect(QRectF(r).adjusted(0.5, 0.5, -0.5, -0.5), kTileRadius, kTileRadius);
    p.fillPath(body, hovered ? Theme::surfaceHover : Theme::surface);
    p.setPen(QPen(hovered ? Theme::border : Theme::borderSubtle, 1));
    p.drawPath(body);

    const QRect pr = toView(previewRect(t.rect));
    QPainterPath previewShape;
    previewShape.addRoundedRect(QRectF(pr), kPreviewRadius, kPreviewRadius);
    const QString key = item->payload();
    bool drawn = false;
    if (item->isInput) {
        QColor tint = kInputTint; tint.setAlphaF(0.15);
        p.fillPath(previewShape, tint);
        const int s = qMin(pr.height() - 8, 24);
        Icons::paint(&p, QRectF(pr.center().x() - s / 2.0, pr.center().y() - s / 2.0, s, s),
                     inputIcon(item->typeRaw), kInputTint);
        drawn = true;
    } else {
        p.fillPath(previewShape, Theme::backgroundTertiary);
        // A preview, when one exists, shows in either mode — in still mode
        // only until its snapshot is captured.
        const QString id = pool_->previewId(key);
        const QImage still = pool_->snapshot(key);
        if (!id.isEmpty() && (live_ || still.isNull())) drawn = drawPreview(p, id, pr);
        if (!drawn) {
            if (!still.isNull()) {
                p.save();
                p.setClipPath(previewShape);
                p.drawImage(QRectF(pr), still);
                p.restore();
                drawn = true;
            }
        }
        if (drawn && !id.isEmpty() && !liveSince_.contains(key))
            liveSince_.insert(key, QDateTime::currentMSecsSinceEpoch());
        if (!drawn) {
            Icons::paint(&p, QRectF(pr.center().x() - 7, pr.center().y() - 7, 14, 14),
                         item->isSource ? Icons::Video : Icons::Sparkles, Theme::textTertiary);
        }
    }

    // Favourite star.
    const QRect sr = toView(starRect(t.rect));
    if (item->favorite) {
        Icons::paint(&p, sr, Icons::StarFilled, Theme::warning);
    } else if (hovered) {
        Icons::paint(&p, sr.translated(0, 1), Icons::Star, QColor(0, 0, 0, 130));
        Icons::paint(&p, sr, Icons::Star, QColor(255, 255, 255, 180));
    }

    QFont f = font(); f.setPixelSize(11); p.setFont(f);
    p.setPen(Theme::textPrimary);
    const QRect nameRect(r.left() + 4, pr.bottom() + 4, r.width() - 8, kNameHeight);
    p.drawText(nameRect, Qt::AlignCenter, p.fontMetrics().elidedText(item->name, Qt::ElideRight, nameRect.width()));
}

void TileGridView::paintHeader(QPainter& p, const QRect& r, int section) {
    QColor bg = Theme::backgroundSecondary; bg.setAlphaF(0.97);
    p.fillRect(r, bg);
    const QString& title = sections_[section].title;
    const bool collapsed = collapsed_.contains(title);
    Icons::paint(&p, QRectF(r.left() + 6, r.center().y() - 4.5, 9, 9),
                 collapsed ? Icons::ChevronRight : Icons::ChevronDown, Theme::textTertiary);
    QFont f = font(); f.setPixelSize(11); f.setWeight(QFont::DemiBold); p.setFont(f);
    p.setPen(Theme::textSecondary);
    p.drawText(r.adjusted(20, 0, -6, 0), Qt::AlignVCenter | Qt::AlignLeft, title);
}

// Draws a preview's newest frame into `viewRect`. Returns false until the
// preview has a frame.
bool TileGridView::drawPreview(QPainter& p, const QString& previewId, const QRect& viewRect) {
    FrameExchange* ex = EngineBridge::instance()->frames(previewId);
    if (!ex || viewRect.isEmpty()) return false;
    const qreal dpr = viewport()->devicePixelRatioF();
    const int left   = int(std::ceil(viewRect.left() * dpr));
    const int right  = int(std::floor((viewRect.left() + viewRect.width()) * dpr));
    const int top    = int(std::ceil(viewRect.top() * dpr));
    const int bottom = int(std::floor((viewRect.top() + viewRect.height()) * dpr));
    const int fbHeight = int(std::lround(viewport()->height() * dpr));

    p.beginNativePainting();
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (!presenter_.isInitialized()) presenter_.initialize();
    GLint saved[4];
    gl->glGetIntegerv(GL_VIEWPORT, saved);
    gl->glViewport(left, fbHeight - bottom, right - left, bottom - top);
    const bool ok = presenter_.draw(ex, FramePresenter::Preview, 1.0f, float(kPreviewRadius * dpr));
    gl->glViewport(saved[0], saved[1], saved[2], saved[3]);
    p.endNativePainting();
    return ok;
}

// Renders a preview's current frame into an image. Needs the viewport's
// context current.
QImage TileGridView::capture(const QString& previewId, QSize size) {
    FrameExchange* ex = EngineBridge::instance()->frames(previewId);
    if (!ex) return {};
    QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
    if (!presenter_.isInitialized()) presenter_.initialize();
    GLint saved[4];
    gl->glGetIntegerv(GL_VIEWPORT, saved);
    QOpenGLFramebufferObject fbo(size);
    fbo.bind();
    gl->glViewport(0, 0, size.width(), size.height());
    const bool ok = presenter_.draw(ex, FramePresenter::Preview);
    QImage image = ok ? fbo.toImage().convertToFormat(QImage::Format_RGB32) : QImage();
    fbo.release();
    gl->glViewport(saved[0], saved[1], saved[2], saved[3]);
    return image;
}

// ------------------------------------------------------------ interaction

void TileGridView::resizeEvent(QResizeEvent* e) {
    QAbstractScrollArea::resizeEvent(e);
    relayout();
}

void TileGridView::scrollContentsBy(int, int) {
    updateWanted();
    viewport()->update();
}

bool TileGridView::viewportEvent(QEvent* e) {
    if (e->type() == QEvent::Show || e->type() == QEvent::Hide)
        QTimer::singleShot(0, this, &TileGridView::updateWanted);
    return QAbstractScrollArea::viewportEvent(e);
}

int TileGridView::tileAt(QPoint viewPos) const {
    if (headerAt(viewPos) >= 0) return -1;
    const QPoint content = viewPos + QPoint(0, verticalScrollBar()->value());
    for (int i = 0; i < tiles_.size(); ++i)
        if (tiles_[i].rect.contains(content)) return i;
    return -1;
}

// Index into headers_, counting the pinned header first.
int TileGridView::headerAt(QPoint viewPos) const {
    const int top = verticalScrollBar()->value();
    for (int i = int(headers_.size()) - 1; i >= 0; --i) {
        if (headers_[i].rect.top() > top) continue;
        int y = 0;
        if (i + 1 < headers_.size()) y = qMin(0, headers_[i + 1].rect.top() - top - kHeaderHeight);
        if (viewPos.y() >= y && viewPos.y() < y + kHeaderHeight) return i;
        break;
    }
    for (int i = 0; i < headers_.size(); ++i)
        if (toView(headers_[i].rect).contains(viewPos)) return i;
    return -1;
}

void TileGridView::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;
    pressPos_ = e->position().toPoint();
    if (const int h = headerAt(pressPos_); h >= 0) {
        const QString title = sections_[headers_[h].section].title;
        if (!collapsed_.remove(title)) collapsed_.insert(title);
        // Keep the clicked header where it was when collapsing a pinned one.
        if (headers_[h].rect.top() < verticalScrollBar()->value())
            verticalScrollBar()->setValue(headers_[h].rect.top());
        relayout();
        emit collapsedChanged();
        pressed_ = -1;
        return;
    }
    pressed_ = tileAt(pressPos_);
}

void TileGridView::mouseMoveEvent(QMouseEvent* e) {
    const QPoint pos = e->position().toPoint();
    if (pressed_ >= 0 && (e->buttons() & Qt::LeftButton)
        && (pos - pressPos_).manhattanLength() >= QApplication::startDragDistance()) {
        const CatalogItem item = *itemAt(tiles_[pressed_]);
        pressed_ = -1;
        startDrag(item);
        return;
    }
    const int h = tileAt(pos);
    if (h != hovered_) {
        hovered_ = h;
        setToolTip(h >= 0 ? itemAt(tiles_[h])->name : QString());
        viewport()->setCursor(h >= 0 ? Qt::OpenHandCursor : Qt::ArrowCursor);
        viewport()->update();
    }
}

void TileGridView::mouseReleaseEvent(QMouseEvent* e) {
    const int t = pressed_;
    pressed_ = -1;
    if (t < 0 || e->button() != Qt::LeftButton || tileAt(e->position().toPoint()) != t) return;
    if (toView(starRect(tiles_[t].rect)).adjusted(-3, -3, 3, 3).contains(e->position().toPoint()))
        emit favoriteToggled(*itemAt(tiles_[t]));
}

void TileGridView::mouseDoubleClickEvent(QMouseEvent* e) {
    const int t = tileAt(e->position().toPoint());
    if (t < 0 || toView(starRect(tiles_[t].rect)).adjusted(-3, -3, 3, 3).contains(e->position().toPoint())) return;
    emit activated(*itemAt(tiles_[t]));
}

void TileGridView::leaveEvent(QEvent*) {
    if (hovered_ < 0) return;
    hovered_ = -1;
    viewport()->update();
}

void TileGridView::contextMenuEvent(QContextMenuEvent* e) {
    const int t = tileAt(e->pos());
    if (t < 0) return;
    const CatalogItem item = *itemAt(tiles_[t]);
    QMenu menu(this);
    menu.addAction("Add to Canvas", this, [this, item] { emit activated(item); });
    menu.addAction(item.favorite ? "Unfavorite" : "Favorite", this, [this, item] { emit favoriteToggled(item); });
    menu.exec(e->globalPos());
}

void TileGridView::startDrag(const CatalogItem& item) {
    // Drag image: the tile's current frame (or its icon) with the name.
    QImage frame;
    const QString id = pool_->previewId(item.payload());
    if (!id.isEmpty()) {
        auto* gl = static_cast<QOpenGLWidget*>(viewport());
        gl->makeCurrent();
        frame = capture(id, QSize(160, 90));
        gl->doneCurrent();
    }
    if (frame.isNull()) frame = pool_->snapshot(item.payload());

    const qreal dpr = devicePixelRatioF();
    QPixmap pm(QSize(124, 92) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    {
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath body;
        body.addRoundedRect(QRectF(0.5, 0.5, 123, 91), kTileRadius, kTileRadius);
        p.fillPath(body, Theme::surfaceHover);
        p.setPen(QPen(Theme::accent, 1.5));
        p.drawPath(body);
        const QRectF pr(6, 6, 112, 63);
        QPainterPath shape;
        shape.addRoundedRect(pr, kPreviewRadius, kPreviewRadius);
        p.fillPath(shape, Theme::backgroundTertiary);
        if (!frame.isNull()) {
            p.save(); p.setClipPath(shape); p.drawImage(pr, frame); p.restore();
        } else {
            Icons::paint(&p, QRectF(pr.center().x() - 10, pr.center().y() - 10, 20, 20),
                         item.isInput ? inputIcon(item.typeRaw) : item.isSource ? Icons::Video : Icons::Sparkles,
                         Theme::textSecondary);
        }
        QFont f = font(); f.setPixelSize(11); p.setFont(f);
        p.setPen(Theme::textPrimary);
        p.drawText(QRectF(4, 72, 116, 16), Qt::AlignCenter,
                   p.fontMetrics().elidedText(item.name, Qt::ElideRight, 116));
    }

    auto* mime = new QMimeData;
    mime->setData(NodeEditorView::kCatalogMime, item.payload().toUtf8());
    auto* drag = new QDrag(this);
    drag->setMimeData(mime);
    drag->setPixmap(pm);
    drag->setHotSpot(QPoint(62, 46));
    drag->exec(Qt::CopyAction);
}
