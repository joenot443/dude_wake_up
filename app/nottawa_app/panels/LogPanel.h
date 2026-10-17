#pragma once
#include <QWidget>
class QPlainTextEdit;

// LogPanel — bottom dock. Captures qInfo/qWarning/qCritical via a
// qInstallMessageHandler and streams them into a scrollback. Phase A.1
// uses it for the engine FPS line + startup diagnostics.
class LogPanel : public QWidget {
    Q_OBJECT
public:
    explicit LogPanel(QWidget* parent = nullptr);
    void append(const QString& line);
private:
    QPlainTextEdit* text_ = nullptr;
};
