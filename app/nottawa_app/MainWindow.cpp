#include "MainWindow.h"

#include "EngineBridge.h"
#include "Icons.h"
#include "ShaderCodeEditor.h"
#include "ShareStrandDialog.h"
#include "StageWidget.h"
#include "Theme.h"
#include "nodes/NodeEditorView.h"
#include "panels/AudioPanel.h"
#include "panels/BrowserPanel.h"
#include "panels/CommunityPage.h"
#include "panels/InspectorPanel.h"
#include "panels/LibraryPage.h"
#include "panels/LogPanel.h"
#include "panels/OscillatorsPage.h"
#include "panels/StrandsPage.h"
#include "panels/StageView.h"
#include "Screenshots.h"
#include "SettingsDialog.h"
#include "panels/WelcomeOverlay.h"
#include "ui/ActionBar.h"
#include "ui/Toast.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QResizeEvent>
#include <QSettings>
#include <QSplitter>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QtGui/qopenglcontext_platform.h>

namespace {
constexpr int kMaxRecent = 10;

QToolButton* toolbarButton(Icons::Kind icon, const QString& tip, QWidget* parent) {
    auto* b = new QToolButton(parent);
    b->setIcon(Icons::icon(icon, Theme::textSecondary, Theme::accent));
    b->setIconSize(QSize(17, 17));
    b->setFixedSize(30, 28);
    b->setToolTip(tip);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

QFrame* vline(QWidget* parent) {
    auto* line = new QFrame(parent);
    line->setFixedSize(1, 20);
    line->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    return line;
}
} // anon

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

    centre_ = new QStackedWidget;
    editor_ = new NodeEditorView(centre_);
    stage_  = new StageView(editor_, centre_);
    connect(stage_, &StageView::exitRequested, this, [this] { stageButton_->setChecked(false); });
    centre_->addWidget(editor_);
    centre_->addWidget(stage_);

    welcome_ = new WelcomeOverlay(centre_);
    welcome_->setGeometry(centre_->rect());
    welcome_->hide();
    connect(welcome_, &WelcomeOverlay::newRequested, this, &MainWindow::onNew);
    connect(welcome_, &WelcomeOverlay::openRequested, this, &MainWindow::onOpen);
    connect(welcome_, &WelcomeOverlay::strandPicked, this, [this](const QString& id) { editor_->loadStrand(id); });

    actionBar_ = new ActionBar(centre_);
    connect(actionBar_, &ActionBar::undo, this, [] { EngineBridge::instance()->undo(); });
    connect(actionBar_, &ActionBar::redo, this, [] { EngineBridge::instance()->redo(); });
    connect(actionBar_, &ActionBar::screenshot, this, &MainWindow::captureScreenshot);
    connect(actionBar_, &ActionBar::clearAll, this, &MainWindow::onNew);
    connect(actionBar_, &ActionBar::zoomToFit, editor_, &NodeEditorView::zoomToFit);
    toast_ = new Toast(centre_);
    fpsOverlay_ = new QLabel(centre_);
    fpsOverlay_->setStyleSheet(QString("QLabel { background: %1; color: %2; border-radius: 6px; padding: 4px 8px;"
                                       " font-family: Consolas; font-size: 8pt; }")
                                   .arg(QColor(30, 36, 51, 220).name(QColor::HexArgb), Theme::textPrimary.name()));
    fpsOverlay_->move(8, 8);
    fpsOverlay_->hide();
    qApp->installEventFilter(this);  // Shift held shows the FPS overlay

    buildPanels();
    buildMenus();

    auto* root = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(buildToolbar());
    auto* line = new QFrame(root);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    rootLayout->addWidget(line);
    rootLayout->addWidget(rows_, 1);
    setCentralWidget(root);

    connect(EngineBridge::instance(), &EngineBridge::graphChanged, this, &MainWindow::refreshGraph);
    connect(editor_, &NodeEditorView::nodeSelected, this, [this](const QString& id) {
        selectedId_ = id;
        EngineBridge::instance()->selectNode(id);
        bool isSource = false;
        for (const auto& n : EngineBridge::instance()->nodes())
            if (n.id == id) isSource = !n.isShader;
        inspector_->setSelection(editor_->selectedNodeIds());
        browser_->setSelectedNode(id, isSource);
        updateChrome();
    });

    // After the event loop starts, so the window is up while services load.
    QTimer::singleShot(0, this, &MainWindow::startEngine);

