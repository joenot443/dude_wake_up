#include "InspectorPanel.h"

#include "EngineBridge.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {
constexpr int kSliderSteps = 1000;

QWidget* floatControl(const ParamInfo& p) {
    auto* row = new QWidget;
    auto* h = new QHBoxLayout(row);
    h->setContentsMargins(0, 0, 0, 0);
    auto* slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, kSliderSteps);
    const float span = p.max - p.min;
    slider->setValue(span > 0 ? int((p.value - p.min) / span * kSliderSteps + 0.5f) : 0);
    auto* value = new QLabel(QString::number(p.value, 'f', 2));
    value->setFixedWidth(48);
    value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    h->addWidget(slider, 1);
    h->addWidget(value);
    const QString id = p.id;
    const float lo = p.min;
    QObject::connect(slider, &QSlider::valueChanged, row, [=](int v) {
        const float f = lo + span * v / float(kSliderSteps);
        value->setText(QString::number(f, 'f', 2));
        EngineBridge::instance()->setParameter(id, f);
    });
    return row;
}

QWidget* intControl(const ParamInfo& p) {
    const QString id = p.id;
    if (!p.options.isEmpty()) {
        auto* combo = new QComboBox;
        combo->addItems(p.options);
        combo->setCurrentIndex(p.intValue);
        QObject::connect(combo, &QComboBox::currentIndexChanged, combo, [id](int i) {
            EngineBridge::instance()->setParameter(id, i);
        });
        return combo;
    }
    auto* spin = new QSpinBox;
    spin->setRange(int(p.min), int(p.max));
    spin->setValue(p.intValue);
    QObject::connect(spin, &QSpinBox::valueChanged, spin, [id](int v) {
        EngineBridge::instance()->setParameter(id, v);
    });
    return spin;
}
} // anon

InspectorPanel::InspectorPanel(QWidget* parent) : QWidget(parent) {
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(12, 12, 12, 12);
    outer->setSpacing(10);

    title_ = new QLabel("Nothing selected");
    title_->setStyleSheet("color: #d8d9df; font-weight: 600; font-size: 11pt;");
    outer->addWidget(title_);

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    form_ = new QWidget;
    layout_ = new QVBoxLayout(form_);
    layout_->setContentsMargins(0, 0, 0, 0);
    scroll->setWidget(form_);
    outer->addWidget(scroll, 1);
}

void InspectorPanel::setNode(const QString& connectableId, const QString& name) {
    nodeId_ = connectableId;
    title_->setText(connectableId.isEmpty() ? "Nothing selected" : name);
    rebuild();
}

void InspectorPanel::rebuild() {
    while (QLayoutItem* item = layout_->takeAt(0)) {
        delete item->widget();
        delete item;
    }
    if (nodeId_.isEmpty()) return;

    auto* box = new QWidget;
    auto* form = new QFormLayout(box);
    form->setContentsMargins(0, 0, 0, 0);
    form->setLabelAlignment(Qt::AlignLeft);
    int shown = 0;
    for (const ParamInfo& p : EngineBridge::instance()->parameters(nodeId_)) {
        QWidget* control = nullptr;
        switch (p.type) {
        case ParamInfo::Standard: control = floatControl(p); break;
        case ParamInfo::Int:      control = intControl(p);   break;
        case ParamInfo::Bool: {
            auto* check = new QCheckBox;
            check->setChecked(p.boolValue);
            const QString id = p.id;
            connect(check, &QCheckBox::toggled, check, [id](bool on) { EngineBridge::instance()->setParameter(id, on); });
            control = check;
            break;
        }
        case ParamInfo::Text: {
            auto* edit = new QLineEdit(p.stringValue);
            const QString id = p.id;
            connect(edit, &QLineEdit::editingFinished, edit, [id, edit] { EngineBridge::instance()->setParameter(id, edit->text()); });
            control = edit;
            break;
        }
        case ParamInfo::Color: {
            auto* note = new QLabel("colour editing not yet supported");
            note->setStyleSheet("color: #6b6e7c; font-style: italic;");
            control = note;
            break;
        }
        default: continue;  // Hidden
        }
        form->addRow(p.name, control);
        ++shown;
    }
    if (shown == 0) {
        auto* none = new QLabel("No editable parameters.");
        none->setStyleSheet("color: #6b6e7c; font-style: italic;");
        form->addRow(none);
    }
    layout_->addWidget(box);
    layout_->addStretch(1);
}
