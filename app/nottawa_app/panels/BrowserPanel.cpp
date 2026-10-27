#include "BrowserPanel.h"

#include "Icons.h"
#include "PreviewPool.h"
#include "Theme.h"
#include "TileGridView.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QSettings>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>

namespace {

// The macOS sidebar's order.
const char* const kEffectCategoryOrder[] = {"Basic", "Filter", "Glitch", "Transform", "Mix", "Mask"};

QToolButton* iconButton(Icons::Kind kind, const QString& tip, QWidget* parent) {
    auto* b = new QToolButton(parent);
    b->setIcon(Icons::icon(kind, Theme::textSecondary));
    b->setIconSize(QSize(14, 14));
    b->setToolTip(tip);
    b->setAutoRaise(true);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

TileItem toTile(const CatalogItem& c) {
    TileItem t;
    t.name = c.name;
    t.payload = c.payload();
    t.starrable = true;
    t.favorite = c.favorite;
    if (c.isInput) {
        t.tint = QColor(0x3b, 0x82, 0xf6);
        switch (c.typeRaw) {
        case VideoSourceKind::Text:   t.icon = Icons::Text; break;
        case VideoSourceKind::Icon:   t.icon = Icons::Smiley; break;
        case VideoSourceKind::Image:  t.icon = Icons::Photo; break;
        case VideoSourceKind::Webcam: t.icon = Icons::Webcam; break;
        default:                      t.icon = Icons::Video; break;
        }
    } else {
        t.live = true;
        t.icon = c.isSource ? Icons::Video : Icons::Sparkles;
    }
    return t;
}

QFrame* divider(QWidget* parent) {
    auto* line = new QFrame(parent);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    return line;
}

// Esc clears and closes the search field.
class SearchField : public QLineEdit {
public:
    using QLineEdit::QLineEdit;
    std::function<void()> onEscape;
protected:
    void keyPressEvent(QKeyEvent* e) override {
        if (e->key() == Qt::Key_Escape && onEscape) { onEscape(); return; }
        QLineEdit::keyPressEvent(e);
    }
};

} // anon

BrowserPanel::BrowserPanel(QWidget* parent) : QWidget(parent) {
    setObjectName("BrowserPanel");
    setStyleSheet(QString(R"(
        #BrowserPanel, #BrowserPanel QWidget { background: %1; }
        #BrowserPanel QToolButton { border: none; border-radius: 4px; padding: 3px; color: %2; }
        #BrowserPanel QToolButton:hover { background: %3; }
        #BrowserTabs QToolButton { font-size: 9pt; padding: 4px 2px; }
        #BrowserTabs QToolButton:checked { color: %4; font-weight: 600; background: transparent; }
        #BrowserPanel QToolButton#Category { color: %5; padding: 3px 6px; }
        #BrowserPanel QLineEdit { background: %6; border: 1px solid %7; border-radius: 5px;
                                  padding: 3px 6px; color: %5; }
        #BrowserPanel QLineEdit:focus { border-color: %4; }
    )").arg(Theme::backgroundSecondary.name(), Theme::textSecondary.name(), Theme::surfaceHover.name(),
            Theme::accent.name(), Theme::textPrimary.name(), Theme::backgroundTertiary.name(),
            Theme::border.name()));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Tab bar: icon over label.
    auto* tabBar = new QWidget(this);
    tabBar->setObjectName("BrowserTabs");
    auto* tabLayout = new QHBoxLayout(tabBar);
    tabLayout->setContentsMargins(8, 8, 8, 6);
    tabLayout->setSpacing(0);
    tabs_ = new QButtonGroup(this);
    const struct { Icons::Kind icon; const char* label; } tabDefs[] = {
        {Icons::Video, "Sources"}, {Icons::Sparkles, "Effects"}, {Icons::Waveform, "Osc"},
        {Icons::Books, "Library"}, {Icons::Stack, "Strands"},
    };
    for (int i = 0; i < 5; ++i) {
        auto* b = new QToolButton(tabBar);
        b->setText(tabDefs[i].label);
        b->setIcon(Icons::icon(tabDefs[i].icon, Theme::textSecondary, Theme::accent));
        b->setIconSize(QSize(18, 18));
        b->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        b->setCheckable(true);
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        b->setCursor(Qt::PointingHandCursor);
        tabs_->addButton(b, i);
        tabLayout->addWidget(b);
    }
    connect(tabs_, &QButtonGroup::idClicked, this, [this](int id) { setTab(Tab(id)); });
    layout->addWidget(tabBar);

    pages_ = new QStackedWidget(this);
    layout->addWidget(pages_, 1);

    // Sources / Effects page.
    catalogPage_ = new QWidget(pages_);
    auto* catalog = new QVBoxLayout(catalogPage_);
    catalog->setContentsMargins(0, 0, 0, 0);
    catalog->setSpacing(0);
    auto* controls = new QWidget(catalogPage_);
    auto* row = new QHBoxLayout(controls);
    row->setContentsMargins(8, 2, 8, 4);
    row->setSpacing(2);
    searchButton_ = iconButton(Icons::Search, "Search", controls);
    searchButton_->setIcon(Icons::icon(Icons::Search, Theme::textSecondary, Theme::accent));
    searchButton_->setCheckable(true);
    connect(searchButton_, &QToolButton::clicked, this, &BrowserPanel::toggleSearch);
    favoritesButton_ = iconButton(Icons::Star, "Show Favorites", controls);
    favoritesButton_->setCheckable(true);
    favoritesButton_->setIcon(Icons::toggle(Icons::Star, Theme::textSecondary, Icons::StarFilled, Theme::warning));
    connect(favoritesButton_, &QToolButton::toggled, this, [this](bool on) {
        (tab_ == Sources ? favoriteSourcesOnly_ : favoriteEffectsOnly_) = on;
        favoritesButton_->setToolTip(on ? "Showing Favorites" : "Show Favorites");
        rebuild();
    });
    categoryButton_ = new QToolButton(controls);
    categoryButton_->setObjectName("Category");
    categoryButton_->setCursor(Qt::PointingHandCursor);
    categoryButton_->setIcon(Icons::icon(Icons::ChevronDown, Theme::textTertiary));
    categoryButton_->setIconSize(QSize(9, 9));
    categoryButton_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    categoryButton_->setLayoutDirection(Qt::RightToLeft);
    connect(categoryButton_, &QToolButton::clicked, this, &BrowserPanel::showCategoryMenu);
    auto* refresh = iconButton(Icons::Refresh, "Refresh Previews", controls);
    connect(refresh, &QToolButton::clicked, this, [this] {
        EngineBridge::instance()->setPreferredPreviewSource(selectedSourceId_);
        pool_->clear();
        grid_->refreshSnapshots();
    });
    liveButton_ = new QToolButton(controls);
    liveButton_->setText("Live");
    liveButton_->setCheckable(true);
    liveButton_->setCursor(Qt::PointingHandCursor);
    connect(liveButton_, &QToolButton::toggled, this, [this](bool on) {
        liveButton_->setToolTip(on ? "Live Preview On" : "Live Preview Off");
        liveButton_->setStyleSheet(on ? QString("color: %1; font-weight: 600;").arg(Theme::accent.name()) : QString());
        grid_->setLivePreview(on);
        QSettings().setValue("browser/livePreview", on);
    });
    row->addWidget(searchButton_);
    row->addWidget(favoritesButton_);
    row->addWidget(categoryButton_);
    row->addWidget(refresh);
    row->addStretch(1);
    row->addWidget(liveButton_);
    catalog->addWidget(controls);

    searchRow_ = new QWidget(catalogPage_);
    auto* searchLayout = new QHBoxLayout(searchRow_);
    searchLayout->setContentsMargins(10, 0, 10, 6);
    auto* field = new SearchField(searchRow_);
    field->onEscape = [this] { searchButton_->setChecked(false); toggleSearch(); };
    search_ = field;
    search_->setClearButtonEnabled(true);
    connect(search_, &QLineEdit::textChanged, this, &BrowserPanel::rebuild);
    searchLayout->addWidget(search_);
    searchRow_->hide();
    catalog->addWidget(searchRow_);
    catalog->addWidget(divider(catalogPage_));

    pool_ = new PreviewPool(this);
    grid_ = new TileGridView(pool_, catalogPage_);
    connect(grid_, &TileGridView::activated, this, [this](const TileItem& tile) {
        if (byPayload_.contains(tile.payload)) emit itemPicked(byPayload_[tile.payload]);
    });
    connect(grid_, &TileGridView::favoriteToggled, this, [this](const TileItem& tile) {
        if (!byPayload_.contains(tile.payload)) return;
        EngineBridge::instance()->toggleFavorite(byPayload_[tile.payload]);
        reload();
    });
    connect(grid_, &TileGridView::contextMenuRequested, this, [this](const TileItem& tile, QPoint pos) {
        if (!byPayload_.contains(tile.payload)) return;
        const CatalogItem item = byPayload_[tile.payload];
        QMenu menu(this);
        menu.addAction("Add to Canvas", this, [this, item] { emit itemPicked(item); });
        menu.addAction(item.favorite ? "Unfavorite" : "Favorite", this, [this, item] {
            EngineBridge::instance()->toggleFavorite(item);
            reload();
        });
        menu.exec(pos);
    });
    connect(grid_, &TileGridView::collapsedChanged, this, [this] {
        const QSet<QString> c = grid_->collapsed();
        QSettings().setValue("browser/collapsed", QStringList(c.begin(), c.end()));
    });
    catalog->addWidget(grid_, 1);
    pages_->addWidget(catalogPage_);

    for (const Tab t : {Oscillators, Library, Strands}) setPage(t, nullptr);

    QSettings settings;
    const QStringList collapsed = settings.value("browser/collapsed").toStringList();
    grid_->setCollapsed(QSet<QString>(collapsed.begin(), collapsed.end()));
    const bool live = settings.value("browser/livePreview", true).toBool();
    liveButton_->setChecked(!live);
    liveButton_->setChecked(live);
    setTab(Tab(settings.value("browser/tab", int(Sources)).toInt()));
}

