#pragma once

#include "EngineBridge.h"

#include <QHash>
#include <QMainWindow>
#include <QPointer>
#include <QString>
#include <QtGlobal>

class ActionBar;
class BrowserPanel;
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
class ShaderCodeEditor;
class StageWidget;
class Toast;
class WelcomeOverlay;

// Qt counterpart of NottawaApp's MainEditorView, driving the engine through
// EngineBridge: toolbar on top; browser (left), node canvas or stage
// (centre) and inspector (right, while something is selected) in resizable
// panels; the audio panel and an optional log along the bottom.
// NOTTAWA_LOG=<path> mirrors the log plus a stats line per second to a file.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const QString& dataPath);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent* e) override;
    bool eventFilter(QObject* watched, QEvent* e) override;

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
    StageWidget*    stage_     = nullptr;
    WelcomeOverlay* welcome_   = nullptr;
    BrowserPanel*   browser_   = nullptr;
    InspectorPanel* inspector_ = nullptr;
    QWidget*        audioPanel_ = nullptr;
    LogPanel*       logPanel_  = nullptr;
    ActionBar*      actionBar_ = nullptr;
    Toast*          toast_     = nullptr;
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
    QAction*        undoAction_ = nullptr;
    QAction*        redoAction_ = nullptr;
    QMenu*          recentMenu_ = nullptr;

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
