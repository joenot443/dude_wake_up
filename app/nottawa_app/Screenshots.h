#pragma once

#include <QString>

class QImage;
class QWidget;

// Where screenshots go and in what format, shared by the toolbar's capture
// button, preview windows and Settings (after the macOS ScreenshotService).
namespace Screenshots {

// The remembered folder, or empty when none has been chosen yet.
QString folder();
// Asks for a folder and remembers it; empty if cancelled.
QString chooseFolder(QWidget* parent);
// "png" or "jpg".
QString format();
void    setFormat(const QString& format);
// Saves `image` as "Nottawa <date> at <time>", asking for a folder first
// when none is set. Returns the file written, or empty on failure/cancel.
QString save(const QImage& image, QWidget* parent);

} // namespace Screenshots
