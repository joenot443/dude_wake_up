#include "Screenshots.h"

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QSettings>
#include <QStandardPaths>

namespace Screenshots {

QString folder() {
    const QString f = QSettings().value("screenshots/folder").toString();
    return QFileInfo(f).isDir() ? f : QString();
}

QString chooseFolder(QWidget* parent) {
    const QString start = folder().isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) : folder();
    const QString f = QFileDialog::getExistingDirectory(parent, "Choose a folder for screenshots", start);
    if (!f.isEmpty()) QSettings().setValue("screenshots/folder", f);
    return f;
}

QString format() { return QSettings().value("screenshots/format", "png").toString() == "jpg" ? "jpg" : "png"; }
void setFormat(const QString& f) { QSettings().setValue("screenshots/format", f); }

QString save(const QImage& image, QWidget* parent) {
    if (image.isNull()) return {};
    QString dir = folder();
    if (dir.isEmpty()) dir = chooseFolder(parent);
    if (dir.isEmpty()) return {};
    const QString file = QDir(dir).filePath(
        QString("Nottawa %1.%2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd 'at' HH.mm.ss"), format()));
    return image.save(file, nullptr, format() == "jpg" ? 92 : -1) ? file : QString();
}

} // namespace Screenshots
