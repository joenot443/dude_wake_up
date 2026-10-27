#include "UpdateCheck.h"
#include "Prefs.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>
#include <QVersionNumber>

#include <windows.h>
#include <appmodel.h>

namespace {
const char* kFeed = "https://nottawa.app/windows/latest.json";
constexpr qint64 kInterval = 24 * 60 * 60;  // seconds between checks

// Installed from the Store: running with a package identity.
bool packaged() {
    UINT32 length = 0;
    return GetCurrentPackageFullName(&length, nullptr) != APPMODEL_ERROR_NO_PACKAGE;
}
}

UpdateCheck::UpdateCheck(QObject* parent) : QObject(parent), network_(new QNetworkAccessManager(this)) {}

bool UpdateCheck::supported() { return !packaged(); }

bool UpdateCheck::enabled() {
    if (!supported() || !qEnvironmentVariableIsEmpty("NOTTAWA_NO_PUBLISH")) return false;
    return Prefs::boolean("updates.check");
}

bool UpdateCheck::isNewer(const QString& version, const QString& current) {
    const QVersionNumber v = QVersionNumber::fromString(version);
    return !v.isNull() && v > QVersionNumber::fromString(current);
}

void UpdateCheck::start(bool force) {
    if (!enabled()) return;
    QSettings settings;
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    if (!force && now - settings.value("updates/lastCheck", 0).toLongLong() < kInterval) {
        // Still remind about a version found earlier.
        const QString known = settings.value("updates/latestVersion").toString();
        if (isNewer(known, QCoreApplication::applicationVersion()))
            emit available(known, settings.value("updates/latestUrl").toUrl(), settings.value("updates/latestNotes").toString());
        return;
    }
    const QString override = qEnvironmentVariable("NOTTAWA_UPDATE_URL");  // for testing the check
    QNetworkRequest request(QUrl(override.isEmpty() ? QString(kFeed) : override));
    request.setTransferTimeout(10000);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Nottawa/" + QCoreApplication::applicationVersion() + " (Windows)");
    QNetworkReply* reply = network_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, now] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;  // offline: try again next launch
        const QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
        const QString version = json.value("version").toString();
        const QUrl url(json.value("url").toString());
        if (version.isEmpty() || !url.isValid() || url.scheme() != "https") return;
        QSettings settings;
        settings.setValue("updates/lastCheck", now);
        settings.setValue("updates/latestVersion", version);
        settings.setValue("updates/latestUrl", url);
        settings.setValue("updates/latestNotes", json.value("notes").toString());
        if (isNewer(version, QCoreApplication::applicationVersion()))
            emit available(version, url, json.value("notes").toString());
    });
}
