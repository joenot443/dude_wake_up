#include "MainWindow.h"

#include "EngineBridge.h"
#include "Icons.h"
#include "UpdateCheck.h"
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
#include "panels/HelpGuide.h"
#include "panels/StageView.h"
#include "PreviewWindow.h"
#include "Screenshots.h"
#include "StressTest.h"
#include "ui/ParamSelection.h"
#include "ui/Flash.h"
#include "Prefs.h"
#include "SettingsDialog.h"
#include "panels/WelcomeOverlay.h"
#include "ui/ActionBar.h"
#include "ui/Toast.h"

#include <windows.h>
#include <QRegularExpression>
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
#include <QAbstractButton>
#include <QAbstractSpinBox>
#include <QClipboard>
#include <QComboBox>
#include <QElapsedTimer>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
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
    connect(actionBar_, &ActionBar::helpToggled, this, [this] { help_->setEnabled(!help_->enabled()); });
    toast_ = new Toast(centre_);
    connect(EngineBridge::instance(), &EngineBridge::tapped, this, [this](float bpm) {
        toast_->show(bpm > 0 ? QString("Tap tempo: %1 BPM").arg(bpm, 0, 'f', 1) : QString("Tap tempo: keep tapping"), 1500);
    });
    fpsOverlay_ = new QLabel(centre_);
    fpsOverlay_->setStyleSheet(QString("QLabel { background: %1; color: %2; border-radius: 6px; padding: 4px 8px;"
                                       " font-family: Consolas; font-size: 8pt; }")
                                   .arg(QColor(30, 36, 51, 220).name(QColor::HexArgb), Theme::textPrimary.name()));
    fpsOverlay_->move(8, 8);
    fpsOverlay_->hide();
    qApp->installEventFilter(this);  // Shift held shows the FPS overlay

    buildPanels();
    help_ = new HelpGuide(centre_, editor_, inspector_, browser_, this);
    help_->setBlocker(welcome_);
    connect(help_, &HelpGuide::enabledChanged, actionBar_, &ActionBar::setHelpActive);
    connect(help_, &HelpGuide::sidebarNeeded, this, [this] { if (!sidebarButton_->isChecked()) sidebarButton_->click(); });
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
    EngineBridge::destroyAllPreviews();
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

// The open output windows, for Reopen Output Windows at the next launch.
void MainWindow::closeEvent(QCloseEvent* e) {
    if (EngineBridge::instance()->isRunning()) QSettings().setValue("outputWindows/open", editor_->outputWindowState());
    QMainWindow::closeEvent(e);
}

void MainWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
    welcome_->setGeometry(centre_->rect());
}

