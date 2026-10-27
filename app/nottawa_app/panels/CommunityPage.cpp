#include "CommunityPage.h"

#include "Icons.h"
#include "Theme.h"
#include "nodes/NodeEditorView.h"

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QToolButton>
#include <QUuid>
#include <QVBoxLayout>
#include <algorithm>

namespace {
const char* const kSortNames[] = {"Newest", "Popular", "Most Viewed", "Top Rated"};
constexpr int kPreviewHeight = 100;
constexpr int kPageSize = 20;
}

// One shared strand: preview, title, author and a vote row.
class StrandCard : public QWidget {
public:
    StrandCard(CommunityPage* page, const SharedStrand& strand, QWidget* parent)
        : QWidget(parent), page_(page), strand_(strand) {
        setCursor(Qt::PointingHandCursor);
        setToolTip(strand.description);
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(8, 8 + kPreviewHeight + 6, 8, 8);
        layout->setSpacing(2);
        auto* title = new QLabel(strand.title, this);
        title->setStyleSheet(QString("font-weight: 600; color: %1; background: transparent;").arg(Theme::textPrimary.name()));
        layout->addWidget(title);
        if (!strand.author.isEmpty()) {
            auto* author = new QLabel("by " + strand.author, this);
            author->setStyleSheet(QString("font-size: 8pt; color: %1; background: transparent;").arg(Theme::textSecondary.name()));
            layout->addWidget(author);
        }
        auto* votes = new QHBoxLayout;
        votes->setSpacing(8);
        votes->setContentsMargins(0, 4, 0, 0);
        auto voteButton = [&](const QString& direction, int count) {
            auto* b = new QToolButton(this);
            const bool mine = strand.userVote == direction;
            const QColor color = mine ? (direction == "up" ? Theme::success : Theme::destructive) : Theme::textSecondary;
            b->setText(QString("%1 %2").arg(direction == "up" ? "▲" : "▼").arg(count));
            b->setAutoRaise(true);
            b->setToolTip(direction == "up" ? "Upvote" : "Downvote");
            b->setStyleSheet(QString("QToolButton { color: %1; font-size: 8pt; border: none; background: transparent; }"
                                     "QToolButton:hover { color: %2; }").arg(color.name(), Theme::textPrimary.name()));
            connect(b, &QToolButton::clicked, this, [this, direction] { page_->vote(strand_.slug, direction); });
            return b;
        };
        votes->addWidget(voteButton("up", strand.upvotes));
        auto* score = new QLabel(QString::number(strand.score), this);
        score->setStyleSheet(QString("font-weight: 600; color: %1; background: transparent;").arg(Theme::textPrimary.name()));
        votes->addWidget(score);
        votes->addWidget(voteButton("down", strand.downvotes));
        votes->addStretch(1);
        layout->addLayout(votes);
    }

    const SharedStrand& strand() const { return strand_; }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath card;
        card.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 8, 8);
        p.fillPath(card, hovered_ ? Theme::surfaceHover : Theme::surface);
        const QRectF pr(8, 8, width() - 16, kPreviewHeight);
        QPainterPath shape;
        shape.addRoundedRect(pr, 6, 6);
        p.fillPath(shape, Theme::backgroundTertiary);
        const QPixmap image = page_->preview(strand_.previewUrl);
        if (!image.isNull()) {
            const QSizeF size = QSizeF(image.size()).scaled(pr.size(), Qt::KeepAspectRatioByExpanding);
            p.save();
            p.setClipPath(shape);
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            p.drawPixmap(QRectF(pr.center().x() - size.width() / 2, pr.center().y() - size.height() / 2,
                                size.width(), size.height()), image, image.rect());
            p.restore();
        } else {
            Icons::paint(&p, QRectF(pr.center().x() - 12, pr.center().y() - 12, 24, 24), Icons::Stack, Theme::textTertiary);
        }
    }
    void enterEvent(QEnterEvent*) override { hovered_ = true; update(); }
    void leaveEvent(QEvent*) override { hovered_ = false; update(); }
    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) pressPos_ = e->position().toPoint();
    }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (!(e->buttons() & Qt::LeftButton) || pressPos_.isNull()) return;
        if ((e->position().toPoint() - pressPos_).manhattanLength() < QApplication::startDragDistance()) return;
        pressPos_ = QPoint();
        auto* mime = new QMimeData;
        mime->setData(NodeEditorView::kCatalogMime, ("shared_strand:" + strand_.slug).toUtf8());
        auto* drag = new QDrag(this);
        drag->setMimeData(mime);
        drag->setPixmap(grab().scaledToWidth(140, Qt::SmoothTransformation));
        drag->exec(Qt::CopyAction);
    }
    void mouseReleaseEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton && !pressPos_.isNull() && rect().contains(e->position().toPoint()))
            emit page_->loadRequested(strand_.slug);
        pressPos_ = QPoint();
    }
    void contextMenuEvent(QContextMenuEvent* e) override {
        QMenu menu(this);
        const QString slug = strand_.slug;
        menu.addAction("Add to Canvas", this, [this, slug] { emit page_->loadRequested(slug); });
        menu.addAction("Copy Link", this, [slug] {
            QGuiApplication::clipboard()->setText("https://nottawa.app/strands/" + slug);
        });
        menu.exec(e->globalPos());
    }

