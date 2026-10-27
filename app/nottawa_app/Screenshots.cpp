#include "Screenshots.h"
#include "Prefs.h"

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QSettings>
#include <QStandardPaths>

namespace Screenshots {

QString folder() {
    const QString f = Prefs::string("screenshots.folder");
    return QFileInfo(f).isDir() ? f : QString();
}

QString chooseFolder(QWidget* parent) {
    const QString start = folder().isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) : folder();
    const QString f = QFileDialog::getExistingDirectory(parent, "Choose a folder for screenshots", start);
    if (!f.isEmpty()) Prefs::set("screenshots.folder", f);
    return f;
}

// (The setting's choices are png and jpeg; files are .png or .jpg.)
QString format() { return Prefs::string("screenshots.format") == "jpeg" ? "jpg" : "png"; }
void setFormat(const QString& f) { Prefs::set("screenshots.format", f == "jpg" ? QStringLiteral("jpeg") : QStringLiteral("png")); }

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
