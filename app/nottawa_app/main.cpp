#include "MainWindow.h"
#include "OFStageEngine.h"
#include "Theme.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QSurfaceFormat>

namespace {
// Finds shaders/ next to the exe or in a parent (dev tree: the exe is at
// app/build/nottawa_app/Release/, shaders at the repo root).
QString findShadersDir() {
    QDir dir(QCoreApplication::applicationDirPath());
    for (int depth = 0; depth < 6; ++depth) {
        const QString candidate = dir.absoluteFilePath("shaders");
        if (QFileInfo::exists(candidate + "/Accretion.frag")) return candidate;
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
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);
    app.setApplicationName("Nottawa");
    app.setOrganizationName("Nottawa");
    Theme::apply(app);

    setNottawaShadersDir(findShadersDir());

    MainWindow w;
    w.show();
    return app.exec();
}
