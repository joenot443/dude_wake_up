// Nottawa Qt spike — Day 3: 1920x1080 FBO, FPS/frame-time overlay, soak.
//
// Go/no-go for picking Qt as the production framework.

#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QOffscreenSurface>
#include <QSurfaceFormat>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "EngineThread.h"
#include "EngineWidget.h"

int main(int argc, char** argv) {
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(0);
    fmt.setStencilBufferSize(0);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);

    QMainWindow w;
    w.setWindowTitle("Nottawa Qt spike — Day 3: 1920x1080 soak");
    w.resize(1280, 800);

    auto* central = new QWidget;
    auto* layout  = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* engineWidget = new EngineWidget;
    layout->addWidget(engineWidget, /*stretch=*/1);

    // FPS/frame-time overlay. Just a dark strip at the bottom. Updated
    // on a 1Hz timer by sampling the engine's rolling stats.
    auto* stats = new QLabel("waiting for engine…");
    stats->setStyleSheet("QLabel { background: #1a1a1f; color: #e0e0e0; padding: 6px; font-family: Consolas; }");
    stats->setMinimumHeight(26);
    layout->addWidget(stats);

    w.setCentralWidget(central);

    EngineThread* engine = nullptr;

    QObject::connect(engineWidget, &EngineWidget::glReady, [&]() {
        // QOffscreenSurface MUST be created on the GUI thread — pass it
        // in to the engine thread which will makeCurrent against it.
        auto* surface = new QOffscreenSurface;
        surface->setFormat(QSurfaceFormat::defaultFormat());
        surface->create();

        engine = new EngineThread(engineWidget, surface);
        engine->start();
        qInfo() << "engine started";
    });

    QTimer statsTimer;
    QObject::connect(&statsTimer, &QTimer::timeout, [&]() {
        if (!engine) return;
        const double fps  = engine->currentFps();
        const double p99  = engine->currentP99FrameMs();
        const quint64 fc  = engine->currentFrame();
        stats->setText(QString("engine: fps=%1  p99=%2 ms  frames=%3  — 1920x1080 FBO")
                       .arg(fps,  0, 'f', 1)
                       .arg(p99,  0, 'f', 2)
                       .arg(fc));
    });
    statsTimer.start(1000);

    w.show();
    return app.exec();
}
