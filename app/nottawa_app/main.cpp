#include "MainWindow.h"
#include "Theme.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QSurfaceFormat>

namespace {
// The engine's data root (fonts, images, shaders, templates): data/ next to
// the exe when deployed, or the dev tree's bin/data found by walking up from
// app/build/nottawa_app/Release/.
QString findDataDir() {
    QDir dir(QCoreApplication::applicationDirPath());
    for (int depth = 0; depth < 6; ++depth) {
        for (const char* rel : {"data", "bin/data"}) {
            const QString candidate = dir.absoluteFilePath(rel);
            if (QFileInfo::exists(candidate + "/shaders")) return QDir::cleanPath(candidate);
        }
        if (!dir.cdUp()) break;
    }
    return {};
}
} // anon

int main(int argc, char** argv) {
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(0);
    fmt.setStencilBufferSize(0);
    // No vsync wait on the GUI thread: while it blocked in SwapBuffers the
    // engine thread's GL calls stalled with it (engine ~45 fps instead of 60).
    // DWM composites windowed output, so this doesn't tear; the stage paces
    // its own repaints to the display's refresh rate.
    fmt.setSwapInterval(0);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);
    app.setApplicationName("Nottawa");
    app.setOrganizationName("Nottawa");
    Theme::apply(app);

    MainWindow w(findDataDir());
    w.show();
    return app.exec();
}
