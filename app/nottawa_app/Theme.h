#pragma once
#include <QString>
class QApplication;
class QColor;

// Nottawa visual theme — a Qt stylesheet string mirroring the NottawaApp
// macOS SwiftUI dark palette. Loaded once at startup.
namespace Theme {
    QString stylesheet();
    void    apply(QApplication& app);
}
