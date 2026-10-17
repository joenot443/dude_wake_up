#pragma once

#include <QWidget>

class StageWidget;

// A separate window showing one node's output — for checking a node in
// isolation or sending it to a second display. F11 or double-click toggles
// fullscreen; Esc leaves fullscreen.
class PreviewWindow : public QWidget {
    Q_OBJECT
public:
    PreviewWindow(const QString& connectableId, const QString& name, QWidget* parent = nullptr);
    void setNodeName(const QString& name);

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;

private:
    void toggleFullscreen();

    StageWidget* view_ = nullptr;
};
