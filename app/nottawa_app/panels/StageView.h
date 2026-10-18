#pragma once

#include "EngineBridge.h"

#include <QHash>
#include <QTimer>
#include <QWidget>

class ChannelStrip;
class NodeEditorView;
class QLabel;
class QToolButton;
class QVBoxLayout;
class StageWidget;

// Stage mode, after the macOS StageModeView, for performing: a channel
// strip per node (live thumbnail, Solo, Bypass, compact controls for its
// parameters — all or just favourites), reorderable by dragging; and the
// output monitor (auto = deepest output, or pinned, or the soloed node),
// pop-out, the open preview windows and the current BPM.
class StageView : public QWidget {
    Q_OBJECT
public:
    StageView(NodeEditorView* editor, QWidget* parent = nullptr);

    // Rebuilds the strips (graph changed) — call while shown.
    void rebuild();
    // The node shown on the monitor.
    QString outputId() const;

signals:
    void exitRequested();

protected:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dragLeaveEvent(QDragLeaveEvent* e) override;
    void dropEvent(QDropEvent* e) override;

private:
    friend class ChannelStrip;
    struct Strip {
        GraphNode node;
        int  chain = 0, depth = 0;
        bool terminal = false;
    };
    QVector<Strip> computeStrips() const;
    void refreshValues();
    void updateMonitor();
    void toggleSolo(const QString& id);
    int  dropIndexAt(QPoint pos) const;

    NodeEditorView* editor_;
    QWidget*     emptyState_;
    QWidget*     content_;
    QToolButton* favoritesButton_;
    QLabel*      favoritesLabel_;
    QToolButton* resetOrder_;
    QWidget*     stripList_;
    QVBoxLayout* stripLayout_;
    QWidget*     dropIndicator_;
    QToolButton* outputPicker_;
    QToolButton* soloBadge_;
    StageWidget* monitor_;
    QLabel*      monitorLabel_;
    QWidget*     windowsBox_;
    QVBoxLayout* windowsLayout_;
    QLabel*      bpm_;
    QToolButton* undo_;
    QToolButton* redo_;
    QTimer       refresh_;

    QVector<Strip> strips_;
    QHash<QString, ChannelStrip*> stripWidgets_;
    QString soloId_, pinnedId_;
    bool    showAll_ = true;
    QStringList order_;      // user's strip order (node ids), empty = default
    QString signature_;
    QString windowsSignature_;
};
