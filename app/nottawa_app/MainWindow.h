#pragma once
#include <QMainWindow>
class QDockWidget;
class QLabel;
class QTimer;
class StageWidget;
class OFStageEngine;
class QComboBox;
class BrowserPanel;
class InspectorPanel;
class LogPanel;
class WelcomeOverlay;

// Main window — the Qt mirror of NottawaApp's MainEditorView. Toolbar
// top; browser dock left; inspector dock right; log dock bottom; stage
// output centered. Welcome overlay centered on stage until a strand is
// opened.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void onNewStrand();
    void onOpenStrand();
    void onSaveStrand();
    void onToggleStageFullscreen();
    void sampleEngineStats();

private:
    void buildMenus();
    void buildToolbar();
    void buildDocks();
    void buildStatusBar();
    void startEngineOnGlReady();

    StageWidget*     stage_        = nullptr;
    WelcomeOverlay*  welcome_      = nullptr;
    OFStageEngine*   engine_       = nullptr;

    BrowserPanel*    browser_      = nullptr;
    InspectorPanel*  inspector_    = nullptr;
    LogPanel*        logPanel_     = nullptr;

    QDockWidget*     browserDock_  = nullptr;
    QDockWidget*     inspectorDock_= nullptr;
    QDockWidget*     logDock_      = nullptr;

    QLabel*          fpsLabel_     = nullptr;
    QLabel*          strandLabel_  = nullptr;
    QTimer*          statsTimer_   = nullptr;
    QComboBox*       shaderPicker_ = nullptr;

    bool             welcomeDismissed_ = false;
};
