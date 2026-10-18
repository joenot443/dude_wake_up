#pragma once

#include <QString>
#include <QWidget>
#include <functional>

class QLabel;
class StageWidget;

// A separate window showing one node's output. Double-click or F11 toggles
// full screen. As on macOS: S saves a screenshot, Shift+S chooses the
// screenshot folder first, C copies the frame.
class PreviewWindow : public QWidget {
    Q_OBJECT
public:
    // `capture` returns a node's current frame at full resolution.
    using Capture = std::function<QImage(const QString& id)>;
    PreviewWindow(const QString& connectableId, const QString& name, Capture capture, QWidget* parent = nullptr);
    void setNodeName(const QString& name);
    // Switches the window to another node's output.
    void setSource(const QString& connectableId, const QString& name);
    QString source() const { return id_; }

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;

private:
    void toggleFullscreen();
    void flash(const QString& text);

    StageWidget* view_ = nullptr;
    QLabel*      notice_ = nullptr;
    QString      id_;
    Capture      capture_;
};
