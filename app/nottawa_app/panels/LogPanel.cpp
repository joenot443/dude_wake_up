#include "LogPanel.h"
#include <QPlainTextEdit>
#include <QVBoxLayout>

LogPanel::LogPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    text_ = new QPlainTextEdit;
    text_->setReadOnly(true);
    text_->setMaximumBlockCount(2000);
    text_->setStyleSheet("QPlainTextEdit { background: #0e0f13; color: #a8aab6;"
                         " font-family: Consolas, monospace; font-size: 9pt;"
                         " border: none; padding: 4px; }");
    layout->addWidget(text_);
}

void LogPanel::append(const QString& line) {
    text_->appendPlainText(line);
}
