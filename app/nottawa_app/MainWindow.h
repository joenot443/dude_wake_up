#pragma once

#include "EngineBridge.h"

#include <QHash>
#include <QMainWindow>
#include <QPointer>
#include <QString>
#include <QtGlobal>

class ActionBar;
class BrowserPanel;
class HelpGuide;
class InspectorPanel;
class LogPanel;
class NodeEditorView;
class QAction;
class QFile;
class QLabel;
class QMenu;
class QSplitter;
class QStackedWidget;
class QTimer;
class QToolButton;
class SettingsDialog;
class ShaderCodeEditor;
class StageView;
class Toast;
class WelcomeOverlay;

// Qt counterpart of NottawaApp's MainEditorView, driving the engine through
// EngineBridge: toolbar on top; browser (left), node canvas or stage
// (centre) and inspector (right, while something is selected) in resizable
// panels; the audio panel and an optional log along the bottom.
// NOTTAWA_LOG=<path> mirrors the log plus a stats line per second to a file.
class MainWindow : public QMainWindow {
    Q_OBJECT
    friend class StressTest;  // drives the editor and window for stress runs
public:
    explicit MainWindow(const QString& dataPath);
    ~MainWindow() override;

    // From another launch (see SingleInstance) or this one's command line:
    // a nottawa://strands/<slug> link loads that shared strand; anything
    // else just brings the window to the front.
    void handleExternalRequest(const QString& request);

protected:
    void resizeEvent(QResizeEvent* e) override;
    bool eventFilter(QObject* watched, QEvent* e) override;
    bool handleParameterKey(QObject* watched, class QKeyEvent* e);
    void stepParameter(const ParamInfo& p, int direction, Qt::KeyboardModifiers mods);

private slots:
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void refreshGraph();
    void sampleStats();

private:
    void buildMenus();
    QWidget* buildToolbar();
    void buildPanels();
    void startEngine();
    void setStageMode(bool on);
    void updateStageSource();
    void addFromLibrary(const CatalogItem& item);
    void openShaderEditor(const QString& shaderId, const QString& name);
    void openWorkspace(const QString& path);
    void setWorkspace(const QString& path);
    void rebuildRecentMenu();
    void captureScreenshot();
    void updateChrome();
    void runScriptedAdds();
    void log(const QString& line);

    QString         dataPath_;
    QSplitter*      columns_   = nullptr;  // browser | centre | inspector
    QSplitter*      rows_      = nullptr;  // columns / audio panel / log
    QStackedWidget* centre_    = nullptr;
    NodeEditorView* editor_    = nullptr;
    StageView*      stage_     = nullptr;
    WelcomeOverlay* welcome_   = nullptr;
    BrowserPanel*   browser_   = nullptr;
    InspectorPanel* inspector_ = nullptr;
    QWidget*        audioPanel_ = nullptr;
    LogPanel*       logPanel_  = nullptr;
    ActionBar*      actionBar_ = nullptr;
    Toast*          toast_     = nullptr;
    HelpGuide*      help_      = nullptr;
    QLabel*         fpsOverlay_ = nullptr;

    QToolButton*    undoButton_ = nullptr;
    QToolButton*    redoButton_ = nullptr;
    QToolButton*    sidebarButton_ = nullptr;
    QToolButton*    audioButton_ = nullptr;
    QToolButton*    stageButton_ = nullptr;
    QToolButton*    deleteButton_ = nullptr;
    QWidget*        deleteSeparator_ = nullptr;
    QLabel*         workspaceLabel_ = nullptr;
    QAction*        stageAction_ = nullptr;
    QAction*        hintsAction_ = nullptr;
    QAction*        undoAction_ = nullptr;
    QAction*        redoAction_ = nullptr;
    QMenu*          recentMenu_ = nullptr;
    SettingsDialog* settings_ = nullptr;

    QTimer*         statsTimer_  = nullptr;
    QTimer*         chromeTimer_ = nullptr;
    QFile*          logFile_     = nullptr;
    quint64         lastPainted_ = 0;
    int             uiFps_ = 0;
    int             lastNodeCount_ = -1;
    int             lastEdgeCount_ = -1;
    QString         selectedId_;
    QString         workspacePath_;
    QHash<QString, QPointer<ShaderCodeEditor>> shaderEditors_;
};
