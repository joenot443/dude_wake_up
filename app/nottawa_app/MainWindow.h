#pragma once
#include <QMainWindow>
#include <QtGlobal>

class QComboBox;
class QDockWidget;
class QFile;
class QLabel;
class QTimer;
class BrowserPanel;
class InspectorPanel;
class LogPanel;
class OFStageEngine;
class StageWidget;
class WelcomeOverlay;

// Qt counterpart of NottawaApp's MainEditorView: toolbar, LIBRARY dock
// (left), INSPECTOR dock (right), LOG dock (bottom), stage in the centre.
// NOTTAWA_LOG=<path> mirrors the log plus a stats line per second to a file.
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
    void selectShader(const QString& name);
    void dismissWelcome();
    void log(const QString& line);

    StageWidget*    stage_        = nullptr;
    WelcomeOverlay* welcome_      = nullptr;
    OFStageEngine*  engine_       = nullptr;

    BrowserPanel*   browser_      = nullptr;
    InspectorPanel* inspector_    = nullptr;
    LogPanel*       logPanel_     = nullptr;
    QComboBox*      shaderPicker_ = nullptr;

    QLabel*         fpsLabel_     = nullptr;
    QLabel*         strandLabel_  = nullptr;
    QTimer*         statsTimer_   = nullptr;
    QFile*          logFile_      = nullptr;
    quint64         lastShown_    = 0;
};
