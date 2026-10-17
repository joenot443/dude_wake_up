#pragma once
#include <QWidget>
class QVBoxLayout;
class QLabel;
class QSlider;
class QDoubleSpinBox;

// InspectorPanel — right sidebar. A.2.f shows a hardcoded brightness
// slider that drives every shader's `brightness` uniform (safe no-op
// if the shader doesn't declare one). A.3 switches to introspection:
// scan the active shader's uniforms, build a control per uniform.
class InspectorPanel : public QWidget {
    Q_OBJECT
public:
    explicit InspectorPanel(QWidget* parent = nullptr);
    void setCurrentShader(const QString& name);

signals:
    void brightnessChanged(double value);

private:
    QVBoxLayout*    layout_      = nullptr;
    QLabel*         shaderLabel_ = nullptr;
    QSlider*        slider_      = nullptr;
    QDoubleSpinBox* spin_        = nullptr;
};
