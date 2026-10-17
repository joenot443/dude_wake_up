// Nottawa — Windows native UI, Phase A.1 shell.
//
// Qt 6 app with Nottawa layout: toolbar, browser/inspector/log docks,
// central stage output. Stage renders a placeholder plasma shader via
// the dual-context pattern proven in spike/qt-of/engine-fbo/. Phase
// A.2 swaps the plasma for the real engine (EngineLoop + Nottawa
// services).

#include "MainWindow.h"
#include "Theme.h"

#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char** argv) {
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    // Match Nottawa's GL version. OF requests 3.2, our widget targets
    // 3.3 Core — compatible: a 3.3 context satisfies 3.2 code.
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(0);
    fmt.setStencilBufferSize(0);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);
    app.setApplicationName("Nottawa");
    app.setOrganizationName("Nottawa");
    Theme::apply(app);

    MainWindow w;
    w.show();
    return app.exec();
}
