#include "LogPanel.h"

#include "Theme.h"
#include <QPlainTextEdit>
#include <QVBoxLayout>

LogPanel::LogPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    text_ = new QPlainTextEdit;
    text_->setReadOnly(true);
    text_->setMaximumBlockCount(2000);
    text_->setStyleSheet(QString("QPlainTextEdit { background: %1; color: %2;"
                                 " font-family: Consolas, monospace; font-size: 9pt;"
                                 " border: none; padding: 4px; }")
                             .arg(Theme::background.name(), Theme::textSecondary.name()));
    layout->addWidget(text_);
}

void LogPanel::append(const QString& line) {
    text_->appendPlainText(line);
}