    statsTimer_ = new QTimer(this);
    connect(statsTimer_, &QTimer::timeout, this, &MainWindow::sampleStats);
    statsTimer_->start(1000);
    // Undo/redo availability has no notification; poll it.
    chromeTimer_ = new QTimer(this);
    connect(chromeTimer_, &QTimer::timeout, this, &MainWindow::updateChrome);
    chromeTimer_->start(250);
}

MainWindow::~MainWindow() {
    qApp->removeEventFilter(this);
    // The views release their shared textures (and the browser its
    // previews) when destroyed; do that while the engine is still running.
    browser_->shutdown();
    editor_->disconnect(this);
    delete rows_;
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

bool MainWindow::eventFilter(QObject* watched, QEvent* e) {
    if ((e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease)
        && static_cast<QKeyEvent*>(e)->key() == Qt::Key_Shift && !static_cast<QKeyEvent*>(e)->isAutoRepeat()) {
        const bool down = e->type() == QEvent::KeyPress;
        if (down) {
            fpsOverlay_->setText(QString("Engine %1 FPS\nUI %2 FPS").arg(int(EngineBridge::instance()->fps())).arg(uiFps_));
            fpsOverlay_->adjustSize();
            fpsOverlay_->raise();
        }
        fpsOverlay_->setVisible(down);
    }
    return QMainWindow::eventFilter(watched, e);
}

// ------------------------------------------------------------------ menus

void MainWindow::buildMenus() {
    auto* engine = EngineBridge::instance();
    auto* file = menuBar()->addMenu("&File");
    file->addAction("&New", QKeySequence::New, this, &MainWindow::onNew);
    file->addAction("&Open…", QKeySequence::Open, this, &MainWindow::onOpen);
    recentMenu_ = file->addMenu("Open &Recent");
    connect(recentMenu_, &QMenu::aboutToShow, this, &MainWindow::rebuildRecentMenu);
    file->addSeparator();
    file->addAction("&Save", QKeySequence::Save, this, &MainWindow::onSave);
    file->addAction("Save &As…", QKeySequence("Ctrl+Shift+S"), this, &MainWindow::onSaveAs);
    file->addSeparator();
    file->addAction("E&xit", QKeySequence::Quit, this, &QWidget::close);

    auto* edit = menuBar()->addMenu("&Edit");
    undoAction_ = edit->addAction("&Undo", QKeySequence::Undo, this, [engine] { engine->undo(); });
    redoAction_ = edit->addAction("&Redo", QKeySequence("Ctrl+Shift+Z"), this, [engine] { engine->redo(); });
    // The canvas also handles these keys itself; menu shortcuts only fire
    // when it doesn't have focus.
    undoAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    redoAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    edit->addSeparator();
    edit->addAction("&Settings…", QKeySequence("Ctrl+,"), this, [this] {
        if (!settings_) {
            settings_ = new SettingsDialog(this);
            connect(settings_, &SettingsDialog::showHintsChanged, editor_, &NodeEditorView::setShowHints);
        }
        settings_->show();
        settings_->raise();
        settings_->activateWindow();
    });

    auto* view = menuBar()->addMenu("&View");
    view->addAction("Toggle &Full Screen", QKeySequence(Qt::Key_F11), this, [this] {
        isFullScreen() ? showNormal() : showFullScreen();
    });
    view->addSeparator();
    view->addAction("&Browser", QKeySequence("Ctrl+B"), this, [this] { sidebarButton_->click(); });
    view->addAction("&Audio Panel", QKeySequence("Ctrl+Shift+A"), this, [this] { audioButton_->click(); });
    stageAction_ = view->addAction("&Stage Mode", QKeySequence("Ctrl+Return"), this, [this] { stageButton_->click(); });
    view->addAction("Zoom to &Fit", QKeySequence("Ctrl+0"), editor_, &NodeEditorView::zoomToFit);
    auto* showLog = view->addAction("Show &Log", this, [this](bool on) { logPanel_->setVisible(on); });
    showLog->setCheckable(true);
    view->addSeparator();
    QSettings settings;
    auto* clearOnDisconnect = view->addAction("Clear on Disconnect", this, [engine](bool on) {
        QSettings().setValue("ui/clearOnDisconnect", on);
        engine->setClearOnDisconnect(on);
    });
    clearOnDisconnect->setCheckable(true);
    clearOnDisconnect->setChecked(settings.value("ui/clearOnDisconnect", false).toBool());

    // Rebuilt each time it opens: sources can appear (devices plugged in)
    // and capture state changes from elsewhere.
    auto* audio = menuBar()->addMenu("&Audio");
    connect(audio, &QMenu::aboutToShow, this, [audio, engine] {
        audio->clear();
        auto* enabled = audio->addAction("Audio &Enabled", engine, &EngineBridge::toggleAudio);
        enabled->setCheckable(true);
        enabled->setChecked(engine->isAudioActive());
        audio->addSection("Source");
        auto* group = new QActionGroup(audio);
        const QString selected = engine->selectedAudioSource();
        for (const AudioSourceItem& s : engine->audioSources()) {
            auto* a = audio->addAction(s.name, engine, [engine, id = s.id] { engine->selectAudioSource(id); });
            a->setCheckable(true);
            a->setChecked(s.id == selected);
            group->addAction(a);
        }
    });

    auto* help = menuBar()->addMenu("&Help");
    help->addAction("Show &Welcome Screen", this, [this] {
        welcome_->setGeometry(centre_->rect());
        welcome_->present();
    });
    help->addSeparator();
    help->addAction("Visit nottawa.app", this, [] { QDesktopServices::openUrl(QUrl("https://nottawa.app")); });
    help->addAction("Send Feedback…", this, [] {
        QDesktopServices::openUrl(QUrl("mailto:joe@nottawa.app?subject=Nottawa%20Feedback"));
    });
    help->addSeparator();
    help->addAction("&About Nottawa", this, [this] {
        QMessageBox::about(this, "About Nottawa", "<b>Nottawa</b><br>Audio-reactive visuals for live performance.");
    });
}

void MainWindow::rebuildRecentMenu() {
    recentMenu_->clear();
    const QStringList recent = QSettings().value("workspaces/recent").toStringList();
    if (recent.isEmpty()) {
        recentMenu_->addAction("No Recent Workspaces")->setEnabled(false);
        return;
    }
    for (const QString& path : recent) {
        QAction* a = recentMenu_->addAction(QFileInfo(path).completeBaseName(), this, [this, path] { openWorkspace(path); });
        a->setToolTip(path);
        a->setEnabled(QFileInfo::exists(path));
    }
    recentMenu_->addSeparator();
    recentMenu_->addAction("Clear Menu", this, [] { QSettings().remove("workspaces/recent"); });
}

// ---------------------------------------------------------------- toolbar

QWidget* MainWindow::buildToolbar() {
    auto* engine = EngineBridge::instance();
    auto* bar = new QWidget(this);
    bar->setObjectName("Toolbar");
    bar->setStyleSheet(QString("#Toolbar { background: %1; }").arg(Theme::backgroundSecondary.name()));
    auto* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(16, 6, 16, 6);
    layout->setSpacing(4);

    undoButton_ = toolbarButton(Icons::Undo, "Undo (Ctrl+Z)", bar);
    redoButton_ = toolbarButton(Icons::Redo, "Redo (Ctrl+Shift+Z)", bar);
    connect(undoButton_, &QToolButton::clicked, this, [engine] { engine->undo(); });
    connect(redoButton_, &QToolButton::clicked, this, [engine] { engine->redo(); });
    layout->addWidget(undoButton_);
    layout->addWidget(redoButton_);
    layout->addSpacing(8);
    layout->addWidget(vline(bar));
    layout->addSpacing(8);

    sidebarButton_ = toolbarButton(Icons::Sidebar, "Toggle Browser Sidebar (Ctrl+B)", bar);
    sidebarButton_->setCheckable(true);
    sidebarButton_->setChecked(QSettings().value("ui/showSidebar", true).toBool());
    browser_->setVisible(sidebarButton_->isChecked());
    connect(sidebarButton_, &QToolButton::toggled, this, [this](bool on) {
        browser_->setVisible(on);
        QSettings().setValue("ui/showSidebar", on);
    });
    audioButton_ = toolbarButton(Icons::Waveform, "Toggle Audio Panel (Ctrl+Shift+A)", bar);
    audioButton_->setCheckable(true);
    connect(audioButton_, &QToolButton::toggled, this, [this](bool on) { audioPanel_->setVisible(on); });
    stageButton_ = toolbarButton(Icons::Sliders, "Stage Mode (Ctrl+Enter)", bar);
    stageButton_->setCheckable(true);
    connect(stageButton_, &QToolButton::toggled, this, &MainWindow::setStageMode);
    layout->addWidget(sidebarButton_);
    layout->addWidget(audioButton_);
    layout->addWidget(stageButton_);

    deleteSeparator_ = new QWidget(bar);
    auto* sepLayout = new QHBoxLayout(deleteSeparator_);
    sepLayout->setContentsMargins(8, 0, 8, 0);
    sepLayout->addWidget(vline(deleteSeparator_));
    deleteButton_ = toolbarButton(Icons::Trash, "Delete selected (Delete)", bar);
    deleteButton_->setIcon(Icons::icon(Icons::Trash, Theme::destructive));
    connect(deleteButton_, &QToolButton::clicked, editor_, &NodeEditorView::deleteSelection);
    layout->addWidget(deleteSeparator_);
    layout->addWidget(deleteButton_);
    layout->addStretch(1);

    workspaceLabel_ = new QLabel(bar);
    workspaceLabel_->setStyleSheet(QString("color: %1; font-weight: 500;").arg(Theme::textSecondary.name()));
    layout->addWidget(workspaceLabel_);
    updateChrome();
    return bar;
}

// Toolbar and action bar state that has no change notification.
void MainWindow::updateChrome() {
    if (!undoButton_) return;
    auto* engine = EngineBridge::instance();
    const bool canUndo = engine->isRunning() && engine->canUndo();
    const bool canRedo = engine->isRunning() && engine->canRedo();
    undoButton_->setEnabled(canUndo);
    redoButton_->setEnabled(canRedo);
    undoButton_->setIcon(Icons::icon(Icons::Undo, canUndo ? Theme::textSecondary : Theme::textTertiary.darker(130)));
    redoButton_->setIcon(Icons::icon(Icons::Redo, canRedo ? Theme::textSecondary : Theme::textTertiary.darker(130)));
    if (undoAction_) { undoAction_->setEnabled(canUndo); redoAction_->setEnabled(canRedo); }
    actionBar_->setUndoRedo(canUndo, canRedo);
    const bool selection = !editor_->selectedNodeIds().isEmpty();
    deleteButton_->setVisible(selection);
    deleteSeparator_->setVisible(selection);
    sidebarButton_->setEnabled(!stageButton_->isChecked());
    inspector_->setVisible(selection && !stageButton_->isChecked());
}

// ----------------------------------------------------------------- panels

void MainWindow::buildPanels() {
    browser_   = new BrowserPanel(this);
    inspector_ = new InspectorPanel(this);
    connect(inspector_, &InspectorPanel::deselectRequested, this, [this] { editor_->scene()->clearSelection(); });
    connect(inspector_, &InspectorPanel::nodeRequested, this, [this](const QString& id) {
        editor_->selectNode(id);
        emit editor_->nodeSelected(id);
    });
    connect(inspector_, &InspectorPanel::graphMayChange, this, &MainWindow::refreshGraph);
    connect(inspector_, &InspectorPanel::editShaderRequested, this, &MainWindow::openShaderEditor);
    connect(browser_, &BrowserPanel::itemPicked, this, &MainWindow::addFromLibrary);

    auto* oscillators = new OscillatorsPage(browser_);
    browser_->setPage(BrowserPanel::Oscillators, oscillators);
    connect(oscillators, &OscillatorsPage::nodeRequested, this, [this](const QString& id) {
        editor_->selectNode(id);
        emit editor_->nodeSelected(id);
    });
    auto* library = new LibraryPage(browser_);
    browser_->setPage(BrowserPanel::Library, library);
    connect(library, &LibraryPage::addRequested, this, [this](const QString& id) {
        if (editor_->addLibrarySource(id).isEmpty()) log("[library] ERROR could not add " + id);
        else welcome_->hide();
    });
    auto* strands = new StrandsPage(browser_);
    browser_->setPage(BrowserPanel::Strands, strands);
    connect(strands, &StrandsPage::loadRequested, this, [this](const QString& id) {
        editor_->loadStrand(id);
        welcome_->hide();
    });
    auto* community = new CommunityPage(strands);
    strands->setCommunityPage(community);
    connect(community, &CommunityPage::loadRequested, this, [this](const QString& slug) {
        editor_->loadSharedStrand(slug);
        welcome_->hide();
    });
    connect(editor_, &NodeEditorView::editShaderRequested, this, &MainWindow::openShaderEditor);
    connect(editor_, &NodeEditorView::shareRequested, this, [this, strands, community](const QString& id, const QString& name) {
        auto* dialog = new ShareStrandDialog(id, name, editor_->captureNode(id), this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &ShareStrandDialog::shared, this, [this, strands, community](const QString& slug) {
            log("[strand] shared https://nottawa.app/strands/" + slug);
            browser_->showTab(BrowserPanel::Strands);
            strands->showCommunity();
            community->refresh();
        });
        dialog->open();
    });
    connect(editor_, &NodeEditorView::strandLoadFailed, this, [this](const QString& error) {
        log("[strand] ERROR " + error);
        toast_->show("Couldn't load strand: " + error, 5000);
    });
    connect(editor_, &NodeEditorView::strandSaved, this, [this](const QString& name) {
        log("[strand] saved " + name);
        toast_->show(QString("Saved strand \"%1\"").arg(name));
    });

    columns_ = new QSplitter(Qt::Horizontal);
    columns_->setChildrenCollapsible(false);
    columns_->setHandleWidth(1);
    columns_->addWidget(browser_);
    columns_->addWidget(centre_);
    columns_->addWidget(inspector_);
    columns_->setStretchFactor(1, 1);
    browser_->setMinimumWidth(180);
    browser_->setMaximumWidth(500);
    inspector_->setMinimumWidth(250);
    inspector_->setMaximumWidth(500);
    columns_->setSizes({260, 1000, 320});

    audioPanel_ = new AudioPanel(this);
    audioPanel_->hide();
    logPanel_ = new LogPanel(this);
    logPanel_->setMinimumHeight(80);
    logPanel_->hide();

    rows_ = new QSplitter(Qt::Vertical);
    rows_->setChildrenCollapsible(false);
    rows_->setHandleWidth(1);
    rows_->addWidget(columns_);
    rows_->addWidget(audioPanel_);
    rows_->addWidget(logPanel_);
    rows_->setStretchFactor(0, 1);
    rows_->setSizes({800, 220, 120});
}

// ----------------------------------------------------------------- engine

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
    engine->setClearOnDisconnect(QSettings().value("ui/clearOnDisconnect", false).toBool());
    const auto sources = engine->sources();
    const auto effects = engine->effects();
    browser_->reload();
    const QSize res = engine->resolution();
    log(QString("[engine] running at %1x%2 — %3 sources, %4 effects available")
        .arg(res.width()).arg(res.height()).arg(sources.size()).arg(effects.size()));
    refreshGraph();
    editor_->setFocus();
    // Scripted runs skip it so their clicks land on the canvas.
    if (WelcomeOverlay::enabledAtLaunch() && qEnvironmentVariableIsEmpty("NOTTAWA_ADD")) {
        welcome_->setGeometry(centre_->rect());
        welcome_->present();
    }
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
            for (const auto& list : {engine->sources(), engine->effects(), engine->inputs()}) {
                for (const auto& item : list) {
                    if (item.name.compare(name, Qt::CaseInsensitive) != 0) continue;
                    addFromLibrary(item);
                    log("[script] added " + item.name);
                    return;
                }
            }
            log("[script] ERROR no catalog entry named " + name);
        });
    }
}

