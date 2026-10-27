#pragma once

#include <QObject>
#include <QString>
#include <memory>

class QLocalServer;
class QLockFile;

// One instance per app data folder (two would share and overwrite the
// session, config and logs). The first instance holds a lock and listens on
// a local socket; a later launch hands its request (a nottawa:// link, or
// nothing: "come to the front") to it and exits.
class SingleInstance : public QObject {
    Q_OBJECT
public:
    explicit SingleInstance(const QString& appDataDir, QObject* parent = nullptr);
    ~SingleInstance() override;

    // Takes the lock and starts listening. False if another instance has
    // it; forwardToRunning() then passes this launch's request on.
    bool acquire();
    bool forwardToRunning(const QString& request);
    // Lets the next instance take over (a theme restart relaunches).
    void release();

signals:
    void requestReceived(const QString& request);

private:
    QString serverName_;
    std::unique_ptr<QLockFile> lock_;
    QLocalServer* server_ = nullptr;
};
