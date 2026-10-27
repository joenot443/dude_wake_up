#include "LibraryPage.h"

#include "EngineBridge.h"
#include "Icons.h"
#include "Theme.h"
#include "TileGridView.h"

#include <QFileInfo>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMap>
#include <QMenu>
#include <QToolButton>
#include <QVBoxLayout>

LibraryPage::LibraryPage(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* controls = new QHBoxLayout;
    controls->setContentsMargins(8, 2, 10, 4);
    controls->setSpacing(2);
    searchButton_ = new QToolButton(this);
    searchButton_->setIcon(Icons::icon(Icons::Search, Theme::textSecondary, Theme::accent));
    searchButton_->setIconSize(QSize(14, 14));
    searchButton_->setCheckable(true);
    searchButton_->setAutoRaise(true);
    searchButton_->setToolTip("Search");
    categoryButton_ = new QToolButton(this);
    categoryButton_->setObjectName("Category");
    categoryButton_->setCursor(Qt::PointingHandCursor);
    categoryButton_->setIcon(Icons::icon(Icons::ChevronDown, Theme::textTertiary));
    categoryButton_->setIconSize(QSize(9, 9));
    categoryButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    categoryButton_->setLayoutDirection(Qt::RightToLeft);
    categoryButton_->setText(category_);
    categoryButton_->hide();
    connect(categoryButton_, &QToolButton::clicked, this, &LibraryPage::showCategoryMenu);
    controls->addWidget(searchButton_);
    controls->addWidget(categoryButton_);
    controls->addStretch(1);
    layout->addLayout(controls);

    searchRow_ = new QWidget(this);
    auto* searchLayout = new QHBoxLayout(searchRow_);
    searchLayout->setContentsMargins(10, 0, 10, 6);
    search_ = new QLineEdit(searchRow_);
    search_->setPlaceholderText("Search library...");
    search_->setClearButtonEnabled(true);
    searchLayout->addWidget(search_);
    searchRow_->hide();
    layout->addWidget(searchRow_);
    connect(searchButton_, &QToolButton::toggled, this, [this](bool on) {
        searchRow_->setVisible(on);
        if (on) search_->setFocus(); else search_->clear();
    });
    connect(search_, &QLineEdit::textChanged, this, &LibraryPage::rebuild);

    auto* line = new QFrame(this);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    layout->addWidget(line);

    grid_ = new TileGridView(nullptr, this);
    grid_->setActivateOnClick(true);
    connect(grid_, &TileGridView::activated, this, [this](const TileItem& tile) {
        emit addRequested(tile.payload.section(':', 1));
        QTimer::singleShot(100, this, &LibraryPage::reload);  // show the download starting
    });
    connect(grid_, &TileGridView::contextMenuRequested, this, [this](const TileItem& tile, QPoint pos) {
        const QString id = tile.payload.section(':', 1);
        QMenu menu(this);
        menu.addAction("Add to Canvas", this, [this, id] { emit addRequested(id); });
        if (tile.progress >= 0) {
            menu.addAction("Pause Download", this, [this, id] {
                EngineBridge::instance()->pauseLibraryDownload(id);
                reload();
            });
        }
        menu.exec(pos);
    });
    layout->addWidget(grid_, 1);

    poll_.setInterval(500);
    connect(&poll_, &QTimer::timeout, this, &LibraryPage::reload);
    connect(EngineBridge::instance(), &EngineBridge::libraryChanged, this, &LibraryPage::reload);
}

void LibraryPage::showEvent(QShowEvent* e) {
    QWidget::showEvent(e);
    auto* engine = EngineBridge::instance();
    const bool failed = fetched_ && idlePolls_ >= 120 && engine->libraryItems().isEmpty();
    if ((!fetched_ || failed) && engine->isRunning()) {
        fetched_ = true;
        engine->fetchLibrary();
    }
    idlePolls_ = 0;
    reload();
}

void LibraryPage::hideEvent(QHideEvent* e) {
    QWidget::hideEvent(e);
    poll_.stop();
}

void LibraryPage::reload() {
    if (isVisible()) rebuild();
}

void LibraryPage::showCategoryMenu() {
    QMenu menu(this);
    for (const QString& c : QStringList{"All"} + categories_) {
        QAction* a = menu.addAction(c, this, [this, c] {
            category_ = c;
            categoryButton_->setText(c);
            rebuild();
        });
        a->setCheckable(true);
        a->setChecked(c == category_);
    }
    menu.exec(categoryButton_->mapToGlobal(QPoint(0, categoryButton_->height())));
}

void LibraryPage::rebuild() {
    const QVector<LibraryItem> items = EngineBridge::instance()->libraryItems();
    const QString text = search_->text().trimmed();

    QMap<QString, TileSection> byCategory;  // sorted by name
    bool pending = !fetched_ || items.isEmpty();
    for (const LibraryItem& f : items) {
        const bool hasThumbnail = !f.thumbnailPath.isEmpty() && QFileInfo::exists(f.thumbnailPath);
        pending |= f.downloading || !hasThumbnail;
        if (!text.isEmpty() && !f.name.contains(text, Qt::CaseInsensitive)) continue;
        if (category_ != "All" && f.category != category_) continue;
        TileItem t;
        t.name = f.name;
        t.payload = "library:" + f.id;
        t.icon = Icons::Film;
        t.tint = Theme::success;
        if (hasThumbnail) t.imagePath = f.thumbnailPath;
        if (f.downloading && !f.paused) t.progress = f.progress;
        t.cloud = !f.downloaded && !f.downloading;
        TileSection& section = byCategory[f.category];
        section.title = category_ == "All" ? f.category : QString();
        section.items.push_back(t);
    }

    categories_ = QStringList();
    for (const LibraryItem& f : items) if (!categories_.contains(f.category)) categories_ << f.category;
    categories_.sort(Qt::CaseInsensitive);
    categoryButton_->setVisible(!categories_.isEmpty());

    // Thumbnails and downloads progress in the background without
    // notifications; poll while any are pending, but give up after a
    // minute of nothing changing (e.g. offline).
    QString state = QString::number(items.size());
    for (const LibraryItem& f : items)
        state += QString("|%1%2%3").arg(f.downloaded).arg(f.progress).arg(QFileInfo::exists(f.thumbnailPath));
    idlePolls_ = state == lastState_ ? idlePolls_ + 1 : 0;
    lastState_ = state;
    const bool gaveUp = idlePolls_ >= 120;
    if (pending && !gaveUp && isVisible()) { if (!poll_.isActive()) poll_.start(); }
    else poll_.stop();

    QString empty = "No videos match your search.";
    if (items.isEmpty()) empty = gaveUp ? "Couldn't load the library.\nCheck your internet connection." : "Loading library...";
    grid_->setEmptyText(empty);
    grid_->setSections(QVector<TileSection>(byCategory.begin(), byCategory.end()));
}
