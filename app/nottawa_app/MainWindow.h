#pragma once
#include <QMainWindow>
#include <QString>
#include <QtGlobal>

class QAction;
class QComboBox;
class QFile;
class QLabel;
class QStackedWidget;
class QTimer;
class BrowserPanel;
class InspectorPanel;
class LogPanel;
class NodeEditorView;
class StageWidget;
class WelcomeOverlay;

// Qt counterpart of NottawaApp's MainEditorView, driving the engine through
// EngineBridge: library (left), node canvas or stage (centre), inspector
// (right), log (bottom). Stage mode swaps the canvas for the full output.
// NOTTAWA_LOG=<path> mirrors the log plus a stats line per second to a file.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const QString& dataPath);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent* e) override;

private slots:
    void onNew();
    void onOpen();
    void onSave();
    void refreshGraph();
    void sampleStats();

private:
    void buildMenusAndToolbar();
    void buildDocks();
    void startEngine();
    void setStageMode(bool on);
    void updateStageSource();
    void addFromLibrary(int typeRaw, bool isSource);
    void runScriptedAdds();
    void log(const QString& line);

    QString         dataPath_;
    QStackedWidget* centre_    = nullptr;
    NodeEditorView* editor_    = nullptr;
    StageWidget*    stage_     = nullptr;
    WelcomeOverlay* welcome_   = nullptr;
    BrowserPanel*   browser_   = nullptr;
    InspectorPanel* inspector_ = nullptr;
    LogPanel*       logPanel_  = nullptr;
    QComboBox*      stagePick_ = nullptr;
    QAction*        stageAction_ = nullptr;

    QLabel*         statsLabel_  = nullptr;
    QLabel*         strandLabel_ = nullptr;
    QTimer*         statsTimer_  = nullptr;
    QFile*          logFile_     = nullptr;
    quint64         lastShown_   = 0;
    int             lastNodeCount_ = -1;
    int             lastEdgeCount_ = -1;
    QString         selectedId_;
};
