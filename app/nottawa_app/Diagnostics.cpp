#include "Diagnostics.h"

#include "EngineBridge.h"
#include "Prefs.h"
#include "UpdateCheck.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QScreen>
#include <QStringList>
#include <QSysInfo>

namespace {
// The GPU, from a context of our own (sharing the app's, so the same driver).
QString graphics() {
    QOpenGLContext context;
    context.setShareContext(QOpenGLContext::globalShareContext());
    if (!context.create()) return "unavailable";
    QOffscreenSurface surface;
    surface.setFormat(context.format());
    surface.create();
    if (!context.makeCurrent(&surface)) return "unavailable";
    auto* gl = context.functions();
    auto text = [gl](GLenum name) {
        const auto* s = reinterpret_cast<const char*>(gl->glGetString(name));
        return s ? QString::fromUtf8(s) : QString("?");
    };
    const QString result = QString("%1 (%2), OpenGL %3").arg(text(GL_RENDERER), text(GL_VENDOR), text(GL_VERSION));
    context.doneCurrent();
    return result;
}

// The crash and hang reports the previous session's log ends with, if any.
QString previousSession() {
    QFile log(QDir(qApp->property("appDataDir").toString()).filePath("logs/engine.previous.log"));
    if (!log.open(QIODevice::ReadOnly)) return "no log";
    QStringList reports;
    while (!log.atEnd()) {
        const QString line = QString::fromUtf8(log.readLine()).trimmed();
        if (line.startsWith("=== CRASH") || line.startsWith("=== HANG") || line.startsWith("=== ENGINE STALL")) reports << line;
    }
    if (reports.isEmpty()) return "ended normally (no crash or hang report)";
    // (Not the full path: it names the Windows user.)
    return QString("%1 report(s) in logs\\engine.previous.log\n  %2").arg(reports.size()).arg(reports.mid(0, 5).join("\n  "));
}
} // anon

namespace Diagnostics {

QString report() {
    auto* eng = EngineBridge::instance();
    QStringList lines;
    lines << QString("Nottawa %1 (Windows, %2)").arg(QCoreApplication::applicationVersion(),
                                                     UpdateCheck::supported() ? "direct download" : "Microsoft Store");
    lines << QString("System: %1 (%2, kernel %3), Qt %4").arg(QSysInfo::prettyProductName(), QSysInfo::currentCpuArchitecture(),
                                                             QSysInfo::kernelVersion(), qVersion());
    lines << "Graphics: " + graphics();
    const auto screens = QGuiApplication::screens();
    for (int i = 0; i < screens.size(); ++i) {
        const QScreen* s = screens[i];
        lines << QString("Display %1: %2, %3x%4 at %5x scale, %6 Hz%7").arg(i + 1).arg(s->name())
                     .arg(s->size().width()).arg(s->size().height()).arg(s->devicePixelRatio())
                     .arg(qRound(s->refreshRate())).arg(s == QGuiApplication::primaryScreen() ? " (main)" : "");
    }
    if (eng->isRunning()) {
        const QSize res = eng->resolution();
        lines << QString("Engine: %1x%2, %3 fps, %4 nodes").arg(res.width()).arg(res.height())
                     .arg(eng->fps(), 0, 'f', 1).arg(eng->nodes().size());
        QString input = eng->selectedAudioSource();
        for (const auto& s : eng->audioSources()) if (s.id == input) input = s.name;
        lines << QString("Audio: %1, %2").arg(input, eng->isAudioActive() ? "on" : "off");
    } else {
        lines << "Engine: not running";
    }
    QStringList changed;
    for (const SettingInfo& s : Prefs::instance()->settings()) {
        if (s.type == SettingInfo::Action) continue;
        const bool text = s.type == SettingInfo::Choice || s.type == SettingInfo::String || s.type == SettingInfo::Path;
        if (s.type == SettingInfo::Path) continue;  // (folders can name people)
        const QString value = text ? Prefs::string(s.id) : QString::number(Prefs::number(s.id));
        const QString def = text ? s.defaultString : QString::number(s.defaultNumber);
        if (value != def) changed << s.id + "=" + value;
    }
    lines << "Settings changed: " + (changed.isEmpty() ? QString("none") : changed.join(", "));
    lines << "Previous session: " + previousSession();
    return lines.join('\n');
}

} // namespace Diagnostics
