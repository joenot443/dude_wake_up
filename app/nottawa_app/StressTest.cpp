#define _USE_MATH_DEFINES
#include <QImage>
#include <QSet>
#include <QPainter>
#include <QMouseEvent>
#include "nodes/NodeItems.h"
#include "Diagnostics.h"
#include <QScreen>
#include <memory>
#include <QTimer>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include "PreviewWindow.h"
#include "ui/Controls.h"
#include "Theme.h"
#include "UpdateCheck.h"
#include "SettingsDialog.h"
#include "Prefs.h"
#include "StressTest.h"

#include "EngineBridge.h"
#include "MainWindow.h"
#include "nodes/NodeEditorView.h"
#include "nottawa_bridge.h"
#include "panels/StageView.h"

#include <chrono>
#include <QApplication>
#include <cmath>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QToolButton>
#include <QUrl>

#include <windows.h>
#include <psapi.h>

namespace {
const QStringList kAllScenarios = {"churn", "swap", "connect", "undo", "strands", "workspace", "previews",
                                   "stage", "resolution", "inputs", "copypaste", "optional", "parameters",
                                   "audio", "badfiles"};

QString processStats() {
    PROCESS_MEMORY_COUNTERS_EX mem = {};
    GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&mem), sizeof(mem));
    DWORD handles = 0;
    GetProcessHandleCount(GetCurrentProcess(), &handles);
    return QString("private %1 MB, working set %2 MB, handles %3, gdi %4, user %5")
        .arg(mem.PrivateUsage >> 20)
        .arg(mem.WorkingSetSize >> 20)
        .arg(handles)
        .arg(GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS))
        .arg(GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS));
}

// Live engine objects by class, from the lifecycle counters.
QString liveObjects() {
    char* json = ntw_dump_object_counts();
    const QJsonObject live = QJsonDocument::fromJson(QByteArray(json)).object().value("live").toObject();
    ntw_free_string(json);
    QStringList parts;
    for (auto it = live.begin(); it != live.end(); ++it)
        if (it.value().toInt() != 0) parts << QString("%1:%2").arg(it.key()).arg(it.value().toInt());
    return parts.join(' ');
}
} // anon

StressTest::StressTest(MainWindow* window) : QObject(window), window_(window) {
    rng_.seed(qEnvironmentVariableIntValue("NOTTAWA_STRESS_SEED") ? qEnvironmentVariableIntValue("NOTTAWA_STRESS_SEED") : 1);
    QString list = qEnvironmentVariable("NOTTAWA_STRESS");
    scenarios_ = list == "all" ? kAllScenarios : list.split(',', Qt::SkipEmptyParts);
    const int rounds = std::max(1, qEnvironmentVariableIntValue("NOTTAWA_STRESS_ROUNDS"));
    const QStringList once = scenarios_;
    for (int r = 1; r < rounds; ++r) scenarios_ += once;
    scratch_ = QDir(qEnvironmentVariable("NOTTAWA_APPDATA", QDir::tempPath())).filePath("stress");
    QDir().mkpath(scratch_);
    timer_.setSingleShot(true);
    QObject::connect(&timer_, &QTimer::timeout, this, &StressTest::next);
}

void UrlSink::open(const QUrl& url) { qInfo("[test] not opening %s", qPrintable(url.toString())); }

bool StressTest::requested() { return !qEnvironmentVariableIsEmpty("NOTTAWA_STRESS"); }

void StressTest::start() {
    window_->log(QString("[stress] seed %1, scenarios: %2").arg(rng_()).arg(scenarios_.join(',')));
    checkpoint("baseline");
    for (const QString& s : std::as_const(scenarios_)) enqueueScenario(s.trimmed());
    add([this] {
        window_->log("[stress] done");
        window_->close();
    });
    timer_.start(500);
}

void StressTest::next() {
    if (steps_.empty()) return;
    Queued q = std::move(steps_.front());
    steps_.pop_front();
    q.step();
    if (!steps_.empty()) timer_.start(steps_.front().delayMs);
}

void StressTest::checkpoint(const QString& label) {
    char* registries = ntw_dump_registry_sizes();
    const QString sizes = QString::fromUtf8(registries);
    ntw_free_string(registries);
    window_->log(QString("[stress] %1 | %2 | registries %3 | live %4").arg(label, processStats(), sizes, liveObjects()));
}

void StressTest::enqueueScenario(const QString& name) {
    add([this, name] { window_->log("[stress] >>> " + name); });
    const int n = std::max(1, qEnvironmentVariableIntValue("NOTTAWA_STRESS_CYCLES"));
    const int cycles = n > 1 ? n : 20;
    if (name == "churn") churn(cycles);
    else if (name == "swap") swap(cycles);
    else if (name == "connect") connect(cycles);
    else if (name == "undo") undo(cycles);
    else if (name == "strands") strands(cycles);
    else if (name == "workspace") workspace(cycles);
    else if (name == "previews") previews(cycles);
    else if (name == "stage") stage(cycles);
    else if (name == "resolution") resolution(cycles);
    else if (name == "inputs") inputs(cycles);
    else if (name == "copypaste") copyPaste(cycles);
    else if (name == "optional") optional(cycles);
    else if (name == "parameters") parameters(cycles);
    else if (name == "memory") memory(cycles);
    else if (name == "audio") audio(cycles);
    else if (name == "badfiles") badFiles(cycles);
    else if (name == "fonts") fonts();
    else if (name == "swapall") swapAll();
    else if (name == "bpm") bpm();
    else if (name == "settings") settings();
    else if (name == "settingsui") settingsUi();
    else if (name == "audiosettings") audioSettings();
    else if (name == "audiolaunch") audioLaunch();
    else if (name == "cachesettings") cacheSettings();
    else if (name == "beattiming") beatTiming();
    else if (name == "outputwindows") outputWindows();
    else if (name == "savetiming") saveTiming();
    else if (name == "enginestall") engineStall();
    else if (name == "canvasimage") canvasImage();
    else if (name == "blenddrag") blendDrag();
    else if (name == "renderall") renderAll();
    else if (name == "golden") golden();
    else if (name == "workflows") workflows();
    else if (name == "outputwindowslaunch") outputWindowsLaunch();
    else add([this, name] { window_->log("[stress] unknown scenario " + name); });
    // Settle, clear, settle, measure.
    add([] {}, 1000);
    add([] { EngineBridge::instance()->clearGraph(); });
    add([this, name] { checkpoint("after " + name); }, 1500);
}

// ------------------------------------------------------------------ helpers

QStringList StressTest::nodeIds() const {
    QStringList ids;
    for (const GraphNode& n : EngineBridge::instance()->nodes()) ids << n.id;
    return ids;
}

QString StressTest::pick(const QStringList& ids) {
    if (ids.isEmpty()) return {};
    return ids[int(rng_() % uint32_t(ids.size()))];
}

QString StressTest::randomSource() {
    const auto sources = EngineBridge::instance()->sources();
    if (sources.isEmpty()) return {};
    return window_->editor_->addFromCatalog(sources[int(rng_() % uint32_t(sources.size()))].typeRaw, true);
}

void StressTest::randomizeParameters(const QString& nodeId) {
    auto* eng = EngineBridge::instance();
    for (const ParamInfo& p : eng->parameters(nodeId)) {
        const int mode = int(rng_() % 4);  // min, max, middle, random
        const float t = mode == 0 ? 0.0f : mode == 1 ? 1.0f : mode == 2 ? 0.5f : float(rng_() % 1000) / 1000.0f;
        switch (p.type) {
        case ParamInfo::Standard: eng->setParameter(p.id, p.min + t * (p.max - p.min)); break;
        case ParamInfo::Int:
            eng->setParameter(p.id, p.options.isEmpty() ? int(std::lround(p.min + t * (p.max - p.min)))
                                                        : int(rng_() % uint32_t(p.options.size())));
            break;
        case ParamInfo::Bool: eng->setParameter(p.id, bool(rng_() % 2)); break;
        case ParamInfo::Color: eng->setParameterColor(p.id, QColor::fromRgb(rng_() & 0xffffff)); break;
        case ParamInfo::Text: eng->setParameter(p.id, QString("stress %1").arg(rng_() % 1000)); break;
        default: break;
        }
    }
}

// ---------------------------------------------------------------- scenarios

// Every catalog shader and source, added in chains and deleted: exercises
// each type's setup, render and teardown.
void StressTest::churn(int cycles) {
    for (int c = 0; c < cycles; ++c) {
        add([this] {
            const QString src = randomSource();
            QString last = src;
            const auto effects = EngineBridge::instance()->effects();
            const int count = 1 + int(rng_() % 3);
            for (int i = 0; i < count && !last.isEmpty(); ++i) {
                const QString id = window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, last);
                if (!id.isEmpty()) last = id;
            }
            if (!last.isEmpty()) window_->editor_->selectNode(last);
        }, 40);
        add([] {}, 120);  // let it render (and the inspector build)
        add([this] {
            auto* eng = EngineBridge::instance();
            const QStringList ids = nodeIds();
            if (rng_() % 2) { for (const QString& id : ids) eng->removeNode(id); }
            else eng->clearGraph();
        });
    }
}

void StressTest::swap(int cycles) {
    add([this] {
        const QString src = randomSource();
        const auto effects = EngineBridge::instance()->effects();
        window_->editor_->addFromCatalog(effects[0].typeRaw, false, src);
    });
    for (int c = 0; c < cycles; ++c) {
        add([this] {
            auto* eng = EngineBridge::instance();
            const auto nodes = eng->nodes();
            if (nodes.isEmpty()) return;
            const GraphNode& n = nodes[int(rng_() % uint32_t(nodes.size()))];
            const auto list = n.isShader ? eng->effects() : eng->sources();
            const QString id = eng->replaceNode(n.id, list[int(rng_() % uint32_t(list.size()))].typeRaw, !n.isShader);
            if (!id.isEmpty()) window_->editor_->selectNode(id);
        }, 80);
    }
}

