#include "MainWindow.h"

#include "OFStageEngine.h"
#include "StageWidget.h"
#include "panels/BrowserPanel.h"
#include "panels/InspectorPanel.h"
#include "panels/LogPanel.h"
#include "panels/WelcomeOverlay.h"

#include <windows.h>

#include <QComboBox>
#include <QDateTime>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>

MainWindow::MainWindow() {
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
    connect(welcome_, &WelcomeOverlay::newStrandRequested,  this, &MainWindow::onNewStrand);
    connect(welcome_, &WelcomeOverlay::openStrandRequested, this, &MainWindow::onOpenStrand);

    buildMenus();
    buildToolbar();
    buildDocks();
    buildStatusBar();
    startEngineOnGlReady();
}

MainWindow::~MainWindow() {
    delete engine_;  // stops and joins the engine thread before the stage widget goes away
}

void MainWindow::log(const QString& line) {
    if (logPanel_) logPanel_->append(line);
    if (logFile_) {
        logFile_->write((QDateTime::currentDateTime().toString("HH:mm:ss.zzz ") + line + '\n').toUtf8());
        logFile_->flush();
    }
}

void MainWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
    if (welcome_ && stage_) welcome_->setGeometry(stage_->rect());
}

void MainWindow::buildMenus() {
    auto* file = menuBar()->addMenu("&File");
    file->addAction("&New Strand", this, &MainWindow::onNewStrand,  QKeySequence::New);
    file->addAction("&Open…",      this, &MainWindow::onOpenStrand, QKeySequence::Open);
    file->addAction("&Save",       this, &MainWindow::onSaveStrand, QKeySequence::Save);
    file->addSeparator();
    file->addAction("E&xit", this, &QWidget::close, QKeySequence::Quit);

    auto* view = menuBar()->addMenu("&View");
    view->addAction("Stage &Fullscreen", this, &MainWindow::onToggleStageFullscreen, QKeySequence("F11"));

    auto* help = menuBar()->addMenu("&Help");
    help->addAction("&About Nottawa", this, [this] {
        QMessageBox::about(this, "About Nottawa",
            "<b>Nottawa</b><br>Audio-reactive visuals for live performance.");
    });
}

void MainWindow::buildToolbar() {
    auto* tb = addToolBar("Main");
    tb->setMovable(false);
    tb->addAction("New",  this, &MainWindow::onNewStrand);
    tb->addAction("Open", this, &MainWindow::onOpenStrand);
    tb->addAction("Save", this, &MainWindow::onSaveStrand);
    tb->addSeparator();
    tb->addAction("Add Source");
    tb->addAction("Add Effect");
    tb->addSeparator();

    shaderPicker_ = new QComboBox;
    shaderPicker_->setMinimumWidth(220);
    shaderPicker_->addItems(nottawaShaderNames());
    shaderPicker_->setCurrentText(nottawaDefaultShader());
    connect(shaderPicker_, &QComboBox::currentTextChanged, this, &MainWindow::selectShader);
    tb->addWidget(new QLabel("  Shader: "));
    tb->addWidget(shaderPicker_);

    tb->addSeparator();
    tb->addAction("Stage FS", this, &MainWindow::onToggleStageFullscreen);
}

void MainWindow::buildDocks() {
    browser_   = new BrowserPanel(this);
    inspector_ = new InspectorPanel(this);
    logPanel_  = new LogPanel(this);

    browser_->populate(nottawaShaderNames());
    browser_->setCurrentShader(nottawaDefaultShader());
    inspector_->setCurrentShader(nottawaDefaultShader());
    connect(browser_, &BrowserPanel::shaderPicked, this, &MainWindow::selectShader);

    auto addDock = [this](const QString& title, QWidget* w, Qt::DockWidgetArea area) {
        auto* dock = new QDockWidget(title, this);
        dock->setWidget(w);
        dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        addDockWidget(area, dock);
        return dock;
    };
    addDock("LIBRARY",   browser_,   Qt::LeftDockWidgetArea)->setMinimumWidth(240);
    addDock("INSPECTOR", inspector_, Qt::RightDockWidgetArea)->setMinimumWidth(300);
    addDock("LOG",       logPanel_,  Qt::BottomDockWidgetArea)->setMinimumHeight(120);
}