// The macOS editor's parameter keys: Tab / Shift+Tab select the next or
// previous parameter (inspector, or the stage strips in stage mode); with one
// selected, Up/Down step it (Shift coarse, Alt fine), a digit, '-' or '.'
// starts typing its value, and Escape clears it. Escape otherwise deselects
// the nodes. Text fields keep their keys.
bool MainWindow::handleParameterKey(QObject* watched, QKeyEvent* e) {
    auto* target = qobject_cast<QWidget*>(watched);
    if (!target || target->window() != this || welcome_->isVisible()) return false;
    if (QApplication::activePopupWidget() || QApplication::activeModalWidget()) return false;
    QWidget* focus = QApplication::focusWidget();
    if (qobject_cast<QLineEdit*>(focus) || qobject_cast<QAbstractSpinBox*>(focus) ||
        qobject_cast<QPlainTextEdit*>(focus) || qobject_cast<QTextEdit*>(focus) || qobject_cast<QComboBox*>(focus))
        return false;
    if (target != (focus ? focus : this)) return false;  // once per key press, at its first receiver

    const bool stage = centre_->currentWidget() == stage_;
    const Qt::KeyboardModifiers mods = e->modifiers();
    auto* selection = ParamSelection::instance();
    const QVector<ParamInfo> params = stage ? stage_->navigableParameters() : inspector_->navigableParameters();
    auto reveal = [&](const QString& id) { stage ? stage_->revealParameter(id) : inspector_->revealParameter(id); };

    if ((e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) && !(mods & (Qt::ControlModifier | Qt::AltModifier))) {
        if (params.isEmpty()) return false;
        const bool back = e->key() == Qt::Key_Backtab || (mods & Qt::ShiftModifier);
        int current = -1;
        for (int i = 0; i < params.size(); ++i) if (params[i].id == selection->id()) current = i;
        const int n = int(params.size());
        const int next = current < 0 ? (back ? n - 1 : 0) : (current + (back ? -1 : 1) + n) % n;
        selection->set(params[next].id);
        reveal(params[next].id);
        return true;
    }

    const ParamInfo* param = nullptr;
    for (const ParamInfo& p : params) if (p.id == selection->id()) param = &p;
    if (param) {
        if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
            stepParameter(*param, e->key() == Qt::Key_Up ? 1 : -1, mods);
            return true;
        }
        if (e->key() == Qt::Key_Escape) {
            selection->clear();
            return true;
        }
        const QString text = e->text();
        if (!stage && param->type == ParamInfo::Standard && !(mods & (Qt::ControlModifier | Qt::AltModifier)) &&
            text.size() == 1 && (text[0].isDigit() || text[0] == '-' || text[0] == '.'))
            return inspector_->beginEdit(param->id, text);
    }
    // T taps the tempo, anywhere (auto-repeat isn't a tap).
    if (e->key() == Qt::Key_T && !(mods & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
        if (!e->isAutoRepeat()) EngineBridge::instance()->tapBpm();
        return true;
    }
    // Stage mode, as on macOS: S saves a screenshot of the output (Shift+S
    // chooses the folder first), C copies it.
    if (stage && !(mods & (Qt::ControlModifier | Qt::AltModifier)) && (e->key() == Qt::Key_S || e->key() == Qt::Key_C)) {
        const QString id = stage_->outputId();
        if (id.isEmpty()) return false;
        const QImage image = editor_->captureNode(id, EngineBridge::instance()->resolution());
        if (image.isNull()) return true;
        if (e->key() == Qt::Key_C) {
            QApplication::clipboard()->setImage(image);
            toast_->show("Copied the output");
            return true;
        }
        if ((mods & Qt::ShiftModifier) && Screenshots::chooseFolder(this).isEmpty()) return true;
        const QString file = Screenshots::save(image, this);
        if (!file.isEmpty()) {
            Flash::over(stage_);
            toast_->show("Screenshot saved to " + QDir::toNativeSeparators(QFileInfo(file).absolutePath()));
        }
        return true;
    }
    if (e->key() == Qt::Key_Escape && !stage && !editor_->selectedNodeIds().isEmpty()) {
        editor_->scene()->clearSelection();
        return true;
    }
    // The canvas's keys work wherever focus is (macOS handles them
    // app-wide): delete, Space to pause, copy / paste / select all, undo.
    if (!stage && target != editor_ && !editor_->isAncestorOf(target)) {
        const bool ctrl = mods & Qt::ControlModifier;
        const int key = e->key();
        const bool canvasKey = key == Qt::Key_Delete || key == Qt::Key_Backspace ||
                               (key == Qt::Key_Space && !qobject_cast<QAbstractButton*>(focus)) ||
                               (ctrl && (key == Qt::Key_C || key == Qt::Key_V || key == Qt::Key_A ||
                                         key == Qt::Key_Z || key == Qt::Key_Y));
        if (canvasKey) {
            QApplication::sendEvent(editor_, e);
            return true;
        }
    }
    return false;
}

