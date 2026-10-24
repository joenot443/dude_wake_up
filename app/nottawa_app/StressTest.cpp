#include "StressTest.h"

#include "EngineBridge.h"
#include "MainWindow.h"
#include "nodes/NodeEditorView.h"
#include "nottawa_bridge.h"
#include "panels/StageView.h"

#include <QApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QToolButton>

#include <windows.h>
#include <psapi.h>

namespace {
const QStringList kAllScenarios = {"churn", "swap", "connect", "undo", "strands", "workspace", "previews",
                                   "stage", "resolution", "inputs", "copypaste", "optional", "parameters"};

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
            EngineBridge::instance()->saveWorkspace(QDir(scratch_).filePath("ws.json"), "stress");
        }, 60);
        add([] { EngineBridge::instance()->clearGraph(); }, 60);
        add([this] {
            if (!EngineBridge::instance()->loadWorkspace(QDir(scratch_).filePath("ws.json")))
                window_->log("[stress] FAIL loadWorkspace returned false");
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
