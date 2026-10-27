#include "Prefs.h"

#include <QSettings>

#include <functional>

namespace {
bool isText(const SettingInfo& s) {
    return s.type == SettingInfo::Choice || s.type == SettingInfo::String || s.type == SettingInfo::Path;
}

// QSettings in the registry gives back bools as "true" / "false".
double toNumber(const QVariant& v) {
    const QString text = v.toString();
    if (text == "true") return 1.0;
    if (text == "false") return 0.0;
    return v.toDouble();
}

QString cacheKey(const QString& id) { return "prefs/" + id; }

// Where the Windows app kept them before they moved to the engine.
struct Legacy {
    const char* id;
    const char* key;
    std::function<QVariant(const QVariant&)> convert;
};
const QVector<Legacy>& legacy() {
    static const QVector<Legacy> list = {
        {"appearance.theme", "ui/theme", [](const QVariant& v) { return QVariant(v.toString().toLower()); }},
        {"display.nodeHints", "ui/showNodeHints", nullptr},
        {"output.autoAdvance", "ui/autoAdvanceOutputWindow", nullptr},
        {"output.advanceFade", "ui/outputWindowFadeSeconds", nullptr},
        {"output.stretchPreviews", "ui/stretchPreviews", nullptr},
        {"graph.clearOnDisconnect", "ui/clearOnDisconnect", nullptr},
        {"browser.livePreview", "browser/livePreview", nullptr},
        {"screenshots.format", "screenshots/format",
         [](const QVariant& v) { return QVariant(v.toString() == "jpg" ? "jpeg" : "png"); }},
        {"screenshots.folder", "screenshots/folder", nullptr},
        {"updates.check", "updates/check", nullptr},
    };
    return list;
}
const Legacy* legacyFor(const QString& id) {
    for (const Legacy& l : legacy()) if (id == l.id) return &l;
    return nullptr;
}
} // anon

Prefs* Prefs::instance() {
    static Prefs* prefs = new Prefs(nullptr);
    return prefs;
}

Prefs::Prefs(QObject* parent) : QObject(parent), settings_(EngineBridge::instance()->settings()) {
    for (const SettingInfo& s : settings_)
        if (s.restart & 2) atLaunch_.insert(s.id, stored(s));
    connect(EngineBridge::instance(), &EngineBridge::settingChanged, this, [this](const QString& id) {
        remember(id);
        emit changed(id);
    });
}

const SettingInfo* Prefs::info(const QString& id) const {
    for (const SettingInfo& s : settings_) if (s.id == id) return &s;
    return nullptr;
}

bool Prefs::shown(const SettingInfo& s) { return (s.platforms & 2) != 0; }

bool Prefs::restartPending() const {
    for (auto it = atLaunch_.cbegin(); it != atLaunch_.cend(); ++it)
        if (const SettingInfo* s = info(it.key()); s && current(*s) != it.value()) return true;
    return false;
}

QVariant Prefs::stored(const SettingInfo& s) const {
    QSettings settings;
    QVariant v;
    if (settings.contains(cacheKey(s.id))) v = settings.value(cacheKey(s.id));
    else if (const Legacy* l = legacyFor(s.id); l && settings.contains(l->key))
        v = l->convert ? l->convert(settings.value(l->key)) : settings.value(l->key);
    else return isText(s) ? QVariant(s.defaultString) : QVariant(s.defaultNumber);
    return isText(s) ? QVariant(v.toString()) : QVariant(toNumber(v));
}

QVariant Prefs::current(const SettingInfo& s) const {
    auto* eng = EngineBridge::instance();
    if (!eng->isRunning()) return stored(s);
    return isText(s) ? QVariant(eng->settingString(s.id)) : QVariant(eng->settingNumber(s.id));
}

void Prefs::remember(const QString& id) {
    const SettingInfo* s = info(id);
    if (s && s->type != SettingInfo::Action && EngineBridge::instance()->isRunning())
        QSettings().setValue(cacheKey(id), current(*s));
}

double Prefs::number(const QString& id) {
    const SettingInfo* s = instance()->info(id);
    return s ? toNumber(instance()->current(*s)) : 0.0;
}

QString Prefs::string(const QString& id) {
    const SettingInfo* s = instance()->info(id);
    return s ? instance()->current(*s).toString() : QString();
}

void Prefs::set(const QString& id, double value) {
    auto* eng = EngineBridge::instance();
    if (eng->isRunning()) { eng->setSetting(id, value); return; }  // the engine reports the change
    QSettings().setValue(cacheKey(id), value);
    emit instance()->changed(id);
}

void Prefs::set(const QString& id, const QString& value) {
    auto* eng = EngineBridge::instance();
    if (eng->isRunning()) { eng->setSetting(id, value); return; }
    QSettings().setValue(cacheKey(id), value);
    emit instance()->changed(id);
}

void Prefs::engineStarted() {
    auto* eng = EngineBridge::instance();
    if (!eng->isRunning()) return;
    QHash<QString, QVariant> before;
    for (const SettingInfo& s : settings_) before.insert(s.id, stored(s));

    QSettings settings;
    if (!settings.value("prefs/migrated").toBool()) {
        // An existing install had Clear on Disconnect off unless it was
        // turned on (the shared default is on): keep what it did.
        const bool existing = settings.contains("ui/theme") || settings.contains("workspaces/recent")
                              || settings.contains("help/completed") || settings.contains("browser/tab");
        for (const Legacy& l : legacy()) {
            const SettingInfo* s = info(l.id);
            if (!s) continue;
            if (settings.contains(l.key)) {
                const QVariant v = stored(*s);
                if (isText(*s)) eng->setSetting(s->id, v.toString());
                else eng->setSetting(s->id, toNumber(v));
            } else if (existing && s->id == "graph.clearOnDisconnect") {
                eng->setSetting(s->id, 0.0);
            }
        }
        settings.setValue("prefs/migrated", true);
    }

    for (const SettingInfo& s : settings_) {
        remember(s.id);
        if (current(s) != before.value(s.id)) emit changed(s.id);
    }
}