void MainWindow::addFromLibrary(const CatalogItem& item) {
    if (!EngineBridge::instance()->isRunning()) return;
    // Effects chain after the selected node, else after the output.
    const QString after = selectedId_.isEmpty() ? EngineBridge::instance()->outputNodeId() : selectedId_;
    const QString id = item.isInput ? editor_->addInput(item.typeRaw)
                                    : editor_->addFromCatalog(item.typeRaw, item.isSource, after);
    // An image input that was cancelled in its file dialog isn't an error.
    if (!id.isEmpty()) welcome_->hide();
    else if (!(item.isInput && item.typeRaw == VideoSourceKind::Image)) log("[graph] ERROR could not add node");
}

// One editor window per shader; asking again raises it.
void MainWindow::openShaderEditor(const QString& shaderId, const QString& name) {
    QPointer<ShaderCodeEditor>& editor = shaderEditors_[shaderId];
    if (!editor) editor = new ShaderCodeEditor(shaderId, name, this);
    editor->show();
    editor->raise();
    editor->activateWindow();
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
    updateStageSource();
    inspector_->setSelection(editor_->selectedNodeIds());

    bool selectedExists = false;
    for (const auto& n : nodes) selectedExists |= (n.id == selectedId_);
    if (!selectedExists && !selectedId_.isEmpty()) selectedId_.clear();
    updateChrome();
}

