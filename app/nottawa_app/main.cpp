#include "CrashHandler.h"
#include "SingleInstance.h"
#include "MainWindow.h"
#include "StressTest.h"
#include "Theme.h"

#include <QApplication>
#include <QMessageBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QSurfaceFormat>
#include <QTimer>
#include <cstdio>
#include <memory>

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
    app.setApplicationVersion(NOTTAWA_VERSION);
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
    // A nottawa:// link this launch was opened with (a shared strand).
    QString link;
    for (const QString& arg : app.arguments().mid(1))
        if (arg.startsWith("nottawa:", Qt::CaseInsensitive)) link = arg;
    // One instance per app data folder: a second launch hands its link (or
    // just "come to the front") to the running one.
    SingleInstance instance(appDataDir());
    if (!instance.acquire()) {
        if (!instance.forwardToRunning(link))
            QMessageBox::information(nullptr, "Nottawa", "Nottawa is already running.");
        return 0;
    }
    app.setProperty("appDataDir", appDataDir());  // for Help > Show Logs Folder
    captureEngineLog();
    CrashHandler::install();

    int code = 0;
    {
        MainWindow w(findDataDir());
        w.show();
        QObject::connect(&instance, &SingleInstance::requestReceived, &w, &MainWindow::handleExternalRequest);
        if (!link.isEmpty()) QTimer::singleShot(0, &w, [&w, link] { w.handleExternalRequest(link); });
        QTimer heartbeat;
        QObject::connect(&heartbeat, &QTimer::timeout, &CrashHandler::heartbeat);
        heartbeat.start(250);
        CrashHandler::startHangWatchdog(2000);
        code = app.exec();
    }  // the window stops the engine and saves the session here
    // Settings > Theme > Restart Now: relaunch once this instance is done.
    if (app.property("restartRequested").toBool()) {
        instance.release();  // the new instance takes over
        QStringList args;
        for (const QString& arg : app.arguments().mid(1))
            if (!arg.startsWith("nottawa:", Qt::CaseInsensitive)) args << arg;  // the link's been opened
        QProcess::startDetached(QCoreApplication::applicationFilePath(), args);
    }
    return code;
}
