#pragma once

#include <QWidget>

class QHBoxLayout;
class QToolButton;

// The floating bar in the canvas's bottom-right corner, after the macOS
// ActionBarView: collapses to a chevron; expanded it has undo/redo,
// screenshot, clear and zoom-to-fit.
class ActionBar : public QWidget {
    Q_OBJECT
public:
    explicit ActionBar(QWidget* parent);
    void setUndoRedo(bool canUndo, bool canRedo);
    void setHelpActive(bool on);
    // Keeps it in the parent's bottom-right corner.
    void place();

signals:
    void undo();
    void redo();
    void screenshot();
    void clearAll();
    void zoomToFit();
    void helpToggled();

protected:
    void paintEvent(QPaintEvent*) override;
    bool eventFilter(QObject* watched, QEvent* e) override;

private:
    void setExpanded(bool on);
    QToolButton* button(int icon, const QString& tip);

    QHBoxLayout* layout_;
    QToolButton* toggle_;
    QWidget*     expanded_;
    QToolButton* undo_;
    QToolButton* redo_;
    QToolButton* help_;
    bool         isExpanded_ = true;
};
