#include "MainWindow.h"

#include "StageEngine.h"
#include "StageWidget.h"
#include "panels/BrowserPanel.h"
#include "panels/InspectorPanel.h"
#include "panels/LogPanel.h"
#include "panels/WelcomeOverlay.h"

#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QOffscreenSurface>
#include <QResizeEvent>
#include <QStatusBar>
#include <QSurfaceFormat>
#include <QTimer>
#include <QToolBar>

MainWindow::MainWindow() {
    setWindowTitle("Nottawa");
    resize(1600, 1000);

    stage_ = new StageWidget(this);
    setCentralWidget(stage_);

    // Overlay lives inside the central widget, resized to track it in
    // resizeEvent(). It's a child of stage_ so paintEvent order works.
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
    if (engine_) { engine_->requestStop(); engine_->wait(2000); }
}

void MainWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
    if (welcome_ && stage_) welcome_->setGeometry(stage_->rect());
}

void MainWindow::buildMenus() {
    auto* mb = menuBar();

    auto* file = mb->addMenu("&File");
    file->addAction("&New Strand",  this, &MainWindow::onNewStrand,  QKeySequence::New);
    file->addAction("&Open…",       this, &MainWindow::onOpenStrand, QKeySequence::Open);
    file->addAction("&Save",        this, &MainWindow::onSaveStrand, QKeySequence::Save);
    file->addSeparator();
    file->addAction("E&xit", this, &QWidget::close, QKeySequence::Quit);

    auto* view = mb->addMenu("&View");
    view->addAction("Stage &Fullscreen", this, &MainWindow::onToggleStageFullscreen,
                    QKeySequence("F11"));

    auto* help = mb->addMenu("&Help");
    help->addAction("&About Nottawa", [this]{
        QMessageBox::about(this, "About Nottawa",
            "<b>Nottawa</b><br>Audio-reactive visuals for live performance.<br><br>"
            "Windows UI — Phase A.1 shell on Qt 6.");
    });
}

void MainWindow::buildToolbar() {
    auto* tb = addToolBar("Main");
    tb->setMovable(false);
    tb->setIconSize(QSize(18, 18));

    tb->addAction("New",      this, &MainWindow::onNewStrand);
    tb->addAction("Open",     this, &MainWindow::onOpenStrand);
    tb->addAction("Save",     this, &MainWindow::onSaveStrand);
    tb->addSeparator();
    tb->addAction("Add Source");
    tb->addAction("Add Effect");
    tb->addSeparator();
    tb->addAction("Stage FS", this, &MainWindow::onToggleStageFullscreen);
}

void MainWindow::buildDocks() {
    browser_      = new BrowserPanel(this);
    inspector_    = new InspectorPanel(this);
    logPanel_     = new LogPanel(this);

    browserDock_ = new QDockWidget("LIBRARY", this);
    browserDock_->setWidget(browser_);
    browserDock_->setFeatures(QDockWidget::DockWidgetMovable |
                              QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, browserDock_);
    browserDock_->setMinimumWidth(240);

    inspectorDock_ = new QDockWidget("INSPECTOR", this);
    inspectorDock_->setWidget(inspector_);
    inspectorDock_->setFeatures(QDockWidget::DockWidgetMovable |
                                QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::RightDockWidgetArea, inspectorDock_);
    inspectorDock_->setMinimumWidth(300);

    logDock_ = new QDockWidget("LOG", this);
    logDock_->setWidget(logPanel_);
    logDock_->setFeatures(QDockWidget::DockWidgetMovable |
                          QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::BottomDockWidgetArea, logDock_);
    logDock_->setMinimumHeight(120);
}

void MainWindow::buildStatusBar() {
    auto* sb = statusBar();
    strandLabel_ = new QLabel("No strand");
    fpsLabel_    = new QLabel("engine: starting…");
    sb->addWidget(strandLabel_, /*stretch=*/1);
    sb->addPermanentWidget(fpsLabel_);
}

void MainWindow::startEngineOnGlReady() {
    // Launch the engine thread as soon as the stage widget's GL context
    // exists. The spike proved: install the connect BEFORE show() or
    // the signal is missed.
    connect(stage_, &StageWidget::glReady, this, [this]() {
        auto* surface = new QOffscreenSurface;
        surface->setFormat(QSurfaceFormat::defaultFormat());
        surface->create();

        engine_ = new StageEngine(stage_, surface);
        engine_->start();
        logPanel_->append("[shell] stage engine started");
    }, Qt::QueuedConnection);

    statsTimer_ = new QTimer(this);
    connect(statsTimer_, &QTimer::timeout, this, &MainWindow::sampleEngineStats);
    statsTimer_->start(500);
}

void MainWindow::sampleEngineStats() {
    if (!engine_) return;
    const double fps = engine_->currentFps();
    const double p99 = engine_->currentP99Ms();
    fpsLabel_->setText(QString("engine: %1 fps · p99 %2 ms")
                       .arg(fps, 0, 'f', 1)
                       .arg(p99, 0, 'f', 2));
}

void MainWindow::onNewStrand() {
    welcomeDismissed_ = true;
    welcome_->hide();
    strandLabel_->setText("Untitled strand");
    logPanel_->append("[shell] new strand (Phase A.3 wires engine)");
}

void MainWindow::onOpenStrand() {
    const QString path = QFileDialog::getOpenFileName(
        this, "Open strand", {},
        "Nottawa strand (*.json);;All files (*)");
    if (path.isEmpty()) return;
    welcomeDismissed_ = true;
    welcome_->hide();
    strandLabel_->setText(path);
    logPanel_->append(QString("[shell] opened %1 (Phase A.3 wires engine)").arg(path));
}

void MainWindow::onSaveStrand() {
    logPanel_->append("[shell] save (Phase A.3)");
}

void MainWindow::onToggleStageFullscreen() {
    // Phase A.1 placeholder — in A.4 we lift the central stage to its
    // own borderless window on the configured output monitor.
    logPanel_->append("[shell] stage fullscreen (Phase A.4)");
}
