#pragma once
#include <QWidget>
class QVBoxLayout;

// InspectorPanel — right sidebar, parameter editor for the selected
// node. Phase A.1 shows the empty state; A.3 wires selection + builds
// controls from Parameter/Oscillator metadata.
class InspectorPanel : public QWidget {
    Q_OBJECT
public:
    explicit InspectorPanel(QWidget* parent = nullptr);
private:
    QVBoxLayout* layout_ = nullptr;
};
