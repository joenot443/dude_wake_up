#pragma once
#include <QMainWindow>
#include <QString>
#include <QtGlobal>

class QComboBox;
class QFile;
class QLabel;
class QListWidget;
class QTimer;
class BrowserPanel;
class InspectorPanel;
class LogPanel;
class StageWidget;
class WelcomeOverlay;

// Qt counterpart of NottawaApp's MainEditorView, driving the engine through
// EngineBridge. Toolbar; LIBRARY and GRAPH docks (left); INSPECTOR (right);
// LOG (bottom); stage in the centre showing the graph's output node.
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
    void addFromLibrary(int typeRaw, bool isSource);
    void selectNode(const QString& id);
    void log(const QString& line);

    QString         dataPath_;
    StageWidget*    stage_     = nullptr;
    WelcomeOverlay* welcome_   = nullptr;
    BrowserPanel*   browser_   = nullptr;
    QListWidget*    graph_     = nullptr;
    InspectorPanel* inspector_ = nullptr;
    LogPanel*       logPanel_  = nullptr;
    QComboBox*      stagePick_ = nullptr;

    QLabel*         statsLabel_  = nullptr;
    QLabel*         strandLabel_ = nullptr;
    QTimer*         statsTimer_  = nullptr;
    QFile*          logFile_     = nullptr;
    quint64         lastShown_   = 0;
    QString         selectedId_;
};