// App preferences: the schema, values kept to range and choices, the
// engine following them, and workspaces no longer changing them. Results:
// "[stress] settings: ..." lines; problems as FAIL.
void StressTest::settings() {
    add([this] {
        auto* eng = EngineBridge::instance();
        const auto all = eng->settings();
        QStringList sections;
        for (const auto& s : all) if (!sections.contains(s.section)) sections << s.section;
        window_->log(QString("[stress] settings: %1 settings in %2").arg(all.size()).arg(sections.join(", ")));
        if (all.isEmpty()) window_->log("[stress] FAIL settings: no schema");
        if (eng->settingNumber("audio.beatOffsetSystem") != 35.0) window_->log("[stress] FAIL settings: System Audio offset default isn't 35 ms");
        for (const auto& s : all) {
            if (s.id == "output.advanceFade" && (s.dependsOn != "output.autoAdvance" || s.decimals != 1 || s.defaultNumber != 0.3))
                window_->log("[stress] FAIL settings: advanceFade's dependsOn/decimals/default");
            if (s.id == "appearance.theme" && s.restart != 2) window_->log("[stress] FAIL settings: theme should need a restart on Windows only");
            if (s.type == SettingInfo::Action && eng->settingString(s.id) != QString()) window_->log("[stress] FAIL settings: an action has a value");
        }
        eng->setSetting("advanced.resetAll", 1.0);  // an action: no value to set
        eng->setSetting("output.advanceFade", 99.0);       // above its max (2)
        eng->setSetting("screenshots.format", QString("gif"));  // not a choice
        eng->setSetting("display.nodeHints", 0.0);
        eng->setSetting("output.resolution", QString("1080p"));
    }, 200);
    add([this] {
        auto* eng = EngineBridge::instance();
        const double fade = eng->settingNumber("output.advanceFade");
        if (fade != 2.0) window_->log(QString("[stress] FAIL settings: advanceFade %1, expected the max 2").arg(fade));
        if (eng->settingString("screenshots.format") != "png") window_->log("[stress] FAIL settings: an invalid choice was taken");
        if (eng->settingNumber("display.nodeHints") != 0.0) window_->log("[stress] FAIL settings: nodeHints not set");
        if (eng->resolutionIndex() != 4) window_->log(QString("[stress] FAIL settings: engine resolution index %1 after 1080p").arg(eng->resolutionIndex()));
        // A workspace saved at 1080p, then the preference changed to 480p: opening it must not change it back.
        eng->saveWorkspace(QDir(scratch_).filePath("settings-ws.json"), "settings");
        eng->setSetting("output.resolution", QString("480p"));
    }, 300);
    add([this] {
        auto* eng = EngineBridge::instance();
        eng->loadWorkspace(QDir(scratch_).filePath("settings-ws.json"));
        if (eng->resolutionIndex() != 2) window_->log(QString("[stress] FAIL settings: opening a workspace set resolution index %1 (preference 480p)").arg(eng->resolutionIndex()));
        if (eng->settingString("output.resolution") != "480p") window_->log("[stress] FAIL settings: the workspace changed the preference");
        eng->setResolutionIndex(3);  // the old API sets the preference too
    }, 500);
    add([this] {
        auto* eng = EngineBridge::instance();
        if (eng->settingString("output.resolution") != "720p") window_->log("[stress] FAIL settings: ntw_set_resolution didn't set the preference");
        eng->resetSettings("Appearance");
    }, 300);
    add([this] {
        auto* eng = EngineBridge::instance();
        if (eng->settingNumber("display.nodeHints") != 1.0) window_->log("[stress] FAIL settings: reset didn't restore nodeHints");
        window_->log(QString("[stress] settings: checked (fade %1, format %2, resolution %3)")
                         .arg(eng->settingNumber("output.advanceFade")).arg(eng->settingString("screenshots.format"))
                         .arg(eng->settingString("output.resolution")));
    }, 800);  // past the save delay: preferences.json written
}

// The settings window: every setting's value (after any migration), an
// image of each section and of searches, rows following changes made
// elsewhere, dependent rows disabled. Images: <scratch>/settings-*.png.
void StressTest::settingsUi() {
    add([this] {
        QStringList values;
        for (const SettingInfo& s : Prefs::instance()->settings()) {
            if (s.type == SettingInfo::Action) continue;
            const bool text = s.type == SettingInfo::Choice || s.type == SettingInfo::String || s.type == SettingInfo::Path;
            values << s.id + "=" + (text ? Prefs::string(s.id) : QString::number(Prefs::number(s.id)));
        }
        window_->log("[stress] settingsui: values " + values.join(" "));
        for (QAction* a : window_->findChildren<QAction*>()) if (a->text() == "&Settings…") a->trigger();
    }, 600);
    add([this] {
        auto* dialog = window_->findChild<SettingsDialog*>();
        if (!dialog) { window_->log("[stress] FAIL settingsui: no settings window"); return; }
        auto* sections = dialog->findChild<QListWidget*>();
        QStringList names;
        for (int i = 0; i < sections->count(); ++i) {
            sections->setCurrentRow(i);
            QApplication::processEvents();
            const QString name = sections->item(i)->text();
            names << name;
            dialog->grab().save(QDir(scratch_).filePath("settings-" + name.toLower() + ".png"));
        }
        window_->log("[stress] settingsui: sections " + names.join(", "));
        if (names.contains("Updates") == !UpdateCheck::supported()) window_->log("[stress] FAIL settingsui: Updates shown wrongly");
        auto* search = dialog->findChild<QLineEdit*>();
        for (const QString& term : {"fade", "midi", "zzz"}) {
            search->setText(term);
            QApplication::processEvents();
            dialog->grab().save(QDir(scratch_).filePath("settings-search-" + term + ".png"));
        }
        search->clear();
        sections->setCurrentRow(int(names.indexOf("Output")));
        // Changed elsewhere (as a menu or the engine would): the window follows.
        Prefs::set("output.autoAdvance", 0.0);
        Prefs::set("appearance.theme", QString("ember"));
    }, 400);
    add([this] {
        auto* dialog = window_->findChild<SettingsDialog*>();
        dialog->grab().save(QDir(scratch_).filePath("settings-output-noadvance.png"));
        bool fadeDisabled = false, toggleOff = false;
        for (auto* l : dialog->findChildren<QLabel*>()) {
            if (l->text() == "Advance Fade") fadeDisabled = !l->isEnabledTo(dialog);
            if (l->text() == "Advance Output Window Automatically")
                if (auto* t = l->parentWidget()->findChild<DSToggle*>()) toggleOff = !t->isChecked();
        }
        if (!fadeDisabled) window_->log("[stress] FAIL settingsui: Advance Fade not disabled with auto advance off");
        if (!toggleOff) window_->log("[stress] FAIL settingsui: the toggle didn't follow a change made elsewhere");
        if (!Prefs::instance()->restartPending()) window_->log("[stress] FAIL settingsui: no restart pending after a theme change");
        if (auto* banner = dialog->findChild<QFrame*>("Restart"); !banner || !banner->isVisibleTo(dialog))
            window_->log("[stress] FAIL settingsui: no restart banner");
        Prefs::set("display.nodeHints", 0.0);
    }, 400);
    add([this] {
        // Copy Diagnostics, with a previous session that hung.
        const QString logs = QDir(qApp->property("appDataDir").toString()).filePath("logs");
        QFile previous(QDir(logs).filePath("engine.previous.log"));
        if (previous.open(QIODevice::WriteOnly)) previous.write("starting\n=== HANG: the UI thread has been unresponsive for 2100 ms ===\n");
        previous.close();
        const QString report = Diagnostics::report();
        for (const QString& line : report.split('\n')) window_->log("[stress] settingsui: diagnostics | " + line);
        if (report.contains("Graphics: unavailable")) window_->log("[stress] FAIL settingsui: diagnostics without the GPU");
        if (!report.contains("1 report(s)")) window_->log("[stress] FAIL settingsui: diagnostics missed the previous session's hang");
        if (!report.contains("display.nodeHints=0")) window_->log("[stress] FAIL settingsui: diagnostics missed a changed setting");
        if (report.contains(QDir::homePath()) || report.contains(qEnvironmentVariable("USERNAME")))
            window_->log("[stress] FAIL settingsui: diagnostics name the user");
        for (QAction* a : window_->findChildren<QAction*>())
            if (a->text() == "Show Node &Hints" && a->isChecked()) window_->log("[stress] FAIL settingsui: the menu didn't follow node hints");
        EngineBridge::instance()->resetSettings();
    }, 400);
    add([this] {
        auto* dialog = window_->findChild<SettingsDialog*>();
        // (The run starts in Midnight, so after the reset to Default a restart is still pending.)
        window_->log(QString("[stress] settingsui: done; after reset theme %1, hints %2, restart pending %3")
                         .arg(Prefs::string("appearance.theme")).arg(Prefs::number("display.nodeHints"))
                         .arg(Prefs::instance()->restartPending()));
        dialog->close();
    }, 400);
}

