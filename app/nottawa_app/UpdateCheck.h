#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

class QNetworkAccessManager;

// "A newer version is available" for copies installed outside the Microsoft
// Store (which updates its own apps). At most once a day, reads
// https://nottawa.app/windows/latest.json:
//   { "version": "2.1.1", "url": "https://nottawa.app/download", "notes": "…" }
// and reports a version newer than this one. Never shows UI itself.
class UpdateCheck : public QObject {
    Q_OBJECT
public:
    explicit UpdateCheck(QObject* parent = nullptr);

    // Store (packaged) copies get updates from the Store instead.
    static bool supported();
    // Supported, not a test run, and not turned off in Settings
    // (QSettings "updates/check").
    static bool enabled();
    // Checks if a day has passed since the last check (or `force`).
    void start(bool force = false);

    // Newer than "2.1.0"-style `current`?
    static bool isNewer(const QString& version, const QString& current);

signals:
    void available(const QString& version, const QUrl& download, const QString& notes);

private:
    QNetworkAccessManager* network_;
};
