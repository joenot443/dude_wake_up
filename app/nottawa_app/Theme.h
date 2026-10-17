#pragma once
#include <QColor>
#include <QString>
class QApplication;

// Nottawa visual theme — a Qt stylesheet string mirroring the NottawaApp
// macOS SwiftUI dark palette. Loaded once at startup.
namespace Theme {
    // The macOS app's default ("Midnight") colour tokens.
    inline const QColor background          {0x0B, 0x0E, 0x14};
    inline const QColor backgroundSecondary {0x13, 0x17, 0x20};
    inline const QColor backgroundTertiary  {0x1C, 0x21, 0x30};
    inline const QColor surface             {0x1E, 0x24, 0x33};
    inline const QColor surfaceHover        {0x26, 0x2D, 0x3F};
    inline const QColor surfacePressed      {0x2E, 0x36, 0x50};
    inline const QColor border              {0x2D, 0x35, 0x48};
    inline const QColor borderSubtle        {0x1E, 0x24, 0x33};
    inline const QColor textPrimary         {0xE6, 0xED, 0xF3};
    inline const QColor textSecondary       {0x8B, 0x94, 0x9E};
    inline const QColor textTertiary        {0x6E, 0x76, 0x81};
    inline const QColor accent              {0xF0, 0x8C, 0x00};
    inline const QColor accentHover         {0xFF, 0xA0, 0x33};
    inline const QColor accentSubtle        {0x2D, 0x1E, 0x0A};
    inline const QColor destructive         {0xF8, 0x51, 0x49};
    inline const QColor success             {0x3F, 0xB9, 0x50};
    inline const QColor warning             {0xD2, 0x99, 0x22};

    QString stylesheet();
    void    apply(QApplication& app);
}
