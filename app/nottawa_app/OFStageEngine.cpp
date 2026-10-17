// ofMain must come before anything that includes <GL/gl.h> (windows.h, Qt):
// it pulls in GLEW, which refuses to compile after gl.h.
#include "ofMain.h"
#include "ofAppGLFWWindow.h"

#include <windows.h>
#include <timeapi.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

#include "OFStageEngine.h"
#include "StageWidget.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <algorithm>
#include <vector>

namespace of_impl {

// Lets us hand a raw GLFWwindow to ofGLFWWindowSettings::shareContextWith,
// which OF only reads through getWindowContext().
class BareShareHandle : public ofAppBaseWindow {
public:
    explicit BareShareHandle(GLFWwindow* w) : raw_(w) {}
    void* getWindowContext() override { return raw_; }
    void  setup(const ofWindowSettings&) override {}
    void  update() override {}
    void  draw() override {}
    ofCoreEvents& events() override { return events_; }
    std::shared_ptr<ofBaseRenderer>& renderer() override { return renderer_; }
private:
    GLFWwindow*                     raw_;
    ofCoreEvents                    events_;
    std::shared_ptr<ofBaseRenderer> renderer_;
};

struct State {
    static constexpr int W = 1920;
    static constexpr int H = 1080;
    static constexpr int kQueries = 3;

    GLFWwindow* shareRoot = nullptr;
    std::shared_ptr<BareShareHandle> shim;
    std::shared_ptr<ofAppBaseWindow> ofWindow;

    ofFbo     fbos[FrameExchange::kSlots];
    ofShader  shader;
    ofTexture inputTex;
    QString   shaderName;

    GLuint queries[kQueries] = {};
    bool   queryPending[kQueries] = {};
    int    queryNext = 0;
    std::vector<double> gpuMs;   // samples for the current stats window
};

} // namespace of_impl

namespace {

QString& shadersDirRef() { static QString s; return s; }

// ofShader logs compile errors to OF's console channel, which a GUI-
// subsystem app doesn't have; recompile standalone to get the info log.
QString compileLog(GLenum type, const std::string& src) {
    GLuint s = glCreateShader(type);
    const char* p = src.c_str();
    glShaderSource(s, 1, &p, nullptr);
    glCompileShader(s);
    GLint len = 0;
    glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
    std::string log(std::max(len, 1), '\0');
    glGetShaderInfoLog(s, len, nullptr, log.data());
    glDeleteShader(s);
    return QString::fromStdString(log).trimmed();
}

bool loadNottawaShader(ofShader& target, const QString& name, QString* err) {
    const QString base = shadersDirRef() + "/" + name;
    ofBuffer vert = ofBufferFromFile((base + ".vert").toStdString(), false);
    ofBuffer frag = ofBufferFromFile((base + ".frag").toStdString(), false);
    if (vert.size() == 0 || frag.size() == 0) {
        *err = "missing source for " + name;
        return false;
    }
    ofShader fresh;
    if (!fresh.setupShaderFromSource(GL_VERTEX_SHADER, vert.getText())) {
        *err = "vert compile failed: " + name + "\n" + compileLog(GL_VERTEX_SHADER, vert.getText());
        return false;
    }
    if (!fresh.setupShaderFromSource(GL_FRAGMENT_SHADER, frag.getText())) {
        *err = "frag compile failed: " + name + "\n" + compileLog(GL_FRAGMENT_SHADER, frag.getText());
        return false;
    }
    fresh.bindDefaults();  // must precede linkProgram
    if (!fresh.linkProgram()) { *err = "link failed: " + name; return false; }
    target = std::move(fresh);
    return true;
}

double percentile(std::vector<double>& v, int pct) {
    if (v.empty()) return 0;
    std::sort(v.begin(), v.end());
    return v[std::min(v.size() - 1, v.size() * pct / 100)];
}

bool envFlag(const char* name) { return qEnvironmentVariableIntValue(name) != 0; }

} // anon

void setNottawaShadersDir(const QString& absPath) { shadersDirRef() = absPath; }
QString nottawaShadersDir() { return shadersDirRef(); }

