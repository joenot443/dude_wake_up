#include "Theme.h"
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QGuiApplication>
#include <QSettings>
#include <QStyleHints>

namespace Theme {

namespace {
// Token order: background, backgroundSecondary, backgroundTertiary, surface,
// surfaceHover, surfacePressed, border, borderSubtle, textPrimary,
// textSecondary, textTertiary, textOnAccent, accent, accentHover,
// accentSubtle, destructive, success, warning.
struct Palette { const char* name; bool dark; QRgb c[18]; };
const Palette kPalettes[] = {
    {"Midnight", true, {0x0B0E14, 0x131720, 0x1C2130, 0x1E2433, 0x262D3F, 0x2E3650, 0x2D3548, 0x1E2433, 0xE6EDF3,
                        0x8B949E, 0x6E7681, 0xFFFFFF, 0xF08C00, 0xFFA033, 0x2D1E0A, 0xF85149, 0x3FB950, 0xD29922}},
    {"Daylight", false, {0xFFFFFF, 0xF7F7F8, 0xEDEDF0, 0xFFFFFF, 0xF3F3F6, 0xE8E8EC, 0xE0E0E5, 0xEDEDF0, 0x1A1A2E,
                         0x6B6B80, 0xA0A0B0, 0xFFFFFF, 0xF08C00, 0xFFA033, 0xFFF3E0, 0xE5484D, 0x1A7F37, 0xBF8700}},
    {"Ember", true, {0x151010, 0x1E1614, 0x2A201C, 0x2A201C, 0x352A25, 0x40342E, 0x3D302A, 0x2A201C, 0xF5EDE8,
                     0xA89890, 0x8A7B72, 0x151010, 0xF08C00, 0xFFA033, 0x2D1E0A, 0xFF6B6B, 0x69DB7C, 0xFFD43B}},
    {"Arctic", false, {0xF0F4F8, 0xE2E8F0, 0xCBD5E1, 0xFFFFFF, 0xF1F5F9, 0xE2E8F0, 0xCBD5E1, 0xE2E8F0, 0x0F172A,
                       0x475569, 0x94A3B8, 0xFFFFFF, 0x0EA5E9, 0x0284C7, 0xE0F2FE, 0xEF4444, 0x22C55E, 0xEAB308}},
    {"Forest", true, {0x0C1510, 0x121F18, 0x1A2E22, 0x1A2E22, 0x223B2C, 0x2A4836, 0x2D4A38, 0x1A2E22, 0xD1E7DD,
                      0x8BAA9A, 0x749584, 0x0C1510, 0x34D399, 0x6EE7B7, 0x0A2618, 0xFB7185, 0x4ADE80, 0xFBBF24}},
};
bool g_dark = true;

void load() {
    QString name = preset();
    if (name == "Default") {
        const bool light = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light;
        name = light ? "Daylight" : "Midnight";
    }
    const Palette* palette = &kPalettes[0];
    for (const Palette& p : kPalettes) if (name == p.name) palette = &p;
    QColor* tokens[] = {&background, &backgroundSecondary, &backgroundTertiary, &surface, &surfaceHover,
                        &surfacePressed, &border, &borderSubtle, &textPrimary, &textSecondary, &textTertiary,
                        &textOnAccent, &accent, &accentHover, &accentSubtle, &destructive, &success, &warning};
    for (int i = 0; i < 18; ++i) *tokens[i] = QColor::fromRgb(palette->c[i]);
    g_dark = palette->dark;
}
} // anon

// macOS lists Default, Ember, Arctic and Forest; Midnight and Daylight are
// offered too, so the dark look is available whatever Windows' mode.
QStringList presets() { return {"Default", "Midnight", "Daylight", "Ember", "Arctic", "Forest"}; }
QString preset() {
    const QString name = QSettings().value("ui/theme", "Default").toString();
    return presets().contains(name) ? name : QStringLiteral("Default");
}
void setPreset(const QString& name) { QSettings().setValue("ui/theme", name); }
bool isDark() { return g_dark; }

// The app-wide stylesheet, from the active preset's tokens. Panels add
// their own rules on top for specifics.
QString stylesheet() {
    QString css = R"CSS(
        QMainWindow, QDialog      { background: @background; color: @text; }
        QWidget                   { background: @bg2; color: @text;
                                    font-family: "Segoe UI"; font-size: 9pt; }
        /* Labels sit on whatever surface they're placed on (cards, panels). */
        QLabel                    { background: transparent; }
        QToolTip                  { background: @surface; color: @text; border: 1px solid @border;
                                    padding: 4px 6px; border-radius: 4px; }

        /* Menu bar and menus */
        QMenuBar                  { background: @bg2; color: @text; padding: 2px; border-bottom: 1px solid @border; }
        QMenuBar::item            { padding: 4px 10px; background: transparent; border-radius: 4px; }
        QMenuBar::item:selected   { background: @hover; }
        QMenu                     { background: @surface; border: 1px solid @border; padding: 4px; border-radius: 6px; }
        QMenu::item               { padding: 5px 22px 5px 22px; border-radius: 4px; color: @text; }
        QMenu::item:selected      { background: @accent; color: @onAccent; }
        QMenu::item:disabled      { color: @text3; }
        QMenu::separator          { height: 1px; background: @border; margin: 4px 6px; }
        QMenu::indicator          { width: 12px; height: 12px; left: 6px; }

        /* Buttons */
        QPushButton               { background: @surface; color: @text; border: 1px solid @border;
                                    padding: 5px 14px; border-radius: 6px; }
        QPushButton:hover         { background: @hover; }
        QPushButton:pressed       { background: @pressed; }
        QPushButton:default       { background: @accent; color: @onAccent; border: none; }
        QPushButton:default:hover { background: @accentHover; }
        QPushButton:disabled      { color: @text3; }
        QToolButton               { background: transparent; color: @text2; border: none; border-radius: 6px; padding: 4px; }
        QToolButton:hover         { background: @hover; color: @text; }
        QToolButton:checked       { color: @accent; }

        /* Text entry */
        QLineEdit, QPlainTextEdit, QTextEdit, QSpinBox, QComboBox {
                                    background: @bg3; color: @text; border: 1px solid @border;
                                    border-radius: 5px; padding: 3px 6px;
                                    selection-background-color: @accent; selection-color: @onAccent; }
        QLineEdit:focus, QPlainTextEdit:focus, QSpinBox:focus, QComboBox:focus { border-color: @accent; }
        QComboBox::drop-down      { border: none; width: 18px; }
        QComboBox QAbstractItemView { background: @surface; border: 1px solid @border; selection-background-color: @accent; }

        /* Lists */
        QListWidget, QTreeWidget  { background: @bg2; border: none; outline: 0; }
        QListWidget::item:selected, QTreeWidget::item:selected { background: @accent; color: @onAccent; }

        /* Scroll bars: thin, rounded */
        QScrollArea               { border: none; }
        QScrollBar:vertical       { background: transparent; width: 10px; margin: 2px; }
        QScrollBar::handle:vertical { background: @pressed; border-radius: 3px; min-height: 24px; }
        QScrollBar::handle:vertical:hover { background: @border; }
        QScrollBar:horizontal     { background: transparent; height: 10px; margin: 2px; }
        QScrollBar::handle:horizontal { background: @pressed; border-radius: 3px; min-width: 24px; }
        QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
        QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }

        /* Splitters (panel resize handles) */
        QSplitter::handle         { background: @border; }
        QSplitter::handle:hover   { background: @accent; }
    )CSS";
    // Longer tokens before the ones they start with (@text3 before @text...).
    const struct { const char* token; const QColor& color; } tokens[] = {
        {"@onAccent", textOnAccent},
        {"@background", background}, {"@bg2", backgroundSecondary}, {"@bg3", backgroundTertiary},
        {"@surface", surface}, {"@hover", surfaceHover}, {"@pressed", surfacePressed},
        {"@border", border}, {"@text3", textTertiary}, {"@text2", textSecondary}, {"@text", textPrimary},
        {"@accentHover", accentHover}, {"@accent", accent},
    };
    for (const auto& t : tokens) css.replace(t.token, t.color.name());
    return css;
}

void apply(QApplication& app) {
    load();
    app.setStyle("Fusion");

    QPalette pal;
    pal.setColor(QPalette::Window,          backgroundSecondary);
    pal.setColor(QPalette::WindowText,      textPrimary);
    pal.setColor(QPalette::Base,            backgroundTertiary);
    pal.setColor(QPalette::AlternateBase,   surface);
    pal.setColor(QPalette::Text,            textPrimary);
    pal.setColor(QPalette::Button,          surface);
    pal.setColor(QPalette::ButtonText,      textPrimary);
    pal.setColor(QPalette::Highlight,       accent);
    pal.setColor(QPalette::HighlightedText, textOnAccent);
    pal.setColor(QPalette::ToolTipBase,     surface);
    pal.setColor(QPalette::ToolTipText,     textPrimary);
    pal.setColor(QPalette::PlaceholderText, textTertiary);
    pal.setColor(QPalette::Link,            accent);
    app.setPalette(pal);

    app.setStyleSheet(stylesheet());
}

} // namespace Theme
