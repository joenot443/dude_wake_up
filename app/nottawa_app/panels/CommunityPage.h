#pragma once

#include "EngineBridge.h"

#include <QHash>
#include <QPixmap>
#include <QWidget>

class QLabel;
class QNetworkAccessManager;
class QPushButton;
class QToolButton;
class QVBoxLayout;
class StrandCard;

// The Strands tab's Community view, after the macOS CommunityStrandListView:
// strands shared to nottawa.app as cards (preview, title, author, votes),
// sortable and paged. Click a card to add the strand; drag it onto the
// canvas; vote with the arrows.
class CommunityPage : public QWidget {
    Q_OBJECT
public:
    explicit CommunityPage(QWidget* parent = nullptr);

    // This install's anonymous voter id (generated once).
    static QString voterId();
    void refresh() { fetch(1); }

signals:
    void loadRequested(const QString& slug);

protected:
    void showEvent(QShowEvent* e) override;

private:
    friend class StrandCard;
    enum Sort { Newest, Popular, MostViewed, TopRated };

    void fetch(int page);
    void rebuild();
    void vote(const QString& slug, const QString& direction);
    void requestPreview(const QString& url);
    QPixmap preview(const QString& url) const { return previews_.value(url); }

    QLabel*      count_ = nullptr;
    QToolButton* sortButton_ = nullptr;
    QToolButton* refresh_ = nullptr;
    QWidget*     list_ = nullptr;
    QVBoxLayout* listLayout_ = nullptr;
    QLabel*      status_ = nullptr;
    QPushButton* loadMore_ = nullptr;
    QNetworkAccessManager* network_ = nullptr;

    QVector<SharedStrand> strands_;
    QHash<QString, QPixmap> previews_;  // preview URL -> image (null while loading)
    Sort sort_ = Newest;
    int  page_ = 0, totalPages_ = 0;
    bool loading_ = false;
};
