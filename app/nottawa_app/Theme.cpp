#include "Theme.h"
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QFont>

namespace Theme {

// Palette approximation of the NottawaApp SwiftUI dark theme. Not pixel
// identical — a close dark theme we can refine in Phase A.4.
QString stylesheet() {
    return R"CSS(
        QMainWindow, QDialog { background: #16171c; color: #d8d9df; }
        QWidget             { background: #16171c; color: #d8d9df;
                              font-family: "Segoe UI"; font-size: 10pt; }

        /* Dock titles + section headers */
        QDockWidget::title {
            background: #1e2029; color: #d8d9df;
            padding: 6px 10px;
            border-bottom: 1px solid #0f1014;
            text-align: left; font-weight: 600; font-size: 9pt;
        }
        QDockWidget {
            titlebar-close-icon: none; titlebar-normal-icon: none;
        }

        /* Toolbar */
        QToolBar {
            background: #1b1d26; border-bottom: 1px solid #0f1014;
            spacing: 2px; padding: 4px;
        }
        QToolBar QToolButton {
            background: transparent; color: #d8d9df;
            padding: 4px 10px; border-radius: 4px;
        }
        QToolBar QToolButton:hover    { background: #2a2d3a; }
        QToolBar QToolButton:pressed  { background: #343849; }
        QToolBar QToolButton:checked  { background: #3f87e6; color: #ffffff; }

        /* Menu bar */
        QMenuBar          { background: #16171c; color: #d8d9df; padding: 2px; }
        QMenuBar::item    { padding: 4px 10px; background: transparent; }
        QMenuBar::item:selected { background: #2a2d3a; border-radius: 3px; }
        QMenu             { background: #1e2029; border: 1px solid #0f1014; padding: 4px; }
        QMenu::item       { padding: 5px 20px 5px 24px; }
        QMenu::item:selected { background: #3f87e6; color: #ffffff; }

        /* Status bar */
        QStatusBar        { background: #1b1d26; color: #a8aab6; border-top: 1px solid #0f1014; }
        QStatusBar::item  { border: none; }

        /* Tree / list in sidebar */
        QTreeWidget, QListWidget {
            background: #1a1c25; border: none; outline: 0;
            alternate-background-color: #1e2029;
        }
        QTreeWidget::item, QListWidget::item {
            padding: 4px 8px;
        }
        QTreeWidget::item:selected, QListWidget::item:selected {
            background: #3f87e6; color: #ffffff;
        }
        QHeaderView::section {
            background: #1e2029; color: #a8aab6;
            padding: 4px 8px; border: none;
            border-bottom: 1px solid #0f1014;
        }

        /* Buttons */
        QPushButton {
            background: #2a2d3a; color: #d8d9df;
            border: 1px solid #0f1014; padding: 6px 14px; border-radius: 4px;
        }
        QPushButton:hover   { background: #343849; }
        QPushButton:pressed { background: #3f87e6; color: #ffffff; }
        QPushButton:default { background: #3f87e6; color: #ffffff; border: none; }

        /* Scroll */
        QScrollBar:vertical {
            background: #16171c; width: 10px; margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #343849; border-radius: 5px; min-height: 20px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    )CSS";
}

void apply(QApplication& app) {
    app.setStyle("Fusion");

    QPalette pal;
    pal.setColor(QPalette::Window,          QColor("#16171c"));
    pal.setColor(QPalette::WindowText,      QColor("#d8d9df"));
    pal.setColor(QPalette::Base,            QColor("#1a1c25"));
    pal.setColor(QPalette::AlternateBase,   QColor("#1e2029"));
    pal.setColor(QPalette::Text,            QColor("#d8d9df"));
    pal.setColor(QPalette::Button,          QColor("#2a2d3a"));
    pal.setColor(QPalette::ButtonText,      QColor("#d8d9df"));
    pal.setColor(QPalette::Highlight,       QColor("#3f87e6"));
    pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    pal.setColor(QPalette::ToolTipBase,     QColor("#1e2029"));
    pal.setColor(QPalette::ToolTipText,     QColor("#d8d9df"));
    pal.setColor(QPalette::PlaceholderText, QColor("#6b6e7c"));
    app.setPalette(pal);

    app.setStyleSheet(stylesheet());
}

} // namespace Theme