void MainWindow::setStageMode(bool on) {
    if (on) editor_->scene()->clearSelection();  // as macOS does on entering
    centre_->setCurrentWidget(on ? static_cast<QWidget*>(stage_) : editor_);
    actionBar_->setVisible(!on);
    stageAction_->setChecked(on);
    browser_->setVisible(!on && sidebarButton_->isChecked());
    if (!on) editor_->setFocus();
    updateStageSource();
    updateChrome();
}

void MainWindow::updateStageSource() {
    if (centre_->currentWidget() == stage_) stage_->rebuild();
}

// -------------------------------------------------------------- workspaces

void MainWindow::setWorkspace(const QString& path) {
    workspacePath_ = path;
    workspaceLabel_->setText(path.isEmpty() ? QString() : QFileInfo(path).completeBaseName());
    setWindowTitle(path.isEmpty() ? "Nottawa" : QFileInfo(path).completeBaseName() + " — Nottawa");
    if (path.isEmpty()) return;
    QSettings settings;
    QStringList recent = settings.value("workspaces/recent").toStringList();
    recent.removeAll(path);
    recent.prepend(path);
    while (recent.size() > kMaxRecent) recent.removeLast();
    settings.setValue("workspaces/recent", recent);
}

void MainWindow::onNew() {
    if (!EngineBridge::instance()->isRunning()) return;
    EngineBridge::instance()->clearGraph();
    setWorkspace({});
    log("[file] new graph");
}

