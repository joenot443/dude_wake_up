#pragma once

#include <QWidget>

class QGraphicsOpacityEffect;
class QLabel;
class TileGridView;
class WelcomeCard;

// The welcome screen, after NottawaApp's WelcomeView: a panel over a
// dimmed canvas with Resume / New Workspace / Open Workspace, bundled
// templates and recent strands, and "Always show at launch". Clicking the
// scrim or pressing Escape dismisses it.
class WelcomeOverlay : public QWidget {
    Q_OBJECT
public:
    explicit WelcomeOverlay(QWidget* parent = nullptr);

    // Whether it should show at launch (remembered setting, default on).
    static bool enabledAtLaunch();
    // Refreshes the strand lists and fades the title in.
    void present();

signals:
    void newRequested();
    void openRequested();
    void strandPicked(const QString& strandId);

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private:
    void layoutPanel();

    QWidget*      panel_;
    QLabel*       title_;
    QLabel*       subtitle_;
    WelcomeCard*  resume_;
    WelcomeCard*  newCard_;
    TileGridView* templates_;
    TileGridView* recent_;
};