QStringList nottawaShaderNames() {
    QStringList names;
    const QDir dir(shadersDirRef());
    if (shadersDirRef().isEmpty()) return names;
    for (const QString& frag : dir.entryList({"*.frag"}, QDir::Files, QDir::Name)) {
        const QString base = frag.chopped(5);
        if (QFileInfo::exists(dir.absoluteFilePath(base + ".vert"))) names << base;
    }
    return names;
}

QString nottawaDefaultShader() {
    const QString env = qEnvironmentVariable("NOTTAWA_SHADER");
    return env.isEmpty() ? QStringLiteral("Accretion") : env;
}

OFStageEngine::OFStageEngine(StageWidget* widget, void* qtHglrc)
  : widget_(widget), qtHglrc_(qtHglrc),
    uncapped_(envFlag("NOTTAWA_UNCAPPED")), benchAll_(envFlag("NOTTAWA_BENCH_ALL")) {}

OFStageEngine::~OFStageEngine() {
    requestStop();
    wait();
}

void OFStageEngine::requestStop() { stop_.store(true); }

EngineStats OFStageEngine::stats() const {
    QMutexLocker lk(&statsMutex_);
    return stats_;
}

void OFStageEngine::requestShader(const QString& name) {
    QMutexLocker lk(&pendingMutex_);
    pendingShader_ = name;
    pendingSet_ = true;
}

void OFStageEngine::setBrightness(double value) { brightness_.store(value); }