void MainWindow::onOpen() {
    const QString path = QFileDialog::getOpenFileName(this, "Open workspace", {}, "Nottawa workspace (*.json);;All files (*)");
    if (!path.isEmpty()) openWorkspace(path);
}

void MainWindow::openWorkspace(const QString& path) {
    if (!EngineBridge::instance()->isRunning()) return;
    welcome_->hide();
    if (EngineBridge::instance()->loadWorkspace(path)) {
        setWorkspace(path);
        log("[file] opened " + path);
        editor_->zoomToFit();
    } else {
        log("[file] ERROR could not open " + path);
        toast_->show("Couldn't open " + QFileInfo(path).fileName(), 5000);
    }
}

// Saves over the open workspace; a new one asks where.
void MainWindow::onSave() {
    if (workspacePath_.isEmpty()) { onSaveAs(); return; }
    EngineBridge::instance()->saveWorkspace(workspacePath_, QFileInfo(workspacePath_).completeBaseName());
    log("[file] saved " + workspacePath_);
    toast_->show("Saved " + QFileInfo(workspacePath_).completeBaseName());
}

void MainWindow::onSaveAs() {
    const QString path = QFileDialog::getSaveFileName(this, "Save workspace", workspacePath_, "Nottawa workspace (*.json)");
    if (path.isEmpty() || !EngineBridge::instance()->isRunning()) return;
    EngineBridge::instance()->saveWorkspace(path, QFileInfo(path).completeBaseName());
    setWorkspace(path);
    log("[file] saved " + path);
    toast_->show("Saved " + QFileInfo(path).completeBaseName());
}

