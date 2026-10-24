#include "CrashHandler.h"
#include "MainWindow.h"
#include "StressTest.h"
#include "Theme.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QSurfaceFormat>
#include <cstdio>

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

// Per-user app data, matching the engine's ConfigService: NOTTAWA_APPDATA
// when set (test runs), else %LOCALAPPDATA%/Nottawa.
QString appDataDir() {
    const QString override = qEnvironmentVariable("NOTTAWA_APPDATA");
    if (!override.isEmpty()) return override;
    return QDir(qEnvironmentVariable("LOCALAPPDATA")).absoluteFilePath("Nottawa");
}
// The engine logs to stdout/stderr, which a Windows GUI app discards; send
// them to <app data>/logs/engine.log (rewritten each launch).
void captureEngineLog() {
    const QString dir = QDir(appDataDir()).filePath("logs");
    QDir().mkpath(dir);
    const std::wstring path = QDir::toNativeSeparators(QDir(dir).filePath("engine.log")).toStdWString();
    if (_wfreopen(path.c_str(), L"w", stdout)) setvbuf(stdout, nullptr, _IONBF, 0);
    if (_wfreopen(path.c_str(), L"a", stderr)) setvbuf(stderr, nullptr, _IONBF, 0);
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
    // UI settings live in an INI file under the app data folder (not the
    // registry), so a throwaway NOTTAWA_APPDATA isolates them too. Kept in
    // settings/: the organisation subfolder would otherwise be "Nottawa",
    // which on Windows is the engine's own "nottawa" folder.
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QDir(appDataDir()).filePath("settings"));
    Theme::apply(app);
    if (!qEnvironmentVariableIsEmpty("NOTTAWA_NO_PUBLISH")) {  // test runs open nothing outside the app
        auto* sink = new UrlSink(&app);
        for (const char* scheme : {"http", "https", "mailto", "file"}) QDesktopServices::setUrlHandler(scheme, sink, "open");
    }
    captureEngineLog();
    CrashHandler::install();

    int code = 0;
    {
        MainWindow w(findDataDir());
        w.show();
        code = app.exec();
    }  // the window stops the engine and saves the session here
    // Settings > Theme > Restart Now: relaunch once this instance is done.
    if (app.property("restartRequested").toBool())
        QProcess::startDetached(QCoreApplication::applicationFilePath(), QCoreApplication::arguments().mid(1));
    return code;
}