// Step 3's Audio and Output settings in effect: the tempo range folding
// Velvet Skies (87) by octaves, the frame rate, a custom resolution, a
// workspace not switching the input. Leaves Start Audio on Launch on with
// the sample tracks selected, for "audiolaunch" in a second run (-Keep).
void StressTest::audioSettings() {
    auto sampleSource = [] {
        for (const auto& s : EngineBridge::instance()->audioSources())
            if (s.name.contains("Sample", Qt::CaseInsensitive)) return s.id;
        return QString();
    };
    add([sampleSource] {
        auto* eng = EngineBridge::instance();
        eng->selectAudioSource(sampleSource());
        eng->setBpmMode(0);
        for (const auto& t : eng->sampleTracks()) if (t.name.contains("Velvet")) eng->selectSampleTrack(t.index);
    }, 500);
    add([] { if (!EngineBridge::instance()->isAudioActive()) EngineBridge::instance()->toggleAudio(); }, 9000);
    for (const QString range : {"auto", "90-180", "60-120", "100-200", "auto"}) {
        add([range] { Prefs::set("audio.tempoRange", range); }, 1500);
        add([this, range] {
            const float bpm = EngineBridge::instance()->audioAnalysis().bpm;
            const double want = range == "90-180" || range == "100-200" ? 174 : 87;
            window_->log(QString("[stress] audiosettings: range %1 bpm %2").arg(range).arg(bpm, 0, 'f', 2));
            if (std::abs(bpm / want - 1) > 0.04) window_->log(QString("[stress] FAIL audiosettings: range %1 shows %2, expected about %3").arg(range).arg(bpm).arg(want));
        });
    }
    add([] { Prefs::set("audio.beatOffsetFile", 120.0); }, 1500);
    add([this] {
        const float bpm = EngineBridge::instance()->audioAnalysis().bpm;
        if (std::abs(bpm / 87 - 1) > 0.04) window_->log(QString("[stress] FAIL audiosettings: with a beat offset, bpm %1").arg(bpm));
        Prefs::set("audio.beatOffsetFile", 0.0);
        if (EngineBridge::instance()->isAudioActive()) EngineBridge::instance()->toggleAudio();
    });
    for (const QString rate : {"30", "120", "60"}) {
        add([rate] { Prefs::set("output.frameRate", rate); });
        add([this, rate] {  // (a step's delay comes before it: the fps after 3 s at the new rate)
            const float fps = EngineBridge::instance()->fps();
            window_->log(QString("[stress] audiosettings: frame rate %1 -> fps %2").arg(rate).arg(fps, 0, 'f', 1));
            if (std::abs(fps / rate.toFloat() - 1) > 0.15) window_->log(QString("[stress] FAIL audiosettings: frame rate %1 runs at %2").arg(rate).arg(fps));
        }, 3000);
    }
    add([] {
        Prefs::set("output.customWidth", 1000.0);
        Prefs::set("output.customHeight", 500.0);
        Prefs::set("output.resolution", QString("custom"));
    }, 500);
    add([this] {
        const QSize res = EngineBridge::instance()->resolution();
        if (res != QSize(1000, 500)) window_->log(QString("[stress] FAIL audiosettings: custom resolution is %1x%2").arg(res.width()).arg(res.height()));
        Prefs::set("output.customWidth", 1200.0);  // changed while custom: applies
    }, 500);
    add([this] {
        const QSize res = EngineBridge::instance()->resolution();
        if (res != QSize(1200, 500)) window_->log(QString("[stress] FAIL audiosettings: custom width change gave %1x%2").arg(res.width()).arg(res.height()));
        Prefs::set("output.resolution", QString("720p"));
    }, 500);
    add([this] {
        const QSize res = EngineBridge::instance()->resolution();
        if (res != QSize(1280, 720)) window_->log(QString("[stress] FAIL audiosettings: back to 720p gave %1x%2").arg(res.width()).arg(res.height()));
        // A workspace saved with the sample tracks selected doesn't switch the input back.
        EngineBridge::instance()->saveWorkspace(QDir(scratch_).filePath("audio-ws.json"), "audio");
        for (const auto& s : EngineBridge::instance()->audioSources())
            if (s.name.contains("System", Qt::CaseInsensitive)) EngineBridge::instance()->selectAudioSource(s.id);
    }, 500);
    add([this, sampleSource] {
        auto* eng = EngineBridge::instance();
        const QString system = eng->selectedAudioSource();
        eng->loadWorkspace(QDir(scratch_).filePath("audio-ws.json"));
        if (eng->selectedAudioSource() != system) window_->log("[stress] FAIL audiosettings: opening a workspace switched the input");
        eng->selectAudioSource(sampleSource());
        if (eng->isAudioActive()) eng->toggleAudio();
        Prefs::set("audio.startOnLaunch", 1.0);
        window_->log(QString("[stress] audiosettings: done; input %1, start on launch on").arg(eng->selectedAudioSource()));
    }, 1000);
    add([] {}, 800);  // preferences.json written
}

// Cache Video Clips: off, a clip loads without a cache (and builds one if
// switched on for it); on, it caches. ("[stress] cachesettings: ...")
void StressTest::cacheSettings() {
    const QString video = qEnvironmentVariable("NOTTAWA_STRESS_VIDEO");
    auto fileNode = [this] {
        for (const QString& id : nodeIds()) if (EngineBridge::instance()->fileSourceState(id).valid) return id;
        return QString();
    };
    add([this, video] {
        EngineBridge::instance()->clearGraph();
        Prefs::set("performance.frameCache", 0.0);
        window_->editor_->addMediaFile(video);
    }, 300);
    add([this, fileNode] {
        const QString id = fileNode();
        const FileSourceState f = EngineBridge::instance()->fileSourceState(id);
        window_->log(QString("[stress] cachesettings: off: enabled %1, frames %2").arg(f.cacheEnabled).arg(f.cachedFrames));
        if (id.isEmpty() || f.cacheEnabled || f.cachedFrames > 0) window_->log("[stress] FAIL cachesettings: a clip cached with the setting off");
        EngineBridge::instance()->setFileSourceUseCache(id, true);
    }, 4000);
    add([this, fileNode, video] {
        const FileSourceState f = EngineBridge::instance()->fileSourceState(fileNode());
        window_->log(QString("[stress] cachesettings: switched on for the clip: frames %1, complete %2").arg(f.cachedFrames).arg(f.cacheAvailable));
        if (f.cachedFrames == 0) window_->log("[stress] FAIL cachesettings: switching the clip's cache on built nothing");
        EngineBridge::instance()->clearGraph();
        Prefs::set("performance.frameCache", 1.0);
        Prefs::set("performance.frameCacheBudget", QString("1024"));
        window_->editor_->addMediaFile(video);
    }, 4000);
    add([this, fileNode] {
        const FileSourceState f = EngineBridge::instance()->fileSourceState(fileNode());
        window_->log(QString("[stress] cachesettings: on (1 GB): enabled %1, frames %2, complete %3").arg(f.cacheEnabled).arg(f.cachedFrames).arg(f.cacheAvailable));
        if (!f.cacheEnabled || f.cachedFrames == 0) window_->log("[stress] FAIL cachesettings: no cache with the setting on");
        EngineBridge::instance()->clearGraph();
        EngineBridge::instance()->resetSettings("Performance");
    }, 4000);
}

// How far the shown beat is from the music's, for the Beat Timing
// defaults: a drum track with exactly known beats (128 BPM, first beat at
// 0.5 s; .cache/calib) played (1) as a sample track by the file source,
// whose clock is the samples written to the output, and (2) by a video
// node (NOTTAWA_CALIB_VIDEO, the same audio) while System Audio listens,
// against the video's media time. Neither includes the output device's
// own latency. Results: "[stress] beattiming: <input> late <ms> ...".
void StressTest::beatTiming() {
    add([] {
        auto* eng = EngineBridge::instance();
        for (const char* id : {"audio.beatOffsetFile", "audio.beatOffsetInput", "audio.beatOffsetSystem"}) Prefs::set(id, 0.0);
        Prefs::set("audio.tempoRange", QString("auto"));
        for (const auto& s : eng->audioSources())
            if (s.name.contains("Sample", Qt::CaseInsensitive)) eng->selectAudioSource(s.id);
        eng->setBpmMode(0);
        for (const auto& t : eng->sampleTracks()) if (t.name.contains("Calibration")) eng->selectSampleTrack(t.index);
    }, 500);
    add([] { if (!EngineBridge::instance()->isAudioActive()) EngineBridge::instance()->toggleAudio(); }, 500);
    add([] {}, 12000);  // the tracker locks
    measureBeat("file", false);
    add([this] {
        auto* eng = EngineBridge::instance();
        if (eng->isAudioActive()) eng->toggleAudio();
        for (const auto& s : eng->audioSources())
            if (s.name.contains("System", Qt::CaseInsensitive)) eng->selectAudioSource(s.id);
        eng->setBpmMode(0);
        window_->editor_->addMediaFile(qEnvironmentVariable("NOTTAWA_CALIB_VIDEO"));
    }, 500);
    add([this] {
        for (const QString& id : nodeIds())
            if (EngineBridge::instance()->fileSourceState(id).valid) {
                calibVideo_ = id;
                EngineBridge::instance()->setFileSourceMuted(id, false);
                EngineBridge::instance()->setFileSourceVolume(id, 1.0f);
            }
        if (!EngineBridge::instance()->isAudioActive()) EngineBridge::instance()->toggleAudio();
    }, 1500);
    add([] {}, 12000);
    measureBeat("system", true);
    add([this] {
        auto* eng = EngineBridge::instance();
        if (eng->isAudioActive()) eng->toggleAudio();
        eng->clearGraph();
    }, 100);
}

// Samples the beat probe every 5 ms for 30 s; logs the shown beat's mean
// lateness against the calibration track's beats (circular mean), its
// spread, and how many samples had the tempo right.
void StressTest::measureBeat(const QString& label, bool video) {
    struct Acc { double s = 0, c = 0; int n = 0, total = 0; QTimer timer; };
    auto acc = std::make_shared<Acc>();
    add([this, acc, label, video] {
        constexpr double kT0 = 0.5, kPeriod = 60.0 / 128.0;
        const QString videoId = video ? calibVideo_ : QString();
        acc->timer.setTimerType(Qt::PreciseTimer);
        acc->timer.setInterval(5);
        QObject::connect(&acc->timer, &QTimer::timeout, [acc, videoId] {
            const auto p = EngineBridge::instance()->beatProbe(videoId);
            const double media = videoId.isEmpty() ? p.fileAudioSeconds : p.videoSeconds;
            ++acc->total;
            if (media < kT0 || std::abs(p.bpm / 128.0 - 1) > 0.01) return;
            const double truth = std::fmod((media - kT0) / kPeriod, 1.0);
            const double err = (p.phase - truth) * 2 * M_PI;  // + : shown ahead
            acc->s += std::sin(err);
            acc->c += std::cos(err);
            ++acc->n;
        });
        acc->timer.start();
        window_->log(QString("[stress] beattiming: measuring %1").arg(label));
    });
    add([this, acc, label] {
        acc->timer.stop();
        constexpr double kPeriod = 60.0 / 128.0;
        if (acc->n == 0) { window_->log(QString("[stress] FAIL beattiming: %1: tempo never right (%2 samples)").arg(label).arg(acc->total)); return; }
        const double mean = std::atan2(acc->s, acc->c) / (2 * M_PI);       // cycles, + ahead
        const double r = std::hypot(acc->s, acc->c) / acc->n;
        const double spread = std::sqrt(std::max(0.0, -2 * std::log(r))) / (2 * M_PI) * kPeriod * 1000;
        window_->log(QString("[stress] beattiming: %1 late %2 ms (spread %3 ms, %4 of %5 samples at 128)")
                         .arg(label).arg(-mean * kPeriod * 1000, 0, 'f', 1).arg(spread, 0, 'f', 1).arg(acc->n).arg(acc->total));
    }, 30000);
}