BrowserPanel::~BrowserPanel() = default;

void BrowserPanel::setPage(Tab tab, QWidget* page) {
    if (tab == Sources || tab == Effects) return;
    if (!page) {
        auto* label = new QLabel("Coming soon", pages_);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(QString("color: %1;").arg(Theme::textTertiary.name()));
        page = label;
    }
    QWidget*& slot = placeholders_[tab];
    if (slot) { pages_->removeWidget(slot); slot->deleteLater(); }
    slot = page;
    pages_->addWidget(page);
    if (tab_ == tab) pages_->setCurrentWidget(page);
}

void BrowserPanel::shutdown() { pool_->clear(); }

void BrowserPanel::setSelectedNode(const QString& id, bool isSource) {
    selectedSourceId_ = isSource ? id : QString();
}

void BrowserPanel::reload() {
    auto* engine = EngineBridge::instance();
    if (!engine->isRunning()) return;
    sources_ = engine->sources();
    effects_ = engine->effects();
    inputs_  = engine->inputs();
    byPayload_.clear();
    for (const auto* list : {&sources_, &effects_, &inputs_})
        for (const CatalogItem& c : *list) byPayload_.insert(c.payload(), c);
    rebuild();
}

void BrowserPanel::setTab(Tab tab) {
    const bool changed = tab_ != tab;
    tab_ = tab;
    tabs_->button(tab)->setChecked(true);
    QSettings().setValue("browser/tab", int(tab));
    if (tab == Sources || tab == Effects) {
        pages_->setCurrentWidget(catalogPage_);
        if (changed) {
            search_->clear();
            searchButton_->setChecked(false);
            searchRow_->hide();
        }
        search_->setPlaceholderText(tab == Effects ? "Search effects..." : "Search sources...");
        QSignalBlocker block(favoritesButton_);
        favoritesButton_->setChecked(tab == Sources ? favoriteSourcesOnly_ : favoriteEffectsOnly_);
        rebuild();
    } else {
        pages_->setCurrentWidget(placeholders_[tab]);
        pool_->setWanted({});
    }
}