bool OFStageEngine::createShareRoot() {
    if (!glfwInit()) { emit engineError("glfwInit failed"); return false; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    of_->shareRoot = glfwCreateWindow(16, 16, "nottawa-share-root", nullptr, nullptr);
    if (!of_->shareRoot) { emit engineError("share-root glfwCreateWindow failed"); return false; }
    return true;
}

bool OFStageEngine::shareWithQt() {
    HGLRC root = glfwGetWGLContext(of_->shareRoot);
    HGLRC qt   = static_cast<HGLRC>(qtHglrc_);
    if (!root || !qt) { emit engineError("missing HGLRC for context sharing"); return false; }
    if (!wglShareLists(qt, root)) {
        emit engineError(QString("wglShareLists failed (0x%1)").arg(GetLastError(), 8, 16, QChar('0')));
        return false;
    }
    return true;
}

bool OFStageEngine::initOF() {
    of_->shim = std::make_shared<of_impl::BareShareHandle>(of_->shareRoot);

    ofGLFWWindowSettings settings;
    settings.setGLVersion(3, 3);
    settings.setSize(64, 64);
    settings.visible = false;
    settings.decorated = false;
    settings.resizable = false;
    settings.shareContextWith = of_->shim;
    of_->ofWindow = ofCreateWindow(settings);
    auto* glfwWin = dynamic_cast<ofAppGLFWWindow*>(of_->ofWindow.get());
    if (!glfwWin || !glfwWin->getGLFWWindow()) { emit engineError("ofCreateWindow failed"); return false; }
    glfwHideWindow(glfwWin->getGLFWWindow());
    glfwSetWindowRefreshCallback(glfwWin->getGLFWWindow(), nullptr);

    ofDisableArbTex();

    ofFboSettings fs;
    fs.width = of_impl::State::W;
    fs.height = of_impl::State::H;
    fs.internalformat = GL_RGBA8;
    fs.textureTarget = GL_TEXTURE_2D;
    uint32_t ids[FrameExchange::kSlots];
    for (int i = 0; i < FrameExchange::kSlots; ++i) {
        of_->fbos[i].allocate(fs);
        if (!of_->fbos[i].isAllocated()) { emit engineError("ofFbo::allocate failed"); return false; }
        ids[i] = of_->fbos[i].getTexture().getTextureData().textureID;
    }
    widget_->exchange().setTextures(ids);

    // Stand-in for the `tex` input until real sources are wired.
    unsigned char white[4] = {255, 255, 255, 255};
    of_->inputTex.allocate(1, 1, GL_RGBA);
    of_->inputTex.loadData(white, 1, 1, GL_RGBA);

    glGenQueries(of_impl::State::kQueries, of_->queries);

    const QString first = benchAll_ ? nottawaShaderNames().value(0) : nottawaDefaultShader();
    if (!switchShader(first)) return false;

    emit engineInfo(QString("OF ready — %1, %2x%3 triple-buffered, shader %4%5")
                    .arg(reinterpret_cast<const char*>(glGetString(GL_RENDERER)))
                    .arg(of_impl::State::W).arg(of_impl::State::H)
                    .arg(of_->shaderName)
                    .arg(uncapped_ ? ", uncapped" : ""));
    return true;
}

bool OFStageEngine::switchShader(const QString& name) {
    QString err;
    if (!loadNottawaShader(of_->shader, name, &err)) {
        emit engineError(err);
        return false;
    }
    of_->shaderName = name;
    return true;
}

void OFStageEngine::applyPendingShader() {
    QString name;
    {
        QMutexLocker lk(&pendingMutex_);
        if (!pendingSet_) return;
        name = pendingShader_;
        pendingSet_ = false;
    }
    if (name != of_->shaderName && switchShader(name))
        emit engineInfo("shader: " + name);
}

void OFStageEngine::renderFrame(float t) {
    FrameExchange& ex = widget_->exchange();
    void* readFence = nullptr;
    const int slot = ex.acquireWriteSlot(&readFence);
    if (readFence) {
        glWaitSync(static_cast<GLsync>(readFence), 0, GL_TIMEOUT_IGNORED);
        glDeleteSync(static_cast<GLsync>(readFence));
    }

    const int q = of_->queryNext;
    glBeginQuery(GL_TIME_ELAPSED, of_->queries[q]);

    ofFbo& fbo = of_->fbos[slot];
    fbo.begin();
    ofClear(0, 0, 0, 255);
    of_->shader.begin();
    of_->shader.setUniform1f("time", t);
    of_->shader.setUniform2f("dimensions", of_impl::State::W, of_impl::State::H);
    of_->shader.setUniform1f("brightness", static_cast<float>(brightness_.load()));
    of_->shader.setUniformTexture("tex", of_->inputTex, 0);
    // Same pattern as Nottawa's Shader::shade(): drawing a texture through
    // the bound shader gives a fullscreen quad with OF's MVP and attributes.
    of_->inputTex.draw(0, 0, of_impl::State::W, of_impl::State::H);
    of_->shader.end();
    fbo.end();

    glEndQuery(GL_TIME_ELAPSED);
    of_->queryPending[q] = true;
    of_->queryNext = (q + 1) % of_impl::State::kQueries;

    GLsync writeFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glFlush();
    if (void* stale = ex.publish(slot, writeFence)) glDeleteSync(static_cast<GLsync>(stale));
    widget_->notifyFrameReady();
}

// Reads the oldest in-flight timer query without stalling; a sample that
// isn't ready by the time its query object is reused is dropped.
void OFStageEngine::collectGpuTime() {
    const int q = of_->queryNext;
    if (!of_->queryPending[q]) return;
    of_->queryPending[q] = false;
    GLuint available = 0;
    glGetQueryObjectuiv(of_->queries[q], GL_QUERY_RESULT_AVAILABLE, &available);
    if (!available) return;
    GLuint64 ns = 0;
    glGetQueryObjectui64v(of_->queries[q], GL_QUERY_RESULT, &ns);
    of_->gpuMs.push_back(ns / 1e6);
}

void OFStageEngine::teardown() {
    if (!of_) return;
    if (of_->queries[0]) glDeleteQueries(of_impl::State::kQueries, of_->queries);
    of_->shader.unload();
    for (auto& f : of_->fbos) f.clear();
    of_->inputTex.clear();
    of_->ofWindow.reset();
    if (of_->shareRoot) glfwDestroyWindow(of_->shareRoot);
    of_.reset();
}

void OFStageEngine::run() {
    of_ = std::make_unique<of_impl::State>();
    if (!createShareRoot() || !shareWithQt() || !initOF()) { teardown(); return; }

    timeBeginPeriod(1);  // 1 ms Sleep granularity for frame pacing

    constexpr qint64 kPeriodNs = 1'000'000'000LL / 60;
    constexpr double kHitchMs  = 1000.0 / 30.0;

    QElapsedTimer clock; clock.start();
    const qint64 t0 = clock.nsecsElapsed();
    qint64 deadline = t0, lastStart = -1, windowStart = t0;
    quint64 framesTotal = 0, hitches = 0, windowFrames = 0;
    std::vector<double> intervals;

    const QStringList benchNames = benchAll_ ? nottawaShaderNames() : QStringList();
    int benchIdx = 0, benchFrames = 0;
    std::vector<double> benchGpu;
    struct BenchResult { QString name; double avg, max; };
    std::vector<BenchResult> benchResults;
    int benchFailures = 0;

    while (!stop_.load()) {
        const qint64 start = clock.nsecsElapsed();
        if (lastStart >= 0) {
            const double ms = (start - lastStart) / 1e6;
            intervals.push_back(ms);
            if (ms > kHitchMs) ++hitches;
        }
        lastStart = start;

        if (!benchAll_) applyPendingShader();
        renderFrame(static_cast<float>((start - t0) / 1e9));
        const size_t gpuBefore = of_->gpuMs.size();
        collectGpuTime();
        ++framesTotal; ++windowFrames;

        if (benchAll_) {
            // Skip the first frames after a switch so compile cost and the
            // previous shader's in-flight queries don't count.
            if (++benchFrames > 15)
                benchGpu.insert(benchGpu.end(), of_->gpuMs.begin() + gpuBefore, of_->gpuMs.end());
            if (benchFrames >= 90) {
                double sum = 0, mx = 0;
                for (double v : benchGpu) { sum += v; mx = std::max(mx, v); }
                const double avg = benchGpu.empty() ? 0 : sum / benchGpu.size();
                benchResults.push_back({of_->shaderName, avg, mx});
                emit engineInfo(QString("bench %1 gpu avg %2 ms max %3 ms")
                                .arg(of_->shaderName).arg(avg, 0, 'f', 3).arg(mx, 0, 'f', 3));
                benchGpu.clear();
                benchFrames = 0;
                // Advance to the next shader that compiles.
                while (++benchIdx < benchNames.size() && !switchShader(benchNames[benchIdx]))
                    ++benchFailures;
                if (benchIdx >= benchNames.size()) {
                    std::sort(benchResults.begin(), benchResults.end(),
                              [](const BenchResult& a, const BenchResult& b) { return a.avg > b.avg; });
                    emit engineInfo(QString("bench done: %1 ok, %2 failed to compile")
                                    .arg(benchResults.size()).arg(benchFailures));
                    for (size_t i = 0; i < std::min<size_t>(10, benchResults.size()); ++i)
                        emit engineInfo(QString("bench heaviest #%1 %2 avg %3 ms max %4 ms")
                                        .arg(i + 1).arg(benchResults[i].name)
                                        .arg(benchResults[i].avg, 0, 'f', 3)
                                        .arg(benchResults[i].max, 0, 'f', 3));
                    emit benchFinished();
                    break;
                }
            }
        }

        if (start - windowStart >= 1'000'000'000LL) {
            EngineStats s;
            s.fps           = windowFrames / ((start - windowStart) / 1e9);
            s.intervalMaxMs = intervals.empty() ? 0 : *std::max_element(intervals.begin(), intervals.end());
            s.intervalP99Ms = percentile(intervals, 99);
            double gpuSum = 0; for (double v : of_->gpuMs) gpuSum += v;
            s.gpuAvgMs      = of_->gpuMs.empty() ? 0 : gpuSum / of_->gpuMs.size();
            s.gpuP99Ms      = percentile(of_->gpuMs, 99);
            s.hitches       = hitches;
            s.frames        = framesTotal;
            {
                QMutexLocker lk(&statsMutex_);
                stats_ = s;
            }
            intervals.clear();
            of_->gpuMs.clear();
            windowFrames = 0;
            windowStart = start;
        }

        if (!uncapped_) {
            deadline += kPeriodNs;
            const qint64 now = clock.nsecsElapsed();
            if (now - deadline > kPeriodNs) deadline = now;  // fell behind: resync, don't burst
            const qint64 sleepMs = (deadline - now) / 1'000'000;
            if (sleepMs > 0) msleep(static_cast<unsigned long>(sleepMs));
        }
    }

    timeEndPeriod(1);
    teardown();
}