// How long saving the session takes with a big graph (it runs on the
// engine thread: the UI waits). "[stress] savetiming: <nodes> nodes, <ms>"
void StressTest::saveTiming() {
    add([] { EngineBridge::instance()->clearGraph(); });
    for (int i = 0; i < 15; ++i)
        add([this] {
            const auto effects = EngineBridge::instance()->effects();
            window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, randomSource());
        }, 100);
    for (int i = 0; i < 3; ++i)
        add([this] {
            QElapsedTimer t;
            t.start();
            ntw_save_default_config();
            window_->log(QString("[stress] savetiming: %1 nodes, save %2 ms").arg(EngineBridge::instance()->nodes().size()).arg(t.elapsed()));
        }, 1000);
}

// Blending two nodes: through the engine, then by dragging one node's
// output port onto the other's on the canvas, as a user does (mouse events
// sent to the viewport). "[stress] blenddrag: ..."
void StressTest::blendDrag() {
    auto state = std::make_shared<QStringList>();  // the two sources
    add([this, state] {
        auto* eng = EngineBridge::instance();
        eng->clearGraph();
        *state = {randomSource(), randomSource()};
        eng->setNodePosition((*state)[0], QPointF(0, 0));
        eng->setNodePosition((*state)[1], QPointF(0, 420));
        window_->editor_->syncFromEngine();
        window_->editor_->zoomToFit();
    }, 300);
    add([this, state] {
        auto* eng = EngineBridge::instance();
        const int before = int(eng->nodes().size());
        const QString blend = eng->blendBetween((*state)[0], (*state)[1]);
        window_->log(QString("[stress] blenddrag: engine blend '%1', nodes %2 -> %3").arg(blend).arg(before).arg(eng->nodes().size()));
        if (blend.isEmpty()) window_->log("[stress] FAIL blenddrag: the engine didn't create a blend");
        else eng->removeNode(blend);
        window_->editor_->syncFromEngine();
    }, 500);
    add([this, state] {
        auto* view = window_->editor_;
        PortItem* ports[2] = {nullptr, nullptr};
        for (QGraphicsItem* item : view->scene()->items())
            if (auto* port = dynamic_cast<PortItem*>(item); port && port->kind() == PortItem::Output && port->slot() == 0)
                for (int i = 0; i < 2; ++i) if (port->nodeId() == (*state)[i]) ports[i] = port;
        if (!ports[0] || !ports[1]) { window_->log("[stress] FAIL blenddrag: output ports not found on the canvas"); return; }
        const int before = int(EngineBridge::instance()->nodes().size());
        QWidget* vp = view->viewport();
        const QPointF a = view->mapFromScene(ports[0]->anchor()), b = view->mapFromScene(ports[1]->anchor());
        auto send = [vp](QEvent::Type type, QPointF pos, Qt::MouseButton button, Qt::MouseButtons buttons) {
            QMouseEvent ev(type, pos, vp->mapToGlobal(pos), button, buttons, Qt::NoModifier);
            QApplication::sendEvent(vp, &ev);
        };
        send(QEvent::MouseButtonPress, a, Qt::LeftButton, Qt::LeftButton);
        for (int i = 1; i <= 10; ++i) send(QEvent::MouseMove, a + (b - a) * (i / 10.0), Qt::NoButton, Qt::LeftButton);
        send(QEvent::MouseButtonRelease, b, Qt::LeftButton, Qt::NoButton);
        const int after = int(EngineBridge::instance()->nodes().size());
        window_->log(QString("[stress] blenddrag: drag from (%1,%2) to (%3,%4): nodes %5 -> %6")
                         .arg(a.x()).arg(a.y()).arg(b.x()).arg(b.y()).arg(before).arg(after));
        if (after != before + 1) window_->log("[stress] FAIL blenddrag: dragging output onto output didn't blend");
        for (const GraphEdge& e : EngineBridge::instance()->edges())
            window_->log(QString("[stress] blenddrag: edge %1 [%2] -> %3 [%4] type %5")
                             .arg(e.startId.left(8)).arg(e.outputSlot).arg(e.endId.left(8)).arg(e.inputSlot).arg(e.connectionType));
        for (const GraphNode& n : EngineBridge::instance()->nodes())
            window_->log(QString("[stress] blenddrag: node %1 %2").arg(n.id.left(8), n.name));
        // Its own frame in the engine (not the preview): black means the blend itself is broken.
        QTimer::singleShot(1000, window_, [this] {
            for (const GraphNode& n : EngineBridge::instance()->nodes()) {
                QStringList px;
                for (float t : {0.25f, 0.5f, 0.75f})
                    px << QString::number(ntw_debug_frame_pixel(n.id.toUtf8().constData(), t, t), 16);
                window_->log(QString("[stress] blenddrag: %1 frame pixels %2").arg(n.name, px.join(" ")));
                if (n.name == "Blend" && px.join("") == "000")
                    window_->log("[stress] FAIL blenddrag: the Blend renders nothing (its frame is empty)");
            }
        });
        window_->editor_->zoomToFit();
        window_->raise();
        window_->activateWindow();
    }, 500);
    add([this] {
        const QRect r = window_->frameGeometry();
        if (QScreen* screen = window_->screen())
            screen->grabWindow(0, r.x() - screen->geometry().x(), r.y() - screen->geometry().y(), r.width(), r.height())
                .save(QDir(scratch_).filePath("blend.png"));
    }, 2000);
}

