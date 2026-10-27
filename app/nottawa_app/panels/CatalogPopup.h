#pragma once

#include <QFrame>
#include <functional>

class PreviewPool;
class QLineEdit;
class TileGridView;

// A pop-up browser of effects or generated sources with live previews,
// search and category pills, after the macOS NodeInlineBrowserView (the
// node's Swap button) and the effect picker of an Auxiliary Strand.
// Picking a tile calls back with its type and closes the popup.
class CatalogPopup : public QFrame {
    Q_OBJECT
public:
    CatalogPopup(bool effects, std::function<void(int typeRaw)> onPick, QWidget* parent);
    ~CatalogPopup() override;

    // Opens next to `globalPos`, kept on screen.
    void popup(QPoint globalPos);

private:
    void rebuild();

    bool          effects_;
    std::function<void(int)> onPick_;
    QString       category_ = "All";
    QLineEdit*    search_;
    PreviewPool*  pool_;
    TileGridView* grid_;
};
