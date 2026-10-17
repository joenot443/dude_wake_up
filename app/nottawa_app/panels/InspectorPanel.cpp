#include "InspectorPanel.h"
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <Qt>

InspectorPanel::InspectorPanel(QWidget* parent) : QWidget(parent) {
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(12, 12, 12, 12);
    layout_->setSpacing(10);

    shaderLabel_ = new QLabel("Shader: (none)");
    shaderLabel_->setStyleSheet("color: #d8d9df; font-weight: 600; font-size: 11pt;");
    layout_->addWidget(shaderLabel_);

    auto* sep = new QLabel();
    sep->setFixedHeight(1);
    sep->setStyleSheet("background: #0f1014;");
    layout_->addWidget(sep);

    auto* row = new QHBoxLayout;
    row->setSpacing(8);
    row->addWidget(new QLabel("Brightness"));
    row->addStretch(1);

    spin_ = new QDoubleSpinBox;
    spin_->setRange(1.0, 1000.0);
    spin_->setDecimals(1);
    spin_->setSingleStep(10.0);
    spin_->setValue(400.0);
    spin_->setFixedWidth(90);
    row->addWidget(spin_);
    layout_->addLayout(row);

    slider_ = new QSlider(Qt::Horizontal);
    slider_->setRange(1, 1000);
    slider_->setValue(400);
    layout_->addWidget(slider_);

    // Keep slider and spin in lock-step; both emit brightnessChanged.
    connect(slider_, &QSlider::valueChanged, this, [this](int v) {
        if (static_cast<int>(spin_->value()) != v) spin_->setValue(v);
        emit brightnessChanged(static_cast<double>(v));
    });
    connect(spin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) {
        const int iv = static_cast<int>(v + 0.5);
        if (slider_->value() != iv) slider_->setValue(iv);
    });

    layout_->addStretch(1);

    auto* note = new QLabel("Shaders without a `brightness` uniform ignore it.");
    note->setStyleSheet("color: #6b6e7c; font-style: italic; font-size: 9pt;");
    note->setWordWrap(true);
    layout_->addWidget(note);
}

void InspectorPanel::setCurrentShader(const QString& name) {
    shaderLabel_->setText("Shader: " + (name.isEmpty() ? "(none)" : name));
}
