#include "MainWindow.h"

#include "EngineBridge.h"
#include "StageWidget.h"
#include "nodes/NodeEditorView.h"
#include "panels/BrowserPanel.h"
#include "panels/InspectorPanel.h"
#include "panels/LogPanel.h"
#include "panels/WelcomeOverlay.h"

#include <QAction>
#include <QComboBox>
#include <QDateTime>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QtGui/qopenglcontext_platform.h>

namespace {
const QString kAutoStage = QStringLiteral("Output (auto)");
}

MainWindow::MainWindow(const QString& dataPath) : dataPath_(dataPath) {
    setWindowTitle("Nottawa");
    resize(1600, 1000);

    const QString logPath = qEnvironmentVariable("NOTTAWA_LOG");
    if (!logPath.isEmpty()) {
        logFile_ = new QFile(logPath, this);
        if (!logFile_->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            delete logFile_;
            logFile_ = nullptr;
        }
    }

    centre_ = new QStackedWidget(this);
    editor_ = new NodeEditorView(centre_);
    stage_  = new StageWidget(centre_);
    centre_->addWidget(editor_);
    centre_->addWidget(stage_);
    setCentralWidget(centre_);

    welcome_ = new WelcomeOverlay(centre_);
    welcome_->setGeometry(centre_->rect());
    connect(welcome_, &WelcomeOverlay::newStrandRequested,  this, [this] { welcome_->hide(); });
    connect(welcome_, &WelcomeOverlay::openStrandRequested, this, &MainWindow::onOpen);

    buildMenusAndToolbar();
    buildDocks();

    strandLabel_ = new QLabel("Untitled");
    statsLabel_  = new QLabel("engine: starting…");
    statusBar()->addWidget(strandLabel_, 1);
    statusBar()->addPermanentWidget(statsLabel_);

    connect(EngineBridge::instance(), &EngineBridge::graphChanged, this, &MainWindow::refreshGraph);
    connect(editor_, &NodeEditorView::nodeSelected, this, [this](const QString& id) {
        selectedId_ = id;
        EngineBridge::instance()->selectNode(id);
        QString name;
        for (const auto& n : EngineBridge::instance()->nodes()) if (n.id == id) name = n.name;
        inspector_->setNode(id, name);
    });

    // After the event loop starts, so the window is up while services load.
    QTimer::singleShot(0, this, &MainWindow::startEngine);

    statsTimer_ = new QTimer(this);
    connect(statsTimer_, &QTimer::timeout, this, &MainWindow::sampleStats);
    statsTimer_->start(1000);
}

MainWindow::~MainWindow() {
    // The views release their shared textures when destroyed; do that while
    // the engine is still running.
    delete centre_;
    EngineBridge::instance()->stop();
}

void MainWindow::log(const QString& line) {
    logPanel_->append(line);
    if (logFile_) {
        logFile_->write((QDateTime::currentDateTime().toString("HH:mm:ss.zzz ") + line + '\n').toUtf8());
        logFile_->flush();
    }
}

void MainWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
    welcome_->setGeometry(centre_->rect());
}

void MainWindow::buildMenusAndToolbar() {
    auto* engine = EngineBridge::instance();
    auto* file = menuBar()->addMenu("&File");
    file->addAction("&New",      this, &MainWindow::onNew,  QKeySequence::New);
    file->addAction("&Open…",    this, &MainWindow::onOpen, QKeySequence::Open);
    file->addAction("&Save As…", this, &MainWindow::onSave, QKeySequence::Save);
    file->addSeparator();
    file->addAction("E&xit", this, &QWidget::close, QKeySequence::Quit);

    auto* view = menuBar()->addMenu("&View");
    stageAction_ = view->addAction("&Stage Mode", QKeySequence("Ctrl+Return"), this, [this](bool on) { setStageMode(on); });
    stageAction_->setCheckable(true);
    view->addAction("Zoom to &Fit", QKeySequence("Ctrl+0"), editor_, &NodeEditorView::zoomToFit);

    auto* help = menuBar()->addMenu("&Help");
    help->addAction("&About Nottawa", this, [this] {
        QMessageBox::about(this, "About Nottawa", "<b>Nottawa</b><br>Audio-reactive visuals for live performance.");
    });

    auto* tb = addToolBar("Main");
    tb->setMovable(false);
    tb->addAction("New",  this, &MainWindow::onNew);
    tb->addAction("Open", this, &MainWindow::onOpen);
    tb->addAction("Save", this, &MainWindow::onSave);
    tb->addSeparator();
    tb->addAction("Undo", this, [engine] { engine->undo(); });
    tb->addAction("Redo", this, [engine] { engine->redo(); });
    tb->addAction("Fit",  editor_, &NodeEditorView::zoomToFit);
    tb->addSeparator();
    tb->addAction(stageAction_);
    stagePick_ = new QComboBox;
    stagePick_->setMinimumWidth(200);
    stagePick_->addItem(kAutoStage);
    connect(stagePick_, &QComboBox::currentIndexChanged, this, &MainWindow::updateStageSource);
    tb->addWidget(stagePick_);
}

