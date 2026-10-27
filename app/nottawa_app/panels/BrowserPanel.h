#pragma once

#include "EngineBridge.h"

#include <QWidget>

class PreviewPool;
class QButtonGroup;
class QLineEdit;
class QStackedWidget;
class QToolButton;
class TileGridView;

// Left sidebar, after the macOS SidebarBrowserView: tabs for Sources,
// Effects, Oscillators, Library and Strands. Sources and Effects show a grid
// of live-preview tiles with search, a favourites filter and a category
// picker; tiles drag onto the canvas or add on double-click.
class BrowserPanel : public QWidget {
    Q_OBJECT
public:
    enum Tab { Sources, Effects, Oscillators, Library, Strands };

    explicit BrowserPanel(QWidget* parent = nullptr);
    ~BrowserPanel() override;

    // Re-reads the catalog (call once the engine is running).
    void reload();
    // Effect previews use the selected node's output when it's a source.
    void setSelectedNode(const QString& id, bool isSource);
    // Destroys live previews; call before the engine stops.
    void shutdown();
    void showTab(Tab tab) { setTab(tab); }
    // Replaces the page shown for a tab (Oscillators, Library, Strands).
    void setPage(Tab tab, QWidget* page);

signals:
    void itemPicked(const CatalogItem& item);

private:
    void setTab(Tab tab);
    void rebuild();
    void toggleSearch();
    void showCategoryMenu();

    QButtonGroup*   tabs_ = nullptr;
    QStackedWidget* pages_ = nullptr;
    QWidget*        catalogPage_ = nullptr;
    QWidget*        searchRow_ = nullptr;
    QLineEdit*      search_ = nullptr;
    QToolButton*    searchButton_ = nullptr;
    QToolButton*    favoritesButton_ = nullptr;
    QToolButton*    categoryButton_ = nullptr;
    QToolButton*    liveButton_ = nullptr;
    TileGridView*   grid_ = nullptr;
    PreviewPool*    pool_ = nullptr;
    QWidget*        placeholders_[5] = {};

    Tab tab_ = Effects;
    QVector<CatalogItem> sources_, effects_, inputs_;
    QHash<QString, CatalogItem> byPayload_;
    QString sourceCategory_ = "All", effectCategory_ = "All";
    bool favoriteSourcesOnly_ = false, favoriteEffectsOnly_ = false;
    QString selectedSourceId_;
};