// One keyboard step: 1% of the range (Shift 10%, Alt 0.1%); ints by 1 (Shift
// 10); bools on for Up, off for Down. Repeats build on the last value set,
// since the controls only refresh a few times a second.
void MainWindow::stepParameter(const ParamInfo& p, int direction, Qt::KeyboardModifiers mods) {
    static QString lastId;
    static double lastValue = 0;
    static QElapsedTimer lastTime;
    const bool recent = lastId == p.id && lastTime.isValid() && lastTime.elapsed() < 600;
    auto* eng = EngineBridge::instance();
    switch (p.type) {
    case ParamInfo::Standard: {
        const double range = p.max - p.min;
        const double fraction = (mods & Qt::ShiftModifier) ? 0.1 : (mods & Qt::AltModifier) ? 0.001 : 0.01;
        const double from = recent ? lastValue : p.value;
        const double value = std::clamp(from + direction * std::max(range * fraction, 1e-6),
                                        double(std::min(p.min, p.max)), double(std::max(p.min, p.max)));
        eng->setParameter(p.id, float(value));
        lastValue = value;
        break;
    }
    case ParamInfo::Int: {
        const int step = (mods & Qt::ShiftModifier) ? 10 : 1;
        const int from = recent ? int(lastValue) : p.intValue;
        const int value = std::clamp(from + direction * step, int(p.min), int(p.max));
        eng->setParameter(p.id, value);
        lastValue = value;
        break;
    }
    case ParamInfo::Bool:
        eng->setParameter(p.id, direction > 0);
        emit inspector_->graphMayChange();  // bools can add or remove pins
        break;
    default:
        return;
    }
    lastId = p.id;
    lastTime.restart();
}

bool MainWindow::eventFilter(QObject* watched, QEvent* e) {
    if (e->type() == QEvent::KeyPress && handleParameterKey(watched, static_cast<QKeyEvent*>(e))) return true;
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
            connect(settings_, &SettingsDialog::restartRequested, this, [this] {
                // main() relaunches once this instance has saved and shut down.
                qApp->setProperty("restartRequested", true);
                close();
            });
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
    // Menu toggles mirroring settings: they set them, and follow them (Prefs::changed below).
    auto* stretch = view->addAction("S&tretch Preview Windows", this, [](bool on) { PreviewWindow::setStretchToFill(on); });
    stretch->setCheckable(true);
    stretch->setChecked(PreviewWindow::stretchToFill());
    hintsAction_ = view->addAction("Show Node &Hints", this, [](bool on) { Prefs::set("display.nodeHints", on ? 1.0 : 0.0); });
    hintsAction_->setCheckable(true);
    hintsAction_->setChecked(Prefs::boolean("display.nodeHints"));
    auto* showLog = view->addAction("Show &Log", this, [this](bool on) { logPanel_->setVisible(on); });
    showLog->setCheckable(true);
    view->addSeparator();
    auto* clearOnDisconnect = view->addAction("Clear on Disconnect", this, [](bool on) {
        Prefs::set("graph.clearOnDisconnect", on ? 1.0 : 0.0);
    });
    clearOnDisconnect->setCheckable(true);
    clearOnDisconnect->setChecked(Prefs::boolean("graph.clearOnDisconnect"));
    connect(Prefs::instance(), &Prefs::changed, this, [stretch, clearOnDisconnect, this](const QString& id) {
        if (id == "output.stretchPreviews") stretch->setChecked(Prefs::boolean(id));
        else if (id == "display.nodeHints") hintsAction_->setChecked(Prefs::boolean(id));
        else if (id == "graph.clearOnDisconnect") clearOnDisconnect->setChecked(Prefs::boolean(id));
    });

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
    updateAction_ = help->addAction(QString());
    updateAction_->setVisible(false);
    updates_ = new UpdateCheck(this);
    connect(updates_, &UpdateCheck::available, this, [this, help](const QString& version, const QUrl& url, const QString& notes) {
        updateAction_->setText(QString("Download Nottawa %1…").arg(version));
        updateAction_->setToolTip(notes);
        updateAction_->setVisible(true);
        help->setToolTipsVisible(!notes.isEmpty());
        disconnect(updateAction_, &QAction::triggered, nullptr, nullptr);
        connect(updateAction_, &QAction::triggered, this, [url] { QDesktopServices::openUrl(url); });
        // Mentioned once, quietly, and never over a performance.
        if (!stageAction_ || !stageAction_->isChecked())
            toast_->show(QString("Nottawa %1 is available: Help > Download Nottawa %1").arg(version), 8000);
        log(QString("[update] %1 available at %2").arg(version, url.toString()));
    });
    QTimer::singleShot(5000, updates_, [this] { updates_->start(); });
    help->addAction("Nottawa &Tutorial", this, [this] { help_->restart(); });
    help->addAction("Show &Welcome Screen", this, [this] {
        welcome_->setGeometry(centre_->rect());
        welcome_->present();
    });
    help->addSeparator();
    help->addAction("Visit nottawa.app", this, [] { QDesktopServices::openUrl(QUrl("https://nottawa.app")); });
    help->addAction("Send Feedback…", this, [] {
        QDesktopServices::openUrl(QUrl("mailto:joe@nottawa.app?subject=Nottawa%20Feedback"));
    });
    // engine.log, with the stack of any crash or hang: to attach to a report.
    help->addAction("Show &Logs Folder", this, [] {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QDir(qApp->property("appDataDir").toString()).filePath("logs")));
    });
    help->addSeparator();
    help->addAction("&About Nottawa", this, [this] {
        QMessageBox::about(this, "About Nottawa",
                           QString("<b>Nottawa</b> %1<br>Audio-reactive visuals for live performance.<br><br>"
                                   "Free software under the GNU GPL v3. Third-party notices are in the "
                                   "licenses folder next to Nottawa.exe.")
                               .arg(QCoreApplication::applicationVersion()));
    });
}