// ------------------------------------------------------------- screenshot

// Saves the selected node's (else the output's) current frame at the
// engine's resolution, to a folder chosen once and remembered.
void MainWindow::captureScreenshot() {
    auto* engine = EngineBridge::instance();
    const QString id = selectedId_.isEmpty() ? engine->outputNodeId() : selectedId_;
    if (id.isEmpty()) return;
    const QImage image = editor_->captureNode(id, engine->resolution());
    if (image.isNull()) { toast_->show("Nothing to capture yet"); return; }
    const QString file = Screenshots::save(image, this);
    if (!file.isEmpty()) {
        toast_->show("Screenshot saved to " + QDir::toNativeSeparators(QFileInfo(file).absolutePath()));
        log("[screenshot] " + file);
    } else if (!Screenshots::folder().isEmpty()) {
        toast_->show("Couldn't save the screenshot", 5000);
    }
}

void MainWindow::sampleStats() {
    const quint64 painted = editor_->framesPainted();
    uiFps_ = int(painted - lastPainted_);
    lastPainted_ = painted;
    if (fpsOverlay_->isVisible()) {
        fpsOverlay_->setText(QString("Engine %1 FPS\nUI %2 FPS").arg(int(EngineBridge::instance()->fps())).arg(uiFps_));
        fpsOverlay_->adjustSize();
    }
    if (logFile_) {
        const QString line = QString("engine %1 fps").arg(EngineBridge::instance()->fps(), 0, 'f', 1);
        logFile_->write((QDateTime::currentDateTime().toString("HH:mm:ss.zzz ") + "[stats] " + line + '\n').toUtf8());
        logFile_->flush();
    }
}
