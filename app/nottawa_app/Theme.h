#pragma once
#include <QColor>
#include <QString>
#include <QStringList>
class QApplication;

// Nottawa visual theme: the NottawaApp macOS presets' colour tokens and a
// Qt stylesheet built from them. Loaded once at startup.
namespace Theme {
    // Colour tokens of the active preset (the macOS ThemePresets), set by
    // apply() at startup from Settings; widgets read them as they're built,
    // so a change of preset takes effect on the next launch.
    inline QColor background;
    inline QColor backgroundSecondary;
    inline QColor backgroundTertiary;
    inline QColor surface;
    inline QColor surfaceHover;
    inline QColor surfacePressed;
    inline QColor border;
    inline QColor borderSubtle;
    inline QColor textPrimary;
    inline QColor textSecondary;
    inline QColor textTertiary;
    inline QColor textOnAccent;
    inline QColor accent;
    inline QColor accentHover;
    inline QColor accentSubtle;
    inline QColor destructive;
    inline QColor success;
    inline QColor warning;

    // Preset names, as in Settings: Default (Midnight, or Daylight when
    // Windows is in light mode), Ember, Arctic, Forest.
    QStringList presets();
    QString     preset();           // the saved choice
    void        setPreset(const QString& name);  // saved; applies on next launch
    bool        isDark();           // the active palette is a dark one

    QString stylesheet();
    void    apply(QApplication& app);
}