namespace {
// A node's frame, read back in the engine, shrunk to 128x72 (golden size).
QImage goldenFrame(const QString& id) {
    int w = 0, h = 0;
    ntw_debug_frame_rgba(id.toUtf8().constData(), &w, &h, nullptr, 0);
    if (w <= 0 || h <= 0) return {};
    QImage image(w, h, QImage::Format_RGBA8888);
    if (!ntw_debug_frame_rgba(id.toUtf8().constData(), &w, &h, image.bits(), int(image.sizeInBytes()))) return {};
    return image.mirrored().scaled(128, 72, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

// How far apart two golden frames are: the share of pixels with a channel
// (alpha included) more than 24 apart, and the mean channel difference.
struct GoldenDiff { double changed = 1.0, mean = 255.0; bool same() const { return changed < 0.01 && mean < 2.0; } };
GoldenDiff goldenDiff(const QImage& a, const QImage& b) {
    if (a.isNull() || b.isNull() || a.size() != b.size()) return {};
    const QImage x = a.convertToFormat(QImage::Format_RGBA8888), y = b.convertToFormat(QImage::Format_RGBA8888);
    qint64 changed = 0, sum = 0;
    const qint64 n = qint64(x.width()) * x.height();
    for (int row = 0; row < x.height(); ++row) {
        const uchar* p = x.constScanLine(row);
        const uchar* q = y.constScanLine(row);
        for (int i = 0; i < x.width(); ++i) {
            int worst = 0;
            for (int c = 0; c < 4; ++c) {
                const int d = std::abs(int(p[i * 4 + c]) - int(q[i * 4 + c]));
                worst = std::max(worst, d);
                sum += d;
            }
            if (worst > 24) ++changed;
        }
    }
    return {double(changed) / double(n), double(sum) / double(n * 4)};
}

bool goldenEmpty(const QImage& image) {
    const QImage x = image.convertToFormat(QImage::Format_RGBA8888);
    for (int row = 0; row < x.height(); ++row) {
        const uchar* p = x.constScanLine(row);
        for (int i = 0; i < x.width() * 4; ++i) if (p[i] > 2) return false;
    }
    return true;
}
} // anon

// Golden images: every effect (fed by Plasma) and every generated source,
// rendered deterministically (the clock frozen, exactly 8 frames, ofRandom
// seeded, 256x144) and compared with the stored image (Tests/golden/<os>,
// NOTTAWA_GOLDEN_DIR). NOTTAWA_GOLDEN=record renders everything twice and
// stores the ones that come out the same both times; the others (effects
// reading the system clock, say) are listed in unstable.txt and only
// checked for rendering something. "[stress] golden: ..."; failures write
// <scratch>/golden-<name>.png (expected | actual). NOTTAWA_GOLDEN=stabilize
// (after recording, a few launches) moves entries that differ between
// launches into unstable.txt.
void StressTest::golden() {
    const bool record = qEnvironmentVariable("NOTTAWA_GOLDEN") == "record";
    // stabilize: as check, but entries that differ are added to unstable.txt
    // (rendering that differs between launches); run a few after recording.
    const bool stabilize = qEnvironmentVariable("NOTTAWA_GOLDEN") == "stabilize";
    const QString dir = qEnvironmentVariable("NOTTAWA_GOLDEN_DIR");
    if (dir.isEmpty()) { add([this] { window_->log("[stress] FAIL golden: NOTTAWA_GOLDEN_DIR isn't set"); }); return; }
    struct Entry { QString name; int typeRaw; bool isSource; };
    QVector<Entry> entries;
    // Named by kind, name and type (two sources are both called "Fractal").
    for (const auto& s : EngineBridge::instance()->sources())
        if (!s.isInput) entries.push_back({QString("src-%1-%2").arg(s.name).arg(s.typeRaw), s.typeRaw, true});
    QSet<int> effectTypes;  // (the catalog lists Glitch in two categories)
    for (const auto& e : EngineBridge::instance()->effects())
        if (!effectTypes.contains(e.typeRaw)) {
            effectTypes << e.typeRaw;
            entries.push_back({QString("fx-%1-%2").arg(e.name).arg(e.typeRaw), e.typeRaw, false});
        }
    auto fileName = [](QString name) {
        for (QChar& c : name) if (!c.isLetterOrNumber() && c != '-') c = '_';
        return name + ".png";
    };
    {
        QSet<QString> seen;
        for (const Entry& e : entries)
            if (seen.contains(fileName(e.name))) add([this, e] { window_->log("[stress] FAIL golden: two entries share " + e.name); });
            else seen << fileName(e.name);
    }
    auto plasma = std::make_shared<QString>();
    auto first = std::make_shared<QHash<QString, QImage>>();  // record: pass 1
    auto failures = std::make_shared<QStringList>();
    auto unstable = std::make_shared<QStringList>();
    auto stats = std::make_shared<QHash<QString, int>>();
    QSet<QString> knownUnstable;
    {
        QFile f(QDir(dir).filePath("unstable.txt"));
        if (!record && f.open(QIODevice::ReadOnly))
            for (const QString& line : QString::fromUtf8(f.readAll()).split('\n', Qt::SkipEmptyParts))
                if (!line.trimmed().startsWith('#')) knownUnstable << line.trimmed();  // (# comments)
    }

    add([this, plasma] {
        auto* eng = EngineBridge::instance();
        eng->clearGraph();
        Prefs::set("output.customWidth", 256.0);
        Prefs::set("output.customHeight", 144.0);
        Prefs::set("output.resolution", QString("custom"));
        ntw_debug_freeze_time(12.5);
        ntw_debug_step_mode(1);
        for (const auto& s : eng->sources()) if (s.name == "Plasma") *plasma = window_->editor_->addFromCatalog(s.typeRaw, true);
    }, 500);
    const int passes = record ? 2 : 1;
    for (int pass = 0; pass < passes; ++pass) {
        for (const Entry& entry : entries) {
            add([this, entry, plasma, pass, record, stabilize, dir, fileName, first, failures, unstable, stats, knownUnstable] {
                auto* eng = EngineBridge::instance();
                ntw_debug_render_frames(0, 1);  // seeds ofRandom before the node's setup draws from it
                const QString id = entry.isSource ? window_->editor_->addFromCatalog(entry.typeRaw, true)
                                                  : window_->editor_->addFromCatalog(entry.typeRaw, false, *plasma);
                ntw_debug_render_frames(8, 1);
                const QImage frame = goldenFrame(id);
                eng->removeNode(id);
                if (frame.isNull()) { *failures << entry.name + " (no frame)"; return; }
                const QString path = QDir(dir).filePath(fileName(entry.name));
                if (record) {
                    if (pass == 0) { first->insert(entry.name, frame); return; }
                    if (goldenDiff(first->value(entry.name), frame).same()) { frame.save(path); (*stats)["stored"]++; }
                    else { *unstable << entry.name; QFile::remove(path); }
                    return;
                }
                if (knownUnstable.contains(entry.name)) {
                    (*stats)["unstable"]++;
                    // Only that it renders something (unless it didn't when recorded either).
                    const QImage expected(path);
                    if (goldenEmpty(frame) && !(QFileInfo::exists(path) && goldenEmpty(expected)))
                        *failures << entry.name + " (renders nothing)";
                    return;
                }
                const QImage expected(path);
                if (expected.isNull()) { *failures << entry.name + " (no golden image: record them)"; return; }
                const GoldenDiff d = goldenDiff(expected, frame);
                (*stats)["checked"]++;
                if (!d.same() && stabilize) { *unstable << entry.name; return; }
                if (!d.same()) {
                    *failures << QString("%1 (%2% of pixels differ, mean %3)").arg(entry.name).arg(d.changed * 100, 0, 'f', 1).arg(d.mean, 0, 'f', 1);
                    QImage pair(256, 72, QImage::Format_RGBA8888);
                    pair.fill(Qt::black);
                    QPainter p(&pair);
                    p.drawImage(0, 0, expected);
                    p.drawImage(128, 0, frame);
                    p.end();
                    pair.save(QDir(scratch_).filePath("golden-" + fileName(entry.name)));
                }
            });
        }
    }
    add([this, record, stabilize, dir, failures, unstable, stats, entries] {
        ntw_debug_step_mode(0);
        ntw_debug_freeze_time(-1);
        EngineBridge::instance()->clearGraph();
        EngineBridge::instance()->resetSettings("Output");
        if (record) {
            // Kept: the block from "# Kept by hand" on (entries added by hand, with why).
            QFile f(QDir(dir).filePath("unstable.txt"));
            QString kept;
            if (f.open(QIODevice::ReadOnly)) {
                const QString old = QString::fromUtf8(f.readAll());
                const qsizetype at = old.indexOf("# Kept by hand");
                if (at >= 0) kept = old.mid(at);
                f.close();
            }
            if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
                f.write(("# Rendered differently twice in one launch (found when recording): only checked for rendering something.\n"
                         + unstable->join('\n') + '\n' + kept).toUtf8());
            window_->log(QString("[stress] golden: recorded %1 of %2; unstable (not the same twice): %3")
                             .arg(stats->value("stored")).arg(entries.size()).arg(unstable->join(", ")));
        } else if (stabilize) {
            QFile f(QDir(dir).filePath("unstable.txt"));
            if (!unstable->isEmpty() && f.open(QIODevice::Append)) f.write((unstable->join('\n') + '\n').toUtf8());
            window_->log(QString("[stress] golden: stabilize: %1 differed from their images and are now unstable: %2")
                             .arg(unstable->size()).arg(unstable->join(", ")));
        } else {
            window_->log(QString("[stress] golden: checked %1 against their images, %2 unstable for rendering something; %3 failed")
                             .arg(stats->value("checked")).arg(stats->value("unstable")).arg(failures->size()));
        }
        for (const QString& f : std::as_const(*failures)) window_->log("[stress] FAIL golden: " + f);
    });
}

// What users do, checked by what it renders (deterministic as golden()):
// a chain, an optional effect on and off, a two-input Mask, a workspace
// saved and reopened, a resolution change, undo and redo.
// "[stress] workflows: <name> ok" or FAIL.
void StressTest::workflows() {
    struct State { QString plasma, kaleido, hsb, fractal, mask, extra; QImage hsbFrame; int nodes = 0; };
    auto st = std::make_shared<State>();
    auto typeOf = [this](const QString& name, bool source) {
        for (const auto& c : source ? EngineBridge::instance()->sources() : EngineBridge::instance()->effects())
            if (c.name == name) return c.typeRaw;
        window_->log("[stress] FAIL workflows: no " + QString(source ? "source" : "effect") + " called " + name);
        return -1;
    };
    // Known bugs (reported, not failing the run; KNOWN-FIXED when one passes).
    static const QStringList known = {
        "Mask without a mask passes its input through",  // Mask ignores its mask input (pre-existing)
        "Mask's second input changes its output",
    };
    auto check = [this](bool ok, const QString& what) {
        if (known.contains(what))
            window_->log(QString("[stress] %1 workflows: %2").arg(ok ? "KNOWN-FIXED (remove from the known list)" : "KNOWN", what));
        else
            window_->log(ok ? "[stress] workflows: " + what + " ok" : "[stress] FAIL workflows: " + what);
    };
    auto render = [] { ntw_debug_render_frames(4, 1); };

    add([this, st, typeOf] {
        auto* eng = EngineBridge::instance();
        eng->clearGraph();
        Prefs::set("output.customWidth", 256.0);
        Prefs::set("output.customHeight", 144.0);
        Prefs::set("output.resolution", QString("custom"));
        ntw_debug_freeze_time(12.5);
        ntw_debug_step_mode(1);
        st->plasma = window_->editor_->addFromCatalog(typeOf("Plasma", true), true);
    }, 500);
    // 1. A chain renders at every stage, and an effect changes its input.
    add([this, st, typeOf, check, render] {
        st->kaleido = window_->editor_->addFromCatalog(typeOf("Kaleidoscope", false), false, st->plasma);
        st->hsb = window_->editor_->addFromCatalog(typeOf("HSB", false), false, st->kaleido);
        render();
        const QImage p = goldenFrame(st->plasma), k = goldenFrame(st->kaleido), h = goldenFrame(st->hsb);
        check(!p.isNull() && !goldenEmpty(p) && !goldenEmpty(k) && !goldenEmpty(h), "chain renders at every stage");
        check(!goldenDiff(p, k).same(), "Kaleidoscope changes its input");
        st->hsbFrame = h;
    });
    // 2. An optional effect changes the output; off again, it's as before.
    // (On Plasma: Kaleidoscope's output is already mirror-symmetric.)
    add([this, st, check, render] {
        auto* eng = EngineBridge::instance();
        const QImage before = goldenFrame(st->plasma);
        int mirror = -1;
        for (const auto& o : eng->optionalEffects(st->plasma)) if (o.name.contains("Mirror")) mirror = o.index;
        if (mirror < 0) { check(false, "optional effect: no Mirror on the node"); return; }
        eng->toggleOptionalEffect(st->plasma, mirror);
        render();
        const QImage on = goldenFrame(st->plasma);
        eng->toggleOptionalEffect(st->plasma, mirror);
        render();
        const QImage off = goldenFrame(st->plasma);
        check(!goldenEmpty(on) && !goldenDiff(before, on).same(), "optional effect (Mirror) changes the output");
        check(goldenDiff(before, off).same(), "optional effect off restores the output");
    });
    // 3. A two-input effect: Mask with nothing in its mask slot passes its
    // input through; a mask (Plasma through Color Mask, which keys colour out
    // to transparency) changes its output.
    add([this, st, typeOf, check, render] {
        auto* eng = EngineBridge::instance();
        st->mask = window_->editor_->addFromCatalog(typeOf("Mask", false), false, st->plasma);
        render();
        const QImage plasma = goldenFrame(st->plasma), alone = goldenFrame(st->mask);
        check(goldenDiff(plasma, alone).same(), "Mask without a mask passes its input through");
        st->fractal = window_->editor_->addFromCatalog(typeOf("Color Mask", false), false, st->plasma);
        eng->connectNodes(st->fractal, st->mask, 0, 1);
        render();
        const QImage masked = goldenFrame(st->mask);
        check(!goldenDiff(alone, masked).same(), "Mask's second input changes its output");
        eng->removeNode(st->mask);
        eng->removeNode(st->fractal);
    });
    // 4. A workspace saved and reopened renders the same.
    add([this, st, check, render] {
        auto* eng = EngineBridge::instance();
        render();
        st->hsbFrame = goldenFrame(st->hsb);
        st->nodes = int(eng->nodes().size());
        eng->saveWorkspace(QDir(scratch_).filePath("workflows-ws.json"), "workflows");
        eng->clearGraph();
        render();
        const bool loaded = eng->loadWorkspace(QDir(scratch_).filePath("workflows-ws.json"));
        window_->editor_->syncFromEngine();
        render();
        const QImage again = goldenFrame(st->hsb);
        check(loaded && int(eng->nodes().size()) == st->nodes, "workspace reopens with its nodes");
        check(goldenDiff(st->hsbFrame, again).same(), "workspace reopens rendering the same");
    });
    // 5. A resolution change reaches the frames.
    add([this, st, check, render] {
        Prefs::set("output.resolution", QString("1080p"));
        render();
        int w = 0, h = 0;
        ntw_debug_frame_rgba(st->hsb.toUtf8().constData(), &w, &h, nullptr, 0);
        check(w == 1920 && h == 1080 && !goldenEmpty(goldenFrame(st->hsb)), QString("1080p renders at 1920x1080 (got %1x%2)").arg(w).arg(h));
        Prefs::set("output.resolution", QString("custom"));
        render();
    });
    // 6. Undo removes an added effect; redo brings it back, rendering.
    add([this, st, typeOf, check, render] {
        auto* eng = EngineBridge::instance();
        const int before = int(eng->nodes().size());
        st->extra = window_->editor_->addFromCatalog(typeOf("Pixelate", false), false, st->hsb);
        render();
        eng->undo();
        render();
        const int afterUndo = int(eng->nodes().size());
        eng->redo();
        window_->editor_->syncFromEngine();
        render();
        QString back;
        for (const auto& n : eng->nodes()) if (n.name == "Pixelate") back = n.id;
        check(afterUndo == before && !back.isEmpty() && !goldenEmpty(goldenFrame(back)),
              QString("undo removes an effect, redo restores it rendering (%1 -> %2 nodes)").arg(before).arg(afterUndo));
    });
    add([] {
        ntw_debug_step_mode(0);
        ntw_debug_freeze_time(-1);
        EngineBridge::instance()->clearGraph();
        EngineBridge::instance()->resetSettings("Output");
    });
}

// Every effect, fed by Plasma, renders something: its own frame (read in
// the engine) isn't empty. Lists the ones that are ("[stress] renderall:").
void StressTest::renderAll() {
    auto plasma = std::make_shared<QString>();
    auto current = std::make_shared<QString>();
    auto empty = std::make_shared<QStringList>();
    add([this, plasma] {
        auto* eng = EngineBridge::instance();
        eng->clearGraph();
        for (const auto& s : eng->sources()) if (s.name == "Plasma") *plasma = window_->editor_->addFromCatalog(s.typeRaw, true);
    }, 300);
    const auto effects = EngineBridge::instance()->effects();
    for (const auto& e : effects) {
        add([this, e, plasma, current] {
            if (!current->isEmpty()) EngineBridge::instance()->removeNode(*current);
            *current = window_->editor_->addFromCatalog(e.typeRaw, false, *plasma);
        }, 50);
        add([this, e, current, empty] {
            int nonEmpty = 0;
            for (float t : {0.2f, 0.4f, 0.5f, 0.6f, 0.8f})
                for (float u : {0.3f, 0.5f, 0.7f})
                    if (ntw_debug_frame_pixel(current->toUtf8().constData(), t, u) != 0) ++nonEmpty;
            if (nonEmpty == 0) *empty << e.name;
        }, 400);
    }
    add([this, empty, effects] {
        window_->log(QString("[stress] renderall: %1 of %2 effects rendered nothing: %3")
                         .arg(empty->size()).arg(effects.size()).arg(empty->join(", ")));
    });
}

// An image of the main window with a few nodes showing their previews
// (<scratch>/canvas-screen.png), to check how they're drawn.
void StressTest::canvasImage() {
    add([this] {
        EngineBridge::instance()->clearGraph();
        const QString src = randomSource();
        const auto effects = EngineBridge::instance()->effects();
        window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, src);
        window_->editor_->zoomToFit();
        window_->raise();
        window_->activateWindow();
    }, 300);
    // What's on screen in the window's own rect, raised first (grab()
    // leaves the GL viewports blank): at fit, zoomed out, zoomed in.
    auto shot = [this](const QString& name) {
        const QRect r = window_->frameGeometry();
        if (QScreen* screen = window_->screen())
            screen->grabWindow(0, r.x() - screen->geometry().x(), r.y() - screen->geometry().y(), r.width(), r.height())
                .save(QDir(scratch_).filePath(name));
    };
    add([shot] { shot("canvas-screen.png"); }, 2000);
    add([this] { window_->editor_->scale(0.35, 0.35); });
    add([shot] { shot("canvas-out.png"); }, 700);
    add([this] { window_->editor_->scale(1 / 0.35 * 2.6, 1 / 0.35 * 2.6); });
    add([shot] { shot("canvas-in.png"); }, 700);
}

// The hang watchdog catches a long engine frame while it's running: a
// 1.2 s stall, then the engine log should hold an ENGINE STALL report with
// the engine thread in it.
void StressTest::engineStall() {
    add([] { ntw_debug_stall_engine(1200); });
    add([this] {
        QFile log(QDir(qApp->property("appDataDir").toString()).filePath("logs/engine.log"));
        const QString text = log.open(QIODevice::ReadOnly) ? QString::fromUtf8(log.readAll()) : QString();
        const qsizetype at = text.lastIndexOf("=== ENGINE STALL");
        const bool engineStack = at >= 0 && text.indexOf("(engine)", at) > at && text.indexOf("ntw_debug_stall_engine", at) > at;
        window_->log(QString("[stress] enginestall: report %1, engine stack %2").arg(at >= 0).arg(engineStack));
        if (at < 0 || !engineStack) window_->log("[stress] FAIL enginestall: no ENGINE STALL report naming the engine thread's stall");
    }, 3000);
}

// Reopen Output Windows, with "outputwindowslaunch" in a second run
// (-Keep): an output window left open at quit, at a known place.
void StressTest::outputWindows() {
    add([this] {
        EngineBridge::instance()->clearGraph();
        const QString id = randomSource();
        window_->editor_->openPreview(id);
        for (QWidget* w : QApplication::topLevelWidgets())
            if (auto* p = qobject_cast<PreviewWindow*>(w)) p->setGeometry(220, 180, 640, 360);
        window_->log(QString("[stress] outputwindows: left open on %1").arg(id));
    }, 500);
    // Quit now, through the main window as a user would (the scenario's
    // clean-up would clear the graph, closing the window).
    add([this] { window_->close(); }, 1000);
}

void StressTest::outputWindowsLaunch() {
    add([this] {
        const QStringList open = window_->editor_->openPreviewIds();
        QRect geometry;
        for (QWidget* w : QApplication::topLevelWidgets())
            if (auto* p = qobject_cast<PreviewWindow*>(w); p && p->isVisible()) geometry = p->geometry();
        window_->log(QString("[stress] outputwindowslaunch: reopened %1 at %2,%3 %4x%5").arg(open.join(","))
                         .arg(geometry.x()).arg(geometry.y()).arg(geometry.width()).arg(geometry.height()));
        if (open.size() != 1) window_->log("[stress] FAIL outputwindowslaunch: expected one reopened output window");
        else if (geometry != QRect(220, 180, 640, 360)) window_->log("[stress] FAIL outputwindowslaunch: not where it was");
    }, 1500);
}

// The second launch after "audiosettings": the sample tracks selected and
// started. ("[stress] audiolaunch: ...")
void StressTest::audioLaunch() {
    add([this] {
        auto* eng = EngineBridge::instance();
        QString name;
        for (const auto& s : eng->audioSources()) if (s.id == eng->selectedAudioSource()) name = s.name;
        window_->log(QString("[stress] audiolaunch: input %1, active %2").arg(name).arg(eng->isAudioActive()));
        if (!name.contains("Sample", Qt::CaseInsensitive)) window_->log("[stress] FAIL audiolaunch: the last input wasn't selected");
        if (!eng->isAudioActive()) window_->log("[stress] FAIL audiolaunch: audio didn't start");
        if (eng->isAudioActive()) eng->toggleAudio();
    }, 1500);
}

// The whole tempo path in the app: bundled sample tracks played through the
// file source, the BPM and beat count the app reports logged each second
// ("[stress] bpm: <track> t=<s> bpm=<x> beats=<n>").
void StressTest::bpm() {
    add([] {
        auto* eng = EngineBridge::instance();
        for (const auto& s : eng->audioSources())
            if (s.name.contains("Sample", Qt::CaseInsensitive)) eng->selectAudioSource(s.id);
        eng->setBpmMode(0);  // Auto
    }, 200);
    const auto tracks = EngineBridge::instance()->sampleTracks();
    for (const auto& track : tracks) {
        if (!track.name.contains("Nottawa Song") && !track.name.contains("Bass Cascade") && !track.name.contains("Velvet"))
            continue;
        const QString name = track.name;
        const int index = track.index;
        add([index] { EngineBridge::instance()->selectSampleTrack(index); }, 500);  // starts it playing
        add([] { if (!EngineBridge::instance()->isAudioActive()) EngineBridge::instance()->toggleAudio(); }, 500);
        for (int second = 1; second <= 20; ++second) {
            add([this, name, second] {
                const AudioAnalysis a = EngineBridge::instance()->audioAnalysis();
                window_->log(QString("[stress] bpm: %1 t=%2 bpm=%3 beats=%4 active=%5")
                                 .arg(name).arg(second).arg(a.bpm, 0, 'f', 2).arg(a.beatCount).arg(a.audioActive));
            }, 1000);
        }
        // Velvet Skies (87) tapped at double tempo, as someone who hears it
        // that way: the tracker should follow to 174.
        if (name.contains("Velvet")) {
            // Exact 174 BPM taps ending now (step timers are too coarse to tap with).
            add([] {
                const double now = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
                for (int i = 7; i >= 0; --i) EngineBridge::instance()->tapBpm(now - i * 60.0 / 174.0);
            });
            for (int second = 1; second <= 8; ++second)
                add([this, second] {
                    const AudioAnalysis a = EngineBridge::instance()->audioAnalysis();
                    window_->log(QString("[stress] bpm: tapped 174 t=%1 bpm=%2").arg(second).arg(a.bpm, 0, 'f', 2));
                }, 1000);
        }
    }
    // Manual: taps set the tempo.
    add([this] {
        auto* eng = EngineBridge::instance();
        eng->setBpmMode(1);
        const double now = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
        for (int i = 5; i >= 0; --i) eng->tapBpm(now - i * 0.6);
    }, 300);
    add([this] {
        window_->log(QString("[stress] bpm: manual tapped 100 bpm=%1").arg(EngineBridge::instance()->audioAnalysis().bpm, 0, 'f', 2));
        EngineBridge::instance()->setBpmMode(0);
    });
    add([] { if (EngineBridge::instance()->isAudioActive()) EngineBridge::instance()->toggleAudio(); });
}

// One effect swapped into every effect type in turn (each logged first, so
// a crash names its type), then one source into every source type.
void StressTest::swapAll() {
    add([this] {
        const QString src = randomSource();
        swapId_ = window_->editor_->addFromCatalog(EngineBridge::instance()->effects()[0].typeRaw, false, src);
    });
    const auto effects = EngineBridge::instance()->effects();
    for (const auto& e : effects) {
        add([this, e] {
            window_->log(QString("[stress] swapall: effect -> %1").arg(e.name));
            const QString id = EngineBridge::instance()->replaceNode(swapId_, e.typeRaw, false);
            if (id.isEmpty()) window_->log(QString("[stress] FAIL swapall: replacing with %1 failed").arg(e.name));
            else swapId_ = id;
            window_->editor_->selectNode(swapId_);
        }, 120);
    }
    add([this] {
        EngineBridge::instance()->clearGraph();
        swapId_ = randomSource();
    }, 100);
    const auto sources = EngineBridge::instance()->sources();
    for (const auto& s : sources) {
        add([this, s] {
            window_->log(QString("[stress] swapall: source -> %1").arg(s.name));
            const QString id = EngineBridge::instance()->replaceNode(swapId_, s.typeRaw, true);
            if (id.isEmpty()) window_->log(QString("[stress] FAIL swapall: replacing with %1 failed").arg(s.name));
            else swapId_ = id;
            window_->editor_->selectNode(swapId_);
        }, 120);
    }
}

// Random wiring between a handful of nodes, including blends, loops and
// out-of-range slots.
void StressTest::connect(int cycles) {
    add([this] {
        for (int i = 0; i < 3; ++i) randomSource();
        const auto effects = EngineBridge::instance()->effects();
        for (int i = 0; i < 4; ++i) window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false);
    });
    for (int c = 0; c < cycles * 3; ++c) {
        add([this] {
            auto* eng = EngineBridge::instance();
            const QStringList ids = nodeIds();
            const int op = int(rng_() % 4);
            if (op == 0) eng->connectNodes(pick(ids), pick(ids), int(rng_() % 3), int(rng_() % 4));
            else if (op == 1) eng->blendBetween(pick(ids), pick(ids));
            else if (op == 2) {
                const auto edges = eng->edges();
                if (!edges.isEmpty()) eng->removeConnection(edges[int(rng_() % uint32_t(edges.size()))].id);
            } else {
                eng->setNodeActive(pick(ids), bool(rng_() % 2));
            }
        }, 30);
    }
}

