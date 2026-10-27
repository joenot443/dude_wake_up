#include "SingleInstance.h"

#include <QCryptographicHash>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLockFile>

#include <windows.h>

SingleInstance::SingleInstance(const QString& appDataDir, QObject* parent) : QObject(parent) {
    QDir().mkpath(appDataDir);
    lock_ = std::make_unique<QLockFile>(QDir(appDataDir).filePath("nottawa.lock"));
    lock_->setStaleLockTime(0);  // a crashed instance's lock (its process gone) is taken over
    const QByteArray key = QCryptographicHash::hash(QDir(appDataDir).absolutePath().toLower().toUtf8(),
                                                    QCryptographicHash::Sha1).toHex().left(16);
    serverName_ = "Nottawa-" + QString::fromLatin1(key);
}

SingleInstance::~SingleInstance() = default;

bool SingleInstance::acquire() {
    if (!lock_->tryLock(500)) return false;
    server_ = new QLocalServer(this);
    QLocalServer::removeServer(serverName_);  // left behind by a crash
    server_->setSocketOptions(QLocalServer::UserAccessOption);
    if (!server_->listen(serverName_)) return true;  // still the only instance; just not reachable
    connect(server_, &QLocalServer::newConnection, this, [this] {
        while (QLocalSocket* socket = server_->nextPendingConnection()) {
            connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
            connect(socket, &QLocalSocket::readyRead, this, [this, socket] {
                if (!socket->canReadLine()) return;
                emit requestReceived(QString::fromUtf8(socket->readLine()).trimmed());
                socket->disconnectFromServer();
            });
        }
    });
    return true;
}

bool SingleInstance::forwardToRunning(const QString& request) {
    QLocalSocket socket;
    socket.connectToServer(serverName_);
    if (!socket.waitForConnected(2000)) return false;
    // This launch came from the user, so it may give the foreground away:
    // the running instance can then bring its window to the front.
    AllowSetForegroundWindow(ASFW_ANY);
    socket.write(request.toUtf8() + '\n');
    if (!socket.waitForBytesWritten(2000)) return false;
    socket.waitForDisconnected(2000);
    return true;
}

void SingleInstance::release() {
    if (server_) server_->close();
    lock_->unlock();
}
