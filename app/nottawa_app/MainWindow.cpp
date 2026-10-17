#include "MainWindow.h"

#include "EngineBridge.h"
#include "StageWidget.h"
#include "panels/BrowserPanel.h"
#include "panels/InspectorPanel.h"
#include "panels/LogPanel.h"
#include "panels/WelcomeOverlay.h"

#include <QComboBox>
#include <QDateTime>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>

namespace {
constexpr int kNodeIdRole = Qt::UserRole;
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

    stage_ = new StageWidget(this);
    setCentralWidget(stage_);
    welcome_ = new WelcomeOverlay(stage_);
    welcome_->setGeometry(stage_->rect());
    connect(welcome_, &WelcomeOverlay::newStrandRequested,  this, [this] { welcome_->hide(); });
    connect(welcome_, &WelcomeOverlay::openStrandRequested, this, &MainWindow::onOpen);

    buildMenusAndToolbar();
    buildDocks();

    strandLabel_ = new QLabel("Untitled");
    statsLabel_  = new QLabel("engine: starting…");
    statusBar()->addWidget(strandLabel_, 1);
    statusBar()->addPermanentWidget(statsLabel_);

    connect(EngineBridge::instance(), &EngineBridge::graphChanged, this, &MainWindow::refreshGraph);
    // The engine needs the stage's GL context to share textures with; it
    // exists once initializeGL has run.
    connect(stage_, &StageWidget::glReady, this, &MainWindow::startEngine, Qt::QueuedConnection);

    statsTimer_ = new QTimer(this);
    connect(statsTimer_, &QTimer::timeout, this, &MainWindow::sampleStats);
    statsTimer_->start(1000);
}

MainWindow::~MainWindow() {
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
    welcome_->setGeometry(stage_->rect());
}

void MainWindow::buildMenusAndToolbar() {
    auto* file = menuBar()->addMenu("&File");
    file->addAction("&New",        this, &MainWindow::onNew,  QKeySequence::New);
    file->addAction("&Open…",      this, &MainWindow::onOpen, QKeySequence::Open);
    file->addAction("&Save As…",   this, &MainWindow::onSave, QKeySequence::Save);
    file->addSeparator();
    file->addAction("E&xit", this, &QWidget::close, QKeySequence::Quit);

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
    tb->addWidget(new QLabel("  Stage: "));
    stagePick_ = new QComboBox;
    stagePick_->setMinimumWidth(220);
    stagePick_->addItem(kAutoStage);
    connect(stagePick_, &QComboBox::currentIndexChanged, this, &MainWindow::refreshGraph);
    tb->addWidget(stagePick_);
}

void MainWindow::buildDocks() {
    browser_   = new BrowserPanel(this);
    graph_     = new QListWidget(this);
    inspector_ = new InspectorPanel(this);
    logPanel_  = new LogPanel(this);

    connect(browser_, &BrowserPanel::itemPicked, this, &MainWindow::addFromLibrary);
    connect(graph_, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* item) {
        if (item) selectNode(item->data(kNodeIdRole).toString());
    });

    auto addDock = [this](const QString& title, QWidget* w, Qt::DockWidgetArea area) {
        auto* dock = new QDockWidget(title, this);
        dock->setWidget(w);
        dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        addDockWidget(area, dock);
        return dock;
    };
    addDock("LIBRARY",   browser_,   Qt::LeftDockWidgetArea)->setMinimumWidth(240);
    addDock("GRAPH",     graph_,     Qt::LeftDockWidgetArea);
    addDock("INSPECTOR", inspector_, Qt::RightDockWidgetArea)->setMinimumWidth(320);
    addDock("LOG",       logPanel_,  Qt::BottomDockWidgetArea)->setMinimumHeight(120);
}

void MainWindow::startEngine() {
    auto* engine = EngineBridge::instance();
    if (engine->isRunning()) return;
    if (!stage_->nativeContext()) { log("[engine] ERROR stage GL context unavailable"); return; }
    log("[engine] starting, data path " + dataPath_);
    if (!engine->start(stage_->nativeContext(), dataPath_)) {
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

    // NOTTAWA_ADD="Plasma Two;Kaleidoscope" adds catalog entries by name, one
    // per second, as library clicks would — for scripted test runs.
    const QStringList names = qEnvironmentVariable("NOTTAWA_ADD").split(';', Qt::SkipEmptyParts);
    for (int i = 0; i < names.size(); ++i) {
        const QString name = names[i].trimmed();
        QTimer::singleShot(1000 * (i + 1), this, [this, name, sources, effects] {
            for (const auto* list : {&sources, &effects}) {
                for (const auto& item : *list) {
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
    auto* engine = EngineBridge::instance();
    if (!engine->isRunning()) return;
    const QString id = isSource ? engine->addSource(typeRaw) : engine->addEffect(typeRaw, engine->outputNodeId());
    if (id.isEmpty()) { log("[graph] ERROR could not add node"); return; }
    welcome_->hide();
    selectedId_ = id;
}

void MainWindow::refreshGraph() {
    auto* engine = EngineBridge::instance();
    if (!engine->isRunning()) return;
    const auto nodes = engine->nodes();
    const QString outputId = engine->outputNodeId();

    {
        const QSignalBlocker blockGraph(graph_);
        graph_->clear();
        for (const auto& n : nodes) {
            auto* item = new QListWidgetItem(QString("%1  %2").arg(n.isShader ? "◆" : "●", n.name), graph_);
            item->setData(kNodeIdRole, n.id);
            if (n.id == outputId) { QFont f = item->font(); f.setBold(true); item->setFont(f); }
        }
    }
    {
        const QSignalBlocker blockPick(stagePick_);
        const QString picked = stagePick_->currentData().toString();
        stagePick_->clear();
        stagePick_->addItem(kAutoStage);
        for (const auto& n : nodes) stagePick_->addItem(n.name, n.id);
        const int idx = picked.isEmpty() ? 0 : stagePick_->findData(picked);
        stagePick_->setCurrentIndex(idx < 0 ? 0 : idx);
    }
    const QString pinned = stagePick_->currentData().toString();
    stage_->setSourceNode(pinned.isEmpty() ? outputId : pinned);

    if (!nodes.isEmpty()) welcome_->hide();

    bool selectedExists = false;
    for (const auto& n : nodes) selectedExists |= (n.id == selectedId_);
    selectNode(selectedExists ? selectedId_ : outputId);
}

void MainWindow::selectNode(const QString& id) {
    selectedId_ = id;
    QString name;
    for (int i = 0; i < graph_->count(); ++i) {
        QListWidgetItem* item = graph_->item(i);
        if (item->data(kNodeIdRole).toString() != id) continue;
        const QSignalBlocker block(graph_);
        graph_->setCurrentItem(item);
        name = item->text().mid(3);
    }
    if (id != inspector_->node()) inspector_->setNode(id, name);
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
    const QString line = QString("engine %1 fps · stage %2 frames/s")
        .arg(EngineBridge::instance()->fps(), 0, 'f', 1).arg(perSec);
    statsLabel_->setText(line);
    if (logFile_) {
        logFile_->write((QDateTime::currentDateTime().toString("HH:mm:ss.zzz ") + "[stats] " + line + '\n').toUtf8());
        logFile_->flush();
    }
}