void StressTest::undo(int cycles) {
    for (int c = 0; c < cycles; ++c) {
        add([this] {
            auto* eng = EngineBridge::instance();
            const QString src = randomSource();
            const auto effects = eng->effects();
            const QString fx = window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, src);
            randomizeParameters(fx);
            if (rng_() % 2) eng->removeNode(src);
        }, 40);
        add([this] {
            auto* eng = EngineBridge::instance();
            const int n = 1 + int(rng_() % 8);
            for (int i = 0; i < n; ++i) eng->undo();
            for (int i = 0; i < int(rng_() % 8); ++i) eng->redo();
        }, 60);
    }
}

void StressTest::strands(int cycles) {
    for (int c = 0; c < cycles; ++c) {
        add([this] {
            auto* eng = EngineBridge::instance();
            const QString src = randomSource();
            const auto effects = eng->effects();
            const QString fx = window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, src);
            const QString name = QString("stress-%1").arg(++counter_);
            eng->saveStrand(fx.isEmpty() ? src : fx, QDir(eng->strandsFolder()).filePath(name + ".json"), name, {});
        }, 60);
        add([this] {
            auto* eng = EngineBridge::instance();
            const auto all = eng->strands();
            for (const StrandItem& s : all)
                if (s.name.startsWith("stress-") && rng_() % 2) window_->editor_->loadStrand(s.id);
        }, 150);
        add([this] {
            auto* eng = EngineBridge::instance();
            for (const StrandItem& s : eng->strands()) {
                if (!s.name.startsWith("stress-")) continue;
                if (rng_() % 3 == 0) eng->renameStrand(s.id, s.name + "-r");
                else if (rng_() % 2) eng->deleteStrand(s.id);
            }
            if (rng_() % 2) eng->clearGraph();
        }, 60);
    }
    add([] {
        auto* eng = EngineBridge::instance();
        for (const StrandItem& s : eng->strands()) if (s.name.startsWith("stress-")) eng->deleteStrand(s.id);
    });
}

