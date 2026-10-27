#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
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