void MainWindow::handleExternalRequest(const QString& request) {
    if (isMinimized()) showNormal();
    raise();
    activateWindow();
    SetForegroundWindow(reinterpret_cast<HWND>(winId()));  // the sender allowed it
    if (request.isEmpty()) return;
    const QUrl url(request);
    const QStringList path = url.path().split('/', Qt::SkipEmptyParts);
    static const QRegularExpression slugPattern("^[A-Za-z0-9_-]{1,128}$");
    if (url.scheme().compare("nottawa", Qt::CaseInsensitive) != 0 || url.host().compare("strands", Qt::CaseInsensitive) != 0
        || path.isEmpty() || !slugPattern.match(path.first()).hasMatch()) {
        log("[link] ignored " + request);
        toast_->show("Nottawa can't open that link", 5000);
        return;
    }
    log("[link] opening shared strand " + path.first());
    editor_->loadSharedStrand(path.first());
    welcome_->hide();
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
    // Delete also removes a selected cable, as on macOS.
    const bool deletable = !editor_->scene()->selectedItems().isEmpty();
    deleteButton_->setVisible(deletable);
    deleteSeparator_->setVisible(deletable);
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
    Prefs::instance()->engineStarted();  // moves old settings into the engine the first time
    const auto sources = engine->sources();
    const auto effects = engine->effects();
    browser_->reload();
    const QSize res = engine->resolution();
    log(QString("[engine] running at %1x%2 — %3 sources, %4 effects available")
        .arg(res.width()).arg(res.height()).arg(sources.size()).arg(effects.size()));
    refreshGraph();
    if (Prefs::boolean("output.reopenWindows")) editor_->restoreOutputWindows(QSettings().value("outputWindows/open").toList());
    editor_->setFocus();
    // Scripted runs skip it so their clicks land on the canvas.
    const bool scripted = !qEnvironmentVariableIsEmpty("NOTTAWA_ADD") || StressTest::requested();
    if (WelcomeOverlay::enabledAtLaunch() && !scripted) {
        welcome_->setGeometry(centre_->rect());
        welcome_->present();
    }
    if (!scripted) help_->autoStartIfFirstLaunch();
    runScriptedAdds();
    if (StressTest::requested()) (new StressTest(this))->start();
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
            // "file:<path>" adds a video or image file.
            if (name.startsWith("file:")) {
                const bool ok = !editor_->addMediaFile(name.mid(5)).isEmpty();
                log(QString("[script] %1 %2").arg(ok ? "added" : "ERROR could not add", name.mid(5)));
                return;
            }
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
    // The selected node, else the most recent effect, else the most recent
    // node (the macOS action bar's choice).
    QString id = selectedId_;
    if (id.isEmpty()) {
        const QVector<GraphNode> nodes = engine->nodes();
        for (const GraphNode& n : nodes) if (n.isShader) id = n.id;
        if (id.isEmpty() && !nodes.isEmpty()) id = nodes.last().id;
    }
    if (id.isEmpty()) return;
    const QImage image = editor_->captureNode(id, engine->resolution());
    if (image.isNull()) { toast_->show("Nothing to capture yet"); return; }
    const QString file = Screenshots::save(image, this);
    if (!file.isEmpty()) {
        Flash::over(centre_);
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
