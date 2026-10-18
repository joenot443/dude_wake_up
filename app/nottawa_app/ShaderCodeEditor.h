#pragma once

#include <QTimer>
#include <QWidget>

class QLabel;
class QPlainTextEdit;
class QPushButton;

// Live GLSL editor for one shader, after the macOS ShaderCodeEditorView:
// edits recompile half a second after you stop typing (Ctrl+Enter applies
// now); a failed compile keeps the last good program running and shows
// the log. Overrides last for the session; Revert reloads from disk.
class ShaderCodeEditor : public QWidget {
    Q_OBJECT
public:
    ShaderCodeEditor(const QString& shaderId, const QString& name, QWidget* parent = nullptr);

signals:
    void overrideChanged();

private:
    void load();
    void apply();
    void revert();
    void updateState();

    QString         shaderId_;
    QPlainTextEdit* editor_;
    QLabel*         modified_;
    QLabel*         status_;
    QPushButton*    revert_;
    QPushButton*    apply_;
    QWidget*        errorBox_;
    QLabel*         errorTitle_;
    QLabel*         errorLog_;
    QTimer          debounce_;
    QString         lastApplied_;
    int             errorLine_ = 0;
};