void MainWindow::buildStatusBar() {
    strandLabel_ = new QLabel("No strand");
    fpsLabel_    = new QLabel("engine: starting…");
    statusBar()->addWidget(strandLabel_, 1);
    statusBar()->addPermanentWidget(fpsLabel_);
}

void MainWindow::selectShader(const QString& name) {
    if (name.isEmpty()) return;
    {
        const QSignalBlocker block(shaderPicker_);
        shaderPicker_->setCurrentText(name);
    }
    browser_->setCurrentShader(name);
    inspector_->setCurrentShader(name);
    if (engine_) engine_->requestShader(name);
    dismissWelcome();
}

void MainWindow::dismissWelcome() {
    if (welcome_) welcome_->hide();
}

void MainWindow::startEngineOnGlReady() {
    // glReady fires on the GUI thread with the stage's context current —
    // the moment to capture its HGLRC for wglShareLists.
    connect(stage_, &StageWidget::glReady, this, [this] {
        HGLRC qtHglrc = wglGetCurrentContext();
        if (!qtHglrc) { log("[engine] ERROR stage context missing at glReady"); return; }

        engine_ = new OFStageEngine(stage_, qtHglrc);
        connect(engine_, &OFStageEngine::engineInfo,  this, [this](const QString& s) { log("[engine] " + s); });
        connect(engine_, &OFStageEngine::engineError, this, [this](const QString& s) { log("[engine] ERROR " + s); });
        connect(engine_, &OFStageEngine::benchFinished, this, &QWidget::close);
        connect(inspector_, &InspectorPanel::brightnessChanged, engine_, &OFStageEngine::setBrightness);
        // Normal priority showed occasional 25-31 ms frame intervals from
        // scheduler preemption despite ~7 ms of GPU work.
        engine_->start(QThread::HighestPriority);
    }, Qt::QueuedConnection);

    statsTimer_ = new QTimer(this);
    connect(statsTimer_, &QTimer::timeout, this, &MainWindow::sampleEngineStats);
    statsTimer_->start(1000);
}

void MainWindow::sampleEngineStats() {
    if (!engine_) return;
    const EngineStats s = engine_->stats();
    const quint64 shown = stage_->framesShown();
    const quint64 shownPerSec = shown - lastShown_;
    lastShown_ = shown;

    const QString line = QString("engine %1 fps · interval p99 %2 ms max %3 ms · GPU %4 ms (p99 %5) · shown %6/s · hitches %7")
        .arg(s.fps, 0, 'f', 1).arg(s.intervalP99Ms, 0, 'f', 1).arg(s.intervalMaxMs, 0, 'f', 1)
        .arg(s.gpuAvgMs, 0, 'f', 2).arg(s.gpuP99Ms, 0, 'f', 2).arg(shownPerSec).arg(s.hitches);
    fpsLabel_->setText(line);
    if (logFile_) {
        logFile_->write((QDateTime::currentDateTime().toString("HH:mm:ss.zzz ") + "[stats] " + line + '\n').toUtf8());
        logFile_->flush();
    }
}

void MainWindow::onNewStrand() {
    dismissWelcome();
    strandLabel_->setText("Untitled strand");
}

void MainWindow::onOpenStrand() {
    const QString path = QFileDialog::getOpenFileName(this, "Open strand", {},
                                                      "Nottawa strand (*.json);;All files (*)");
    if (path.isEmpty()) return;
    dismissWelcome();
    strandLabel_->setText(path);
    log("[shell] strand loading not implemented yet: " + path);
}

void MainWindow::onSaveStrand() {
    log("[shell] strand saving not implemented yet");
}

void MainWindow::onToggleStageFullscreen() {
    log("[shell] stage fullscreen not implemented yet");
}
