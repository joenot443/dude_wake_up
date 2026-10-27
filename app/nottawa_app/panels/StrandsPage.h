#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QStackedWidget;
class QToolButton;
class TileGridView;

// The browser's Strands tab, after the macOS StrandsSidebarView: saved
// strands as tiles (thumbnail, name) with search, rename and delete. Click
// a strand to add it to the graph, or drag it onto the canvas.
class StrandsPage : public QWidget {
    Q_OBJECT
public:
    explicit StrandsPage(QWidget* parent = nullptr);
    // Shown in place of the community feed until it's available.
    void setCommunityPage(QWidget* page);
    void showCommunity();

public slots:
    void reload();

signals:
    void loadRequested(const QString& strandId);

protected:
    void showEvent(QShowEvent* e) override;

private:
    void rebuild();
    void setCommunity(bool on);

    QToolButton*    localButton_ = nullptr;
    QToolButton*    communityButton_ = nullptr;
    QStackedWidget* modes_ = nullptr;
    QWidget*        localPage_ = nullptr;
    QWidget*        communityPage_ = nullptr;
    QToolButton*    searchButton_ = nullptr;
    QLabel*         count_ = nullptr;
    QWidget*        searchRow_ = nullptr;
    QLineEdit*      search_ = nullptr;
    TileGridView*   grid_ = nullptr;
};