void MainWindow::buildDocks() {
    browser_   = new BrowserPanel(this);
    inspector_ = new InspectorPanel(this);
    logPanel_  = new LogPanel(this);
    connect(browser_, &BrowserPanel::itemPicked, this, &MainWindow::addFromLibrary);

    auto addDock = [this](const QString& title, QWidget* w, Qt::DockWidgetArea area) {
        auto* dock = new QDockWidget(title, this);
        dock->setWidget(w);
        dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        addDockWidget(area, dock);
        return dock;
    };
    addDock("LIBRARY",   browser_,   Qt::LeftDockWidgetArea)->setMinimumWidth(240);
    addDock("INSPECTOR", inspector_, Qt::RightDockWidgetArea)->setMinimumWidth(320);
    addDock("LOG",       logPanel_,  Qt::BottomDockWidgetArea)->setMinimumHeight(90);
}

void MainWindow::startEngine() {
    auto* engine = EngineBridge::instance();
    if (engine->isRunning()) return;
    // Every Qt GL context shares with the global share context
    // (AA_ShareOpenGLContexts), so joining it shares engine textures with
    // the canvas and the stage alike.
    QOpenGLContext* shared = QOpenGLContext::globalShareContext();
    auto* wgl = shared ? shared->nativeInterface<QNativeInterface::QWGLContext>() : nullptr;
    if (!wgl) { log("[engine] ERROR no shared GL context"); return; }
    log("[engine] starting, data path " + dataPath_);
    if (!engine->start(wgl->nativeContext(), dataPath_)) {
        log("[engine] ERROR engine failed to initialise");
        return;
    }
    const auto sources = engine->sources();
    const auto effects = engine->effects();
    browser_->populate(sources, effects);
    const QSize res = engine->resolution();
    log(QString("[engine] running at %1x%2 — %3 sources, %4 effects available")
        .arg(res.width()).arg(res.height()).arg(sources.size()).arg(effects.size()));
    refreshGraph();
    editor_->setFocus();
    runScriptedAdds();
}

// NOTTAWA_ADD="Plasma Two;Kaleidoscope" adds catalog entries by name, one per
// second, as library clicks would (effects chain after the previous node) —
// for scripted runs.
void MainWindow::runScriptedAdds() {
    auto* engine = EngineBridge::instance();
    const QStringList names = qEnvironmentVariable("NOTTAWA_ADD").split(';', Qt::SkipEmptyParts);
    for (int i = 0; i < names.size(); ++i) {
        const QString name = names[i].trimmed();
        QTimer::singleShot(1000 * (i + 1), this, [this, engine, name] {
            for (const auto& list : {engine->sources(), engine->effects()}) {
                for (const auto& item : list) {
                    if (item.name.compare(name, Qt::CaseInsensitive) != 0) continue;
                    addFromLibrary(item.typeRaw, item.isSource);
                    log("[script] added " + item.name);
                    return;
                }
            }
            log("[script] ERROR no catalog entry named " + name);
        });
    }
}

