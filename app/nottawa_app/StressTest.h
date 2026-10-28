#pragma once

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <deque>
#include <functional>
#include <random>

class MainWindow;
class QUrl;

// Swallows QDesktopServices URL opens in test runs (NOTTAWA_NO_PUBLISH), so
// random clicking can't launch a browser, mail client or editor.
class UrlSink : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
public slots:
    void open(const QUrl& url);
};

// Automated stress runs for finding crashes and leaks: NOTTAWA_STRESS lists
// scenarios (comma separated; "all" for every one), NOTTAWA_STRESS_ROUNDS how
// many times to repeat the list, NOTTAWA_STRESS_SEED the random seed. Each
// scenario drives the same editor and engine calls the UI makes, a step at a
// time on a timer so the app keeps rendering; between scenarios the graph is
// cleared and memory, handles, GUI objects and the engine's live object
// counts are logged ("[stress]" lines). The app quits when done.
class StressTest : public QObject {
    Q_OBJECT
public:
    explicit StressTest(MainWindow* window);
    static bool requested();
    void start();

private:
    using Step = std::function<void()>;
    void add(Step step, int delayMs = 15) { steps_.push_back({std::move(step), delayMs}); }
    void next();
    void enqueueScenario(const QString& name);
    void checkpoint(const QString& label);

    // Scenario builders: each appends its steps.
    void churn(int cycles);
    void swap(int cycles);
    void connect(int cycles);
    void undo(int cycles);
    void strands(int cycles);
    void workspace(int cycles);
    void previews(int cycles);
    void stage(int cycles);
    void resolution(int cycles);
    void inputs(int cycles);
    void copyPaste(int cycles);
    void optional(int cycles);
    void parameters(int cycles);
    void memory(int nodes);
    void audio(int cycles);
    void badFiles(int cycles);
    void fonts();
    void swapAll();
    void bpm();
    void settings();
    void settingsUi();
    void audioSettings();
    void audioLaunch();
    void cacheSettings();
    void beatTiming();
    void outputWindows();
    void saveTiming();
    void engineStall();
    void canvasImage();
    void blendDrag();
    void renderAll();
    void golden();
    void workflows();
    void videoFormats();
    void outputWindowsLaunch();
    void measureBeat(const QString& label, bool video);

    // Helpers working on the live graph.
    QString randomSource();
    void    randomizeParameters(const QString& nodeId);
    QStringList nodeIds() const;
    QString pick(const QStringList& ids);

    MainWindow* window_;
    QString     calibVideo_;  // beattiming's video node
    struct Queued { Step step; int delayMs; };
    std::deque<Queued> steps_;
    QTimer timer_;
    std::mt19937 rng_;
    QStringList scenarios_;
    QString scratch_;   // folder for workspace / strand files
    int counter_ = 0;
    int savedNodes_ = 0;
    QString textId_;
    QString swapId_;
};
