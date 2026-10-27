#pragma once

#include "EngineBridge.h"

#include <QHash>
#include <QSettings>
#include <QObject>
#include <QVariant>
#include <QVector>

// App preferences: the engine's, shared with the macOS app (docs/SETTINGS.md).
//
// Some are needed before the engine runs (the theme is applied before the
// window exists; menus are built before the engine starts), so a copy of
// every value is kept in QSettings ("prefs/<id>") and read until then; on
// the first run after preferences moved to the engine, the old QSettings
// key stands in. changed(id) follows every change, whoever made it (this
// app, the engine, a reset).
class Prefs : public QObject {
    Q_OBJECT
public:
    static Prefs* instance();

    static bool    boolean(const QString& id) { return number(id) != 0.0; }
    static double  number(const QString& id);
    static QString string(const QString& id);
    static void    set(const QString& id, double value);  // Bool: 0 / 1
    static void    set(const QString& id, const QString& value);

    // The schema (fixed for the run), and one setting's entry (nullptr if unknown).
    const QVector<SettingInfo>& settings() const { return settings_; }
    const SettingInfo* info(const QString& id) const;
    // Whether this Windows app shows it.
    static bool shown(const SettingInfo& s);
    // A restart-to-apply setting changed since launch.
    bool restartPending() const;

    // Once the engine is running: moves the old QSettings values into it
    // (the first time), refreshes the copy and reports what differs from it.
    void engineStarted();

signals:
    void changed(const QString& id);

private:
    explicit Prefs(QObject* parent);
    QVariant stored(const SettingInfo& s) const;  // the copy / old key / default
    QVariant current(const SettingInfo& s) const;
    void     remember(const QString& id);         // copy the engine's value

    // One QSettings for the copy, kept open: Qt batches its writes. A
    // QSettings per change synced the file (lock file, disk flush) each
    // time, on the UI thread: a Reset All stalled the UI for seconds.
    QSettings*               store_;
    QVector<SettingInfo>     settings_;
    QHash<QString, QVariant> atLaunch_;  // restart-to-apply settings, as the app started with them
};