private:
    CommunityPage* page_;
    SharedStrand   strand_;
    QPoint         pressPos_;
    bool           hovered_ = false;
};

CommunityPage::CommunityPage(QWidget* parent) : QWidget(parent) {
    network_ = new QNetworkAccessManager(this);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* header = new QHBoxLayout;
    header->setContentsMargins(10, 2, 8, 4);
    header->setSpacing(4);
    count_ = new QLabel(this);
    count_->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textSecondary.name()));
    sortButton_ = new QToolButton(this);
    sortButton_->setObjectName("Category");
    sortButton_->setCursor(Qt::PointingHandCursor);
    sortButton_->setIcon(Icons::icon(Icons::ChevronDown, Theme::textTertiary));
    sortButton_->setIconSize(QSize(9, 9));
    sortButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    sortButton_->setLayoutDirection(Qt::RightToLeft);
    sortButton_->setText(kSortNames[sort_]);
    connect(sortButton_, &QToolButton::clicked, this, [this] {
        QMenu menu(this);
        for (int i = 0; i < 4; ++i) {
            QAction* a = menu.addAction(kSortNames[i], this, [this, i] {
                sort_ = Sort(i);
                sortButton_->setText(kSortNames[i]);
                rebuild();
            });
            a->setCheckable(true);
            a->setChecked(i == sort_);
        }
        menu.exec(sortButton_->mapToGlobal(QPoint(0, sortButton_->height())));
    });
    refresh_ = new QToolButton(this);
    refresh_->setIcon(Icons::icon(Icons::Refresh, Theme::textSecondary));
    refresh_->setIconSize(QSize(14, 14));
    refresh_->setAutoRaise(true);
    refresh_->setToolTip("Refresh");
    connect(refresh_, &QToolButton::clicked, this, [this] { fetch(1); });
    header->addWidget(count_);
    header->addStretch(1);
    header->addWidget(sortButton_);
    header->addWidget(refresh_);
    layout->addLayout(header);

    auto* line = new QFrame(this);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    layout->addWidget(line);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list_ = new QWidget(scroll);
    listLayout_ = new QVBoxLayout(list_);
    listLayout_->setContentsMargins(8, 8, 8, 8);
    listLayout_->setSpacing(8);
    status_ = new QLabel(list_);
    status_->setAlignment(Qt::AlignCenter);
    status_->setWordWrap(true);
    status_->setStyleSheet(QString("color: %1; padding: 24px 8px;").arg(Theme::textTertiary.name()));
    loadMore_ = new QPushButton("Load More", list_);
    loadMore_->setCursor(Qt::PointingHandCursor);
    connect(loadMore_, &QPushButton::clicked, this, [this] { fetch(page_ + 1); });
    scroll->setWidget(list_);
    layout->addWidget(scroll, 1);

    auto* engine = EngineBridge::instance();
    connect(engine, &EngineBridge::communityFeedFetched, this,
            [this](bool ok, const QVector<SharedStrand>& strands, int page, int totalPages) {
        loading_ = false;
        refresh_->setEnabled(true);
        if (!ok) {
            if (strands_.isEmpty()) page_ = -1;  // failed: rebuild shows the error
            rebuild();
            return;
        }
        if (page <= 1) strands_.clear();
        strands_ += strands;
        page_ = page;
        totalPages_ = totalPages;
        for (const SharedStrand& s : strands) requestPreview(s.previewUrl);
        rebuild();
    });
    connect(engine, &EngineBridge::strandVoted, this,
            [this](const QString& slug, bool ok, int up, int down, int score, const QString& userVote) {
        if (!ok) return;
        for (SharedStrand& s : strands_) {
            if (s.slug != slug) continue;
            s.upvotes = up; s.downvotes = down; s.score = score; s.userVote = userVote;
        }
        rebuild();
    });
    rebuild();
}