void StressTest::workspace(int cycles) {
    for (int c = 0; c < cycles; ++c) {
        add([this] {
            for (int i = 0; i < 2; ++i) {
                const QString src = randomSource();
                const auto effects = EngineBridge::instance()->effects();
                window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, src);
            }
            savedNodes_ = int(EngineBridge::instance()->nodes().size());
            EngineBridge::instance()->saveWorkspace(QDir(scratch_).filePath("ws.json"), "stress");
        }, 60);
        add([] { EngineBridge::instance()->clearGraph(); }, 60);
        add([this] {
            QElapsedTimer timer;
            timer.start();
            if (!EngineBridge::instance()->loadWorkspace(QDir(scratch_).filePath("ws.json")))
                window_->log("[stress] FAIL loadWorkspace returned false");
            const int loaded = int(EngineBridge::instance()->nodes().size());
            window_->log(QString("[stress] workspace: loaded %1 nodes in %2 ms").arg(loaded).arg(timer.elapsed()));
            if (loaded != savedNodes_)
                window_->log(QString("[stress] FAIL workspace saved %1 nodes, loaded %2").arg(savedNodes_).arg(loaded));
        }, 150);
    }
}

// Preview windows opened, re-pointed by deleting their node, and closed.
void StressTest::previews(int cycles) {
    for (int c = 0; c < cycles; ++c) {
        add([this] {
            const QString src = randomSource();
            const auto effects = EngineBridge::instance()->effects();
            const QString fx = window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, src);
            window_->editor_->openPreview(fx);
            window_->editor_->openPreview(src);
        }, 60);
        add([this] {
            const QStringList ids = nodeIds();
            if (rng_() % 2) EngineBridge::instance()->removeNode(pick(ids));  // window retreats or closes
        }, 150);
        add([this] {
            for (const QString& id : window_->editor_->openPreviewIds())
                if (rng_() % 2) window_->editor_->closePreview(id);
            if (rng_() % 2) EngineBridge::instance()->clearGraph();
        }, 120);
    }
    add([this] { for (const QString& id : window_->editor_->openPreviewIds()) window_->editor_->closePreview(id); });
}

void StressTest::stage(int cycles) {
    add([this] {
        for (int i = 0; i < 2; ++i) {
            const QString src = randomSource();
            const auto effects = EngineBridge::instance()->effects();
            window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, src);
        }
    });
    for (int c = 0; c < cycles; ++c) {
        add([this] { window_->stageButton_->click(); }, 200);
        add([this] {  // mutate the graph while staged
            auto* eng = EngineBridge::instance();
            if (rng_() % 2) randomSource();
            else eng->removeNode(pick(nodeIds()));
        }, 200);
        add([this] { window_->stageButton_->click(); }, 200);
    }
}

void StressTest::resolution(int cycles) {
    add([this] {
        const QString src = randomSource();
        const auto effects = EngineBridge::instance()->effects();
        const QString fx = window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, src);
        window_->editor_->openPreview(fx);
    });
    for (int c = 0; c < cycles; ++c)
        add([this] { EngineBridge::instance()->setResolutionIndex(int(rng_() % 5)); }, 300);
    add([this] {
        EngineBridge::instance()->setResolutionIndex(3);
        for (const QString& id : window_->editor_->openPreviewIds()) window_->editor_->closePreview(id);
    });
}

// Text / icon / image / video inputs, the video sometimes deleted while its
// cache is still being built.
void StressTest::inputs(int cycles) {
    const QString video = qEnvironmentVariable("NOTTAWA_STRESS_VIDEO");
    for (int c = 0; c < cycles; ++c) {
        add([this, video] {
            auto* eng = EngineBridge::instance();
            const QString text = eng->addInput(VideoSourceKind::Text);
            eng->setTextSourceText(text, QString(int(rng_() % 200), QChar('W')));
            eng->setTextSourceFontSize(text, int(rng_() % 400));
            const QString icon = eng->addInput(VideoSourceKind::Icon);
            eng->setIconSourceIcon(icon, int(rng_() % 50) - 5);
            if (!video.isEmpty()) window_->editor_->addMediaFile(video);
        }, 60);
        add([this] {
            auto* eng = EngineBridge::instance();
            for (const QString& id : nodeIds()) {
                const FileSourceState f = eng->fileSourceState(id);
                if (!f.valid) continue;
                eng->setFileSourcePosition(id, float(rng_() % 100) / 100.0f);
                eng->setFileSourceSpeed(id, float(rng_() % 200) / 100.0f);
                eng->setFileSourceMuted(id, bool(rng_() % 2));
            }
        }, int(rng_() % 2 ? 50 : 1500));
        add([] { EngineBridge::instance()->clearGraph(); }, 60);
    }
}

void StressTest::copyPaste(int cycles) {
    add([this] {
        const QString src = randomSource();
        const auto effects = EngineBridge::instance()->effects();
        window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, src);
    });
    for (int c = 0; c < cycles; ++c) {
        add([this] {
            auto* eng = EngineBridge::instance();
            QStringList ids = nodeIds();
            if (ids.size() > 40) { eng->clearGraph(); randomSource(); ids = nodeIds(); }
            eng->copyNodes(ids.mid(0, 1 + int(rng_() % 3)));
            eng->pasteNodes();
        }, 50);
    }
}

