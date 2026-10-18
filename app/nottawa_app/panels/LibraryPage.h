#pragma once

#include <QTimer>
#include <QWidget>

class QLineEdit;
class QToolButton;
class TileGridView;

// The browser's Library tab, after the macOS LibrarySidebarView: videos
// from the nottawa.app library as tiles grouped by category, with search
// and a category picker. Click a video to add it as a source (it downloads
// first if needed, the tile showing progress) or drag it onto the canvas.
class LibraryPage : public QWidget {
    Q_OBJECT
public:
    explicit LibraryPage(QWidget* parent = nullptr);

public slots:
    void reload();

signals:
    void addRequested(const QString& libraryFileId);

protected:
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;

private:
    void rebuild();
    void showCategoryMenu();

    QToolButton*  searchButton_ = nullptr;
    QToolButton*  categoryButton_ = nullptr;
    QWidget*      searchRow_ = nullptr;
    QLineEdit*    search_ = nullptr;
    TileGridView* grid_ = nullptr;
    QTimer        poll_;  // while downloads or thumbnails are pending
    QString       category_ = "All";
    QStringList   categories_;
    bool          fetched_ = false;
    QString       lastState_;      // to notice when polling stops changing anything
    int           idlePolls_ = 0;
};
