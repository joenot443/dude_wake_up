#include "Theme.h"
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QFont>

namespace Theme {

// The app-wide stylesheet, from the Midnight tokens in Theme.h. Panels add
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
        QMenu::item:selected      { background: @accent; color: #ffffff; }
        QMenu::item:disabled      { color: @text3; }
        QMenu::separator          { height: 1px; background: @border; margin: 4px 6px; }
        QMenu::indicator          { width: 12px; height: 12px; left: 6px; }

        /* Buttons */
        QPushButton               { background: @surface; color: @text; border: 1px solid @border;
                                    padding: 5px 14px; border-radius: 6px; }
        QPushButton:hover         { background: @hover; }
        QPushButton:pressed       { background: @pressed; }
        QPushButton:default       { background: @accent; color: #ffffff; border: none; }
        QPushButton:default:hover { background: @accentHover; }
        QPushButton:disabled      { color: @text3; }
        QToolButton               { background: transparent; color: @text2; border: none; border-radius: 6px; padding: 4px; }
        QToolButton:hover         { background: @hover; color: @text; }
        QToolButton:checked       { color: @accent; }

        /* Text entry */
        QLineEdit, QPlainTextEdit, QTextEdit, QSpinBox, QComboBox {
                                    background: @bg3; color: @text; border: 1px solid @border;
                                    border-radius: 5px; padding: 3px 6px;
                                    selection-background-color: @accent; selection-color: #ffffff; }
        QLineEdit:focus, QPlainTextEdit:focus, QSpinBox:focus, QComboBox:focus { border-color: @accent; }
        QComboBox::drop-down      { border: none; width: 18px; }
        QComboBox QAbstractItemView { background: @surface; border: 1px solid @border; selection-background-color: @accent; }

        /* Lists */
        QListWidget, QTreeWidget  { background: @bg2; border: none; outline: 0; }
        QListWidget::item:selected, QTreeWidget::item:selected { background: @accent; color: #ffffff; }

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
        {"@background", background}, {"@bg2", backgroundSecondary}, {"@bg3", backgroundTertiary},
        {"@surface", surface}, {"@hover", surfaceHover}, {"@pressed", surfacePressed},
        {"@border", border}, {"@text3", textTertiary}, {"@text2", textSecondary}, {"@text", textPrimary},
        {"@accentHover", accentHover}, {"@accent", accent},
    };
    for (const auto& t : tokens) css.replace(t.token, t.color.name());
    return css;
}

void apply(QApplication& app) {
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
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::ToolTipBase,     surface);
    pal.setColor(QPalette::ToolTipText,     textPrimary);
    pal.setColor(QPalette::PlaceholderText, textTertiary);
    pal.setColor(QPalette::Link,            accent);
    app.setPalette(pal);

    app.setStyleSheet(stylesheet());
}

} // namespace Theme