void MainWindow::addFromLibrary(int typeRaw, bool isSource) {
    if (!EngineBridge::instance()->isRunning()) return;
    // Effects chain after the selected node, else after the output.
    const QString after = selectedId_.isEmpty() ? EngineBridge::instance()->outputNodeId() : selectedId_;
    if (editor_->addFromCatalog(typeRaw, isSource, after).isEmpty()) log("[graph] ERROR could not add node");
    else welcome_->hide();
}

void MainWindow::refreshGraph() {
    auto* engine = EngineBridge::instance();
    if (!engine->isRunning()) return;
    editor_->syncFromEngine();

    const auto nodes = engine->nodes();
    const int edgeCount = int(engine->edges().size());
    if (nodes.size() != lastNodeCount_ || edgeCount != lastEdgeCount_) {
        lastNodeCount_ = int(nodes.size());
        lastEdgeCount_ = edgeCount;
        log(QString("[graph] %1 nodes, %2 connections").arg(lastNodeCount_).arg(lastEdgeCount_));
    }
    {
        const QSignalBlocker block(stagePick_);
        const QString picked = stagePick_->currentData().toString();
        stagePick_->clear();
        stagePick_->addItem(kAutoStage);
        for (const auto& n : nodes) stagePick_->addItem(n.name, n.id);
        const int idx = picked.isEmpty() ? 0 : stagePick_->findData(picked);
        stagePick_->setCurrentIndex(idx < 0 ? 0 : idx);
    }
    updateStageSource();
    if (!nodes.isEmpty()) welcome_->hide();

    bool selectedExists = false;
    for (const auto& n : nodes) selectedExists |= (n.id == selectedId_);
    if (!selectedExists && !selectedId_.isEmpty()) {
        selectedId_.clear();
        inspector_->setNode({}, {});
    }
}

void MainWindow::setStageMode(bool on) {
    centre_->setCurrentWidget(on ? static_cast<QWidget*>(stage_) : editor_);
    if (!on) editor_->setFocus();
    updateStageSource();
}

// The stage only holds a shared texture while it is shown.
void MainWindow::updateStageSource() {
    const bool shown = centre_->currentWidget() == stage_;
    const QString pinned = stagePick_->currentData().toString();
    stage_->setSourceNode(!shown ? QString() : pinned.isEmpty() ? EngineBridge::instance()->outputNodeId() : pinned);
}

void MainWindow::onNew() {
    if (!EngineBridge::instance()->isRunning()) return;
    EngineBridge::instance()->clearGraph();
    strandLabel_->setText("Untitled");
    log("[file] new graph");
}

void MainWindow::onOpen() {
    const QString path = QFileDialog::getOpenFileName(this, "Open workspace", {}, "Nottawa workspace (*.json);;All files (*)");
    if (path.isEmpty() || !EngineBridge::instance()->isRunning()) return;
    welcome_->hide();
    if (EngineBridge::instance()->loadWorkspace(path)) {
        strandLabel_->setText(QFileInfo(path).fileName());
        log("[file] opened " + path);
        editor_->zoomToFit();
    } else {
        log("[file] ERROR could not open " + path);
    }
}

void MainWindow::onSave() {
    const QString path = QFileDialog::getSaveFileName(this, "Save workspace", {}, "Nottawa workspace (*.json)");
    if (path.isEmpty() || !EngineBridge::instance()->isRunning()) return;
    EngineBridge::instance()->saveWorkspace(path, QFileInfo(path).completeBaseName());
    strandLabel_->setText(QFileInfo(path).fileName());
    log("[file] saved " + path);
}

void MainWindow::sampleStats() {
    const quint64 shown = stage_->framesShown();
    const quint64 perSec = shown - lastShown_;
    lastShown_ = shown;
    QString line = QString("engine %1 fps").arg(EngineBridge::instance()->fps(), 0, 'f', 1);
    if (centre_->currentWidget() == stage_) line += QString(" · stage %1 frames/s").arg(perSec);
    statsLabel_->setText(line);
    if (logFile_) {
        logFile_->write((QDateTime::currentDateTime().toString("HH:mm:ss.zzz ") + "[stats] " + line + '\n').toUtf8());
        logFile_->flush();
    }
}