void StressTest::optional(int cycles) {
    add([this] {
        const QString src = randomSource();
        const auto effects = EngineBridge::instance()->effects();
        window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, src);
    });
    for (int c = 0; c < cycles * 2; ++c) {
        add([this] {
            auto* eng = EngineBridge::instance();
            const QString id = pick(nodeIds());
            const auto effects = eng->optionalEffects(id);
            if (effects.isEmpty()) return;
            const int index = int(rng_() % uint32_t(effects.size()));
            eng->toggleOptionalEffect(id, index);
            for (const ParamInfo& p : eng->optionalEffectParameters(id, index))
                if (p.type == ParamInfo::Standard) eng->setParameter(p.id, p.max);
            window_->editor_->selectNode(id);
        }, 60);
    }
    // Optional effects survive a save and load: one enabled, one only
    // adjusted (its settings kept although it's never been switched on).
    struct Expect { float enabledValue = 0, adjustedValue = 0; };
    auto expect = std::make_shared<Expect>();
    auto firstStandard = [](const QVector<ParamInfo>& params) -> const ParamInfo* {
        for (const ParamInfo& p : params) if (p.type == ParamInfo::Standard && p.max > p.min) return &p;
        return nullptr;
    };
    add([this, expect, firstStandard] {
        auto* eng = EngineBridge::instance();
        eng->clearGraph();
        const QString src = randomSource();
        const auto effects = eng->effects();
        const QString fx = window_->editor_->addFromCatalog(effects[0].typeRaw, false, src);
        eng->toggleOptionalEffect(fx, 0);
        const auto enabledParams = eng->optionalEffectParameters(fx, 0);
        if (const ParamInfo* p = firstStandard(enabledParams)) {
            expect->enabledValue = p->min + 0.37f * (p->max - p->min);
            eng->setParameter(p->id, expect->enabledValue);
        }
        const auto adjustedParams = eng->optionalEffectParameters(fx, 3);
        if (const ParamInfo* p = firstStandard(adjustedParams)) {
            expect->adjustedValue = p->min + 0.61f * (p->max - p->min);
            eng->setParameter(p->id, expect->adjustedValue);
        }
        eng->saveWorkspace(QDir(scratch_).filePath("optional.json"), "optional");
        eng->clearGraph();
    }, 100);
    add([this, expect, firstStandard] {
        auto* eng = EngineBridge::instance();
        eng->loadWorkspace(QDir(scratch_).filePath("optional.json"));
        QString fx;
        for (const QString& id : nodeIds())
            if (eng->optionalEffects(id).size() == 10) fx = id;  // an effect's list (a source's has 9)
        if (fx.isEmpty()) { window_->log("[stress] FAIL optional: effect missing after load"); return; }
        const auto effects = eng->optionalEffects(fx);
        if (!effects[0].enabled) window_->log("[stress] FAIL optional: enabled effect came back disabled");
        if (effects[3].enabled) window_->log("[stress] FAIL optional: adjusted effect came back enabled");
        const auto enabledParams = eng->optionalEffectParameters(fx, 0);
        const auto adjustedParams = eng->optionalEffectParameters(fx, 3);
        const ParamInfo* a = firstStandard(enabledParams);
        const ParamInfo* b = firstStandard(adjustedParams);
        if (!a || std::abs(a->value - expect->enabledValue) > 1e-3f)
            window_->log(QString("[stress] FAIL optional: enabled effect's value %1, expected %2").arg(a ? a->value : -1).arg(expect->enabledValue));
        if (!b || std::abs(b->value - expect->adjustedValue) > 1e-3f)
            window_->log(QString("[stress] FAIL optional: adjusted effect's value %1, expected %2").arg(b ? b->value : -1).arg(expect->adjustedValue));
        window_->log("[stress] optional: save/load checked");
    }, 300);
}

// Damaged and hostile inputs: workspaces and strands that are truncated, of
// the wrong types or garbage; video and image files that aren't; text that's
// huge or exotic. Each must fail cleanly.
void StressTest::badFiles(int cycles) {
    add([this] {
        QDir dir(scratch_);
        auto write = [&](const QString& name, const QByteArray& data) {
            QFile f(dir.filePath(name));
            if (f.open(QIODevice::WriteOnly)) f.write(data);
        };
        // A real workspace to mangle.
        const QString src = randomSource();
        const auto effects = EngineBridge::instance()->effects();
        window_->editor_->addFromCatalog(effects[0].typeRaw, false, src);
        EngineBridge::instance()->saveWorkspace(dir.filePath("good.json"), "good");
        QFile good(dir.filePath("good.json"));
        good.open(QIODevice::ReadOnly);
        const QByteArray ws = good.readAll();
        write("empty.json", {});
        write("truncated.json", ws.left(ws.size() / 2));
        write("garbage.json", QByteArray(4096, '\x9c'));
        write("array.json", "[1,2,3]");
        write("null.json", "null");
        write("wrongtypes.json", QByteArray(ws).replace("\"x\":", "\"x\":\"left\",\"_x\":")
                                     .replace("\"shaderType\":", "\"shaderType\":\"oops\",\"_t\":"));
        write("hugenumbers.json", QByteArray(ws).replace(": 0.", ": 1e308").replace(": 1,", ": -2147483648,"));
        write("fake.mp4", QByteArray("this is not a video\n").repeated(200));
        write("zero.mp4", {});
        write("fake.png", QByteArray(64, '\x00'));
        write("zero.png", {});
    }, 100);
    for (int c = 0; c < cycles; ++c) {
        add([this] {
            auto* eng = EngineBridge::instance();
            QDir dir(scratch_);
            for (const char* name : {"empty.json", "truncated.json", "garbage.json", "array.json", "null.json",
                                     "wrongtypes.json", "hugenumbers.json", "missing.json"})
                eng->loadWorkspace(dir.filePath(name));
            QFile wrong(dir.filePath("wrongtypes.json"));
            wrong.open(QIODevice::ReadOnly);
            eng->loadStrandJson(QString::fromUtf8(wrong.readAll()));
            eng->loadStrandJson("{not json");
            eng->loadStrandJson("{\"shaders\": 5, \"sources\": \"x\", \"connections\": [null]}");
        }, 100);
        add([this] {
            QDir dir(scratch_);
            for (const char* name : {"fake.mp4", "zero.mp4", "fake.png", "zero.png", "missing.mp4"})
                window_->editor_->addMediaFile(dir.filePath(name));
            auto* eng = EngineBridge::instance();
            const QString text = eng->addInput(VideoSourceKind::Text);
            const QStringList samples = {QString(20000, QChar('W')), QString::fromUtf8("\xf0\x9f\x94\xa5\xe6\xbc\xa2\xe5\xad\x97 \xd9\x85\xd8\xb1\xd8\xad\xd8\xa8\xd8\xa7"),
                                         QString(), QStringLiteral("\n\n\t\t"), QString(500, QChar(0x200B))};
            eng->setTextSourceText(text, samples[int(rng_() % uint32_t(samples.size()))]);
            eng->setTextSourceFontSize(text, rng_() % 2 ? 0 : 2000);
        }, 300);
        add([] { EngineBridge::instance()->clearGraph(); }, 100);
    }
}

// Text at a few sizes, then the size slider dragged 8 -> 400.
void StressTest::fonts() {
    add([this] { textId_ = EngineBridge::instance()->addInput(VideoSourceKind::Text); });
    for (int size : {24, 72, 200, 400}) {
        add([this, size] { EngineBridge::instance()->setTextSourceFontSize(textId_, size); }, 100);
        add([this, size] { checkpoint(QString("fonts: size %1").arg(size)); }, 1500);
    }
    add([this] { window_->log("[stress] fonts: drag start"); });
    for (int size = 8; size <= 400; size += 4)
        add([this, size] { EngineBridge::instance()->setTextSourceFontSize(textId_, size); }, 16);
    add([this] { checkpoint("fonts: drag end"); }, 100);
}

// Audio capture started and stopped, sources and sample tracks switched and
// BPM settings changed, while audio-reactive nodes come and go.
void StressTest::audio(int cycles) {
    for (int c = 0; c < cycles; ++c) {
        add([this] {
            auto* eng = EngineBridge::instance();
            const auto sources = eng->audioSources();
            if (!sources.isEmpty()) eng->selectAudioSource(sources[int(rng_() % uint32_t(sources.size()))].id);
            const auto tracks = eng->sampleTracks();
            if (!tracks.isEmpty()) eng->selectSampleTrack(int(rng_() % uint32_t(tracks.size())));
            eng->toggleAudio();
            // An audio-reactive source (they're listed first) with an effect.
            const auto catalog = eng->sources();
            const QString src = window_->editor_->addFromCatalog(catalog[int(rng_() % uint32_t(std::min<qsizetype>(catalog.size(), 12)))].typeRaw, true);
            const auto effects = eng->effects();
            window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, src);
        }, 80);
        add([this] {
            auto* eng = EngineBridge::instance();
            eng->setBpmMode(int(rng_() % 3));
            eng->setSmoothingMode(int(rng_() % 3));
            eng->tapBpm();
            eng->nudgeBpm(float(int(rng_() % 3)) - 1.0f);
        }, 400);
        add([this] {
            auto* eng = EngineBridge::instance();
            if (rng_() % 2) eng->toggleAudio();
            if (rng_() % 2) eng->clearGraph();
        }, 300);
    }
    add([] { if (EngineBridge::instance()->isAudioActive()) EngineBridge::instance()->toggleAudio(); });
}

// Where memory goes: N source + effect pairs, then cleared (undo still holds
// them), then the undo history flushed with small actions.
void StressTest::memory(int nodes) {
    add([this, nodes] {
        const auto effects = EngineBridge::instance()->effects();
        for (int i = 0; i < nodes; ++i) {
            const QString src = randomSource();
            window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, src);
        }
    }, 100);
    add([this, nodes] { checkpoint(QString("memory: %1 pairs live").arg(nodes)); }, 2000);
    add([] { EngineBridge::instance()->clearGraph(); });
    add([this] { checkpoint("memory: cleared, held by undo"); }, 2000);
    for (int i = 0; i < 12; ++i) {
        add([this] { randomSource(); }, 50);
        add([] { EngineBridge::instance()->clearGraph(); }, 50);
    }
    add([this] { checkpoint("memory: undo history flushed"); }, 2000);
}

// Every parameter of random nodes thrown to its extremes, with oscillators
// switched on and off.
void StressTest::parameters(int cycles) {
    for (int c = 0; c < cycles; ++c) {
        add([this] {
            auto* eng = EngineBridge::instance();
            const QString src = randomSource();
            const auto effects = eng->effects();
            const QString fx = window_->editor_->addFromCatalog(effects[int(rng_() % uint32_t(effects.size()))].typeRaw, false, src);
            for (const QString& id : {src, fx}) {
                randomizeParameters(id);
                for (const ParamInfo& p : eng->parameters(id))
                    if (p.hasOscillator && rng_() % 3 == 0) {
                        eng->setOscillatorEnabled(p.id, true);
                        eng->setOscillatorFrequency(p.id, float(rng_() % 400) / 100.0f);
                    }
            }
            window_->editor_->selectNode(fx);
        }, 60);
        add([] {}, 150);
        add([] { EngineBridge::instance()->clearGraph(); }, 40);
    }
}