QString CommunityPage::voterId() {
    QSettings settings;
    QString id = settings.value("community/voterId").toString();
    if (id.isEmpty()) {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces).toUpper();
        settings.setValue("community/voterId", id);
    }
    return id;
}

void CommunityPage::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    if (strands_.isEmpty() && !loading_) fetch(1);
}

void CommunityPage::fetch(int page) {
    if (loading_ || !EngineBridge::instance()->isRunning()) return;
    loading_ = true;
    refresh_->setEnabled(false);
    EngineBridge::instance()->fetchCommunityFeed(page, kPageSize, voterId());
    rebuild();
}

// Optimistic, like macOS: update the counts now, correct them when the
// server answers.
void CommunityPage::vote(const QString& slug, const QString& direction) {
    for (SharedStrand& s : strands_) {
        if (s.slug != slug) continue;
        const QString current = s.userVote;
        const bool undo = current == direction;
        if (direction == "up") {
            s.score += undo ? -1 : (current == "down" ? 2 : 1);
            s.upvotes += undo ? -1 : 1;
            if (current == "down") --s.downvotes;
        } else {
            s.score += undo ? 1 : (current == "up" ? -2 : -1);
            s.downvotes += undo ? -1 : 1;
            if (current == "up") --s.upvotes;
        }
        s.userVote = undo ? QString() : direction;
        EngineBridge::instance()->voteOnStrand(slug, voterId(), undo ? QStringLiteral("none") : direction);
    }
    rebuild();
}

void CommunityPage::requestPreview(const QString& url) {
    if (url.isEmpty() || previews_.contains(url)) return;
    previews_.insert(url, QPixmap());
    QNetworkReply* reply = network_->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, [this, reply, url] {
        reply->deleteLater();
        QPixmap image;
        if (reply->error() == QNetworkReply::NoError && image.loadFromData(reply->readAll())) {
            previews_.insert(url, image.width() > 600 ? image.scaledToWidth(600, Qt::SmoothTransformation) : image);
            for (QObject* child : list_->children())
                if (auto* card = dynamic_cast<StrandCard*>(child); card && card->strand().previewUrl == url) card->update();
        }
    });
}

void CommunityPage::rebuild() {
    // Cards are rebuilt wholesale: a page holds at most a few dozen.
    while (QLayoutItem* item = listLayout_->takeAt(0)) {
        if (QWidget* w = item->widget(); w && w != status_ && w != loadMore_) w->deleteLater();
        delete item;
    }

    QVector<SharedStrand> sorted = strands_;
    std::stable_sort(sorted.begin(), sorted.end(), [this](const SharedStrand& a, const SharedStrand& b) {
        switch (sort_) {
        case Newest:     return a.createdAt > b.createdAt;
        case Popular:    return a.opens > b.opens;
        case MostViewed: return a.views > b.views;
        case TopRated:   return a.score > b.score;
        }
        return false;
    });
    for (const SharedStrand& s : sorted) listLayout_->addWidget(new StrandCard(this, s, list_));

    count_->setText(QString("%1 strand%2").arg(strands_.size()).arg(strands_.size() == 1 ? "" : "s"));
    QString status;
    if (loading_) status = "Loading...";
    else if (page_ < 0) status = "Couldn't load community strands.\nCheck your internet connection.";
    else if (strands_.isEmpty() && page_ > 0) status = "No shared strands yet.\nShare your creations with the community!";
    status_->setText(status);
    status_->setVisible(!status.isEmpty());
    listLayout_->addWidget(status_);
    loadMore_->setVisible(!loading_ && page_ > 0 && page_ < totalPages_);
    listLayout_->addWidget(loadMore_);
    listLayout_->addStretch(1);
}