void BrowserPanel::toggleSearch() {
    const bool on = searchButton_->isChecked();
    searchRow_->setVisible(on);
    if (on) search_->setFocus();
    else search_->clear();
}

void BrowserPanel::showCategoryMenu() {
    QStringList categories{"All"};
    if (tab_ == Effects) {
        for (const char* c : kEffectCategoryOrder) categories << c;
    } else {
        categories << "Input";
        QStringList found;
        for (const CatalogItem& s : std::as_const(sources_))
            if (!s.category.isEmpty() && !found.contains(s.category)) found << s.category;
        found.sort(Qt::CaseInsensitive);
        categories += found;
    }
    QString& current = tab_ == Effects ? effectCategory_ : sourceCategory_;
    QMenu menu(this);
    for (const QString& c : categories) {
        QAction* a = menu.addAction(c, this, [this, &current, c] { current = c; rebuild(); });
        a->setCheckable(true);
        a->setChecked(c == current);
    }
    menu.exec(categoryButton_->mapToGlobal(QPoint(0, categoryButton_->height())));
}

// Builds the grid's sections from the catalog, the category, the favourites
// filter and the search text.
void BrowserPanel::rebuild() {
    if (tab_ != Sources && tab_ != Effects) return;
    const QString text = search_->text().trimmed();
    const bool favoritesOnly = tab_ == Sources ? favoriteSourcesOnly_ : favoriteEffectsOnly_;
    const QString category = tab_ == Effects ? effectCategory_ : sourceCategory_;
    categoryButton_->setText(category);

    auto keep = [&](const CatalogItem& item) {
        return (!favoritesOnly || item.favorite)
            && (text.isEmpty() || item.name.contains(text, Qt::CaseInsensitive));
    };
    // `inCategory` null takes every item.
    auto section = [&](const QString& title, const QVector<CatalogItem>& items, const QString* inCategory) {
        TileSection s{title, {}};
        for (const CatalogItem& item : items)
            if ((!inCategory || item.category == *inCategory) && keep(item)) s.items.push_back(toTile(item));
        return s;
    };

    QVector<TileSection> sections;
    if (tab_ == Effects) {
        if (category == "All") {
            for (const char* c : kEffectCategoryOrder) {
                const QString name(c);
                sections << section(name, effects_, &name);
            }
        } else {
            sections << section({}, effects_, &category);
        }
    } else {
        if (category == "All" || category == "Input")
            sections << section(category == "All" ? "Input" : QString(), inputs_, nullptr);
        if (category == "All") {
            QStringList found;
            for (const CatalogItem& s : std::as_const(sources_))
                if (!found.contains(s.category)) found << s.category;
            found.sort(Qt::CaseInsensitive);
            for (const QString& c : found) sections << section(c.isEmpty() ? "Other" : c, sources_, &c);
        } else if (category != "Input") {
            sections << section({}, sources_, &category);
        }
    }
    grid_->setSections(sections);
}
