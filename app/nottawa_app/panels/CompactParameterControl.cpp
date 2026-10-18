#include "CompactParameterControl.h"

#include "Icons.h"
#include "Theme.h"
#include "ui/Controls.h"
#include "ui/ParamSelection.h"
#include "ui/WaveformPreview.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <cmath>

namespace {
QLabel* caption(const QString& text, QWidget* parent, const QColor& color = Theme::textPrimary) {
    auto* label = new QLabel(text, parent);
    label->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(color.name()));
    return label;
}
const QColor kOscillatorOn(0x32, 0xd7, 0xe6);  // the macOS cyan
} // anon

CompactParameterControl::CompactParameterControl(const ParamInfo& p, QWidget* parent) : QWidget(parent), info_(p) {
    // Shares the strip's width with its neighbour rather than widening it.
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    auto* eng = EngineBridge::instance();
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(1);
    auto* header = new QHBoxLayout;
    header->setSpacing(4);
    layout->addLayout(header);

    // Optional right-hand extras on the header row.
    auto addOscToggle = [&] {
        if (!p.hasOscillator) return;
        oscToggle_ = new QToolButton(this);
        oscToggle_->setFixedSize(16, 16);
        oscToggle_->setIconSize(QSize(11, 11));
        oscToggle_->setCursor(Qt::PointingHandCursor);
        oscToggle_->setToolTip("Oscillate");
        connect(oscToggle_, &QToolButton::clicked, this, [this, eng] {
            info_.oscEnabled = !info_.oscEnabled;
            eng->setOscillatorEnabled(info_.id, info_.oscEnabled);
            updateOscillator();
        });
        header->addWidget(oscToggle_);
    };
    auto addStar = [&] {
        auto* star = new QToolButton(this);
        star->setFixedSize(16, 16);
        star->setIconSize(QSize(10, 10));
        star->setIcon(Icons::icon(Icons::StarFilled, Theme::warning));
        star->setToolTip("Remove from favorites");
        star->setCursor(Qt::PointingHandCursor);
        connect(star, &QToolButton::clicked, this, [this, eng] {
            eng->toggleParameterFavorite(info_.id);
            emit favoritesChanged();
        });
        header->addWidget(star);
    };

    switch (p.type) {
    case ParamInfo::Bool:
        toggle_ = new DSToggle(p.name, this);
        toggle_->setSmall(true);
        connect(toggle_, &DSToggle::toggled, this, [this, eng](bool on) { eng->setParameter(info_.id, on); });
        header->addWidget(toggle_);
        header->addStretch(1);
        addStar();
        break;
    case ParamInfo::Int:
        if (!p.options.isEmpty()) {
            header->addWidget(caption(p.name, this));
            header->addStretch(1);
            picker_ = new DSPicker(this);
            picker_->setOptions(p.options);
            connect(picker_, &DSPicker::currentIndexChanged, this, [this, eng](int v) { eng->setParameter(info_.id, v); });
            header->addWidget(picker_);
            addStar();
            break;
        } else {
            header->addWidget(caption(p.name, this));
            header->addStretch(1);
            addOscToggle();
            addStar();
            auto* row = new QHBoxLayout;
            row->setSpacing(4);
            minus_ = smallButton("-", this);
            plus_ = smallButton("+", this);
            slider_ = new DSSlider(this);
            slider_->setRange(p.min, p.max);
            slider_->setStep(1);
            value_ = caption({}, this, Theme::textSecondary);
            value_->setMinimumWidth(22);
            value_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            auto setInt = [this, eng](int v) {
                v = std::clamp(v, int(info_.min), int(info_.max));
                info_.intValue = v;
                eng->setParameter(info_.id, v);
                setInfo(info_);
            };
            connect(minus_, &QToolButton::clicked, this, [this, setInt] { setInt(info_.intValue - 1); });
            connect(plus_, &QToolButton::clicked, this, [this, setInt] { setInt(info_.intValue + 1); });
            connect(slider_, &DSSlider::valueChanged, this, [this, setInt](float v) { setInt(int(std::lround(v))); });
            row->addWidget(minus_);
            row->addWidget(slider_, 1);
            row->addWidget(plus_);
            row->addWidget(value_);
            layout->addLayout(row);
            buildOscillator(layout);
        }
        break;
    case ParamInfo::Standard:
        header->addWidget(caption(p.name, this));
        header->addStretch(1);
        value_ = caption({}, this, Theme::textSecondary);
        value_->setStyleSheet(value_->styleSheet() + "font-family: Consolas;");
        header->addWidget(value_);
        addOscToggle();
        addStar();
        slider_ = new DSSlider(this);
        slider_->setRange(p.min, p.max);
        connect(slider_, &DSSlider::valueChanged, this, [this, eng](float v) {
            info_.value = v;
            eng->setParameter(info_.id, v);
            value_->setText(QString::number(v, 'f', 2));
        });
        layout->addWidget(slider_);
        buildOscillator(layout);
        break;
    case ParamInfo::Color:
        header->addWidget(caption(p.name, this));
        header->addStretch(1);
        swatch_ = new QToolButton(this);
        swatch_->setFixedSize(22, 18);
        swatch_->setCursor(Qt::PointingHandCursor);
        connect(swatch_, &QToolButton::clicked, this, [this, eng] {
            const QColor c = QColorDialog::getColor(info_.color, this, info_.name);
            if (!c.isValid()) return;
            info_.color = c;
            eng->setParameterColor(info_.id, c);
            setInfo(info_);
        });
        header->addWidget(swatch_);
        addStar();
        break;
    case ParamInfo::Text:
        header->addWidget(caption(p.name, this));
        header->addStretch(1);
        text_ = new QLineEdit(this);
        text_->setPlaceholderText(p.name);
        connect(text_, &QLineEdit::textEdited, this, [this, eng](const QString& t) { eng->setParameter(info_.id, t); });
        layout->addWidget(text_);
        break;
    default:
        break;
    }
    setInfo(p);
    ParamSelection::instance()->track(this, p.id);
}

QToolButton* CompactParameterControl::smallButton(const QString& text, QWidget* parent) {
    auto* b = new QToolButton(parent);
    b->setText(text);
    b->setFixedSize(16, 16);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(QString("QToolButton { background: %1; color: %2; border: none; border-radius: 3px;"
                             " font-weight: 700; font-size: 8pt; padding: 0; }"
                             "QToolButton:disabled { color: %3; }")
                         .arg(Theme::surfaceHover.name(), Theme::textPrimary.name(), Theme::textTertiary.name()));
    return b;
}

// The inline oscillator: a mini waveform with the shape picker, then
// frequency (0-4 Hz), min and max.
void CompactParameterControl::buildOscillator(QVBoxLayout* layout) {
    if (!info_.hasOscillator) return;
    auto* eng = EngineBridge::instance();
    oscBox_ = new QWidget(this);
    auto* box = new QVBoxLayout(oscBox_);
    box->setContentsMargins(0, 3, 0, 2);
    box->setSpacing(3);
    auto* top = new QHBoxLayout;
    top->setSpacing(6);
    wave_ = new WaveformPreview(oscBox_);
    wave_->setFixedHeight(24);
    wave_->setMinimumWidth(60);
    shape_ = new DSPicker(oscBox_);
    shape_->setOptions(WaveformPreview::shapeNames());
    connect(shape_, &DSPicker::currentIndexChanged, this, [this, eng](int s) {
        info_.oscShape = s;
        eng->setOscillatorWaveShape(info_.id, s);
        wave_->setShape(s);
    });
    top->addWidget(wave_, 1);
    top->addWidget(shape_);
    box->addLayout(top);
    auto slider = [&](const QString& label, float lo, float hi) {
        auto* s = new DSSlider(oscBox_);
        s->setLabel(label);
        s->setShowValue(true);
        s->setDecimals(1);
        s->setRange(lo, hi);
        box->addWidget(s);
        return s;
    };
    freq_ = slider("Freq", 0, 4);
    min_ = slider("Min", info_.min, info_.max);
    max_ = slider("Max", info_.min, info_.max);
    connect(freq_, &DSSlider::valueChanged, this, [this, eng](float f) {
        info_.oscFrequency = f;
        eng->setOscillatorFrequency(info_.id, f);
        wave_->setFrequency(f);
    });
    auto setRange = [this, eng] {
        info_.oscMin = min_->value();
        info_.oscMax = max_->value();
        eng->setOscillatorRange(info_.id, info_.oscMin, info_.oscMax);
        wave_->setOutputRange(info_.oscMin, info_.oscMax);
    };
    connect(min_, &DSSlider::valueChanged, this, setRange);
    connect(max_, &DSSlider::valueChanged, this, setRange);
    layout->addWidget(oscBox_);
}

void CompactParameterControl::updateOscillator() {
    if (oscToggle_) {
        oscToggle_->setIcon(Icons::icon(Icons::Waveform, info_.oscEnabled ? kOscillatorOn : Theme::textSecondary));
        oscToggle_->setToolTip(info_.oscEnabled ? "Stop oscillating" : "Oscillate");
    }
    if (!oscBox_) return;
    oscBox_->setVisible(info_.oscEnabled);
    if (!info_.oscEnabled) return;
    wave_->setOn(true);
    wave_->setShape(info_.oscShape);
    wave_->setFrequency(info_.oscFrequency);
    wave_->setParamRange(info_.min, info_.max);
    wave_->setOutputRange(info_.oscMin, info_.oscMax);
    shape_->setCurrentIndex(info_.oscShape);
    if (!freq_->isDragging()) freq_->setValue(info_.oscFrequency);
    if (!min_->isDragging()) min_->setValue(info_.oscMin);
    if (!max_->isDragging()) max_->setValue(info_.oscMax);
}

void CompactParameterControl::setInfo(const ParamInfo& p) {
    info_ = p;
    // Oscillators and audio drivers own the value while they run.
    const bool driven = p.oscEnabled || p.hasDriver;
    if (toggle_) toggle_->setChecked(p.boolValue);
    if (picker_) picker_->setCurrentIndex(p.intValue);
    if (slider_) {
        if (!slider_->isDragging()) slider_->setValue(p.type == ParamInfo::Int ? float(p.intValue) : p.value);
        slider_->setEnabled(!driven);
    }
    if (value_) value_->setText(p.type == ParamInfo::Int ? QString::number(p.intValue) : QString::number(p.value, 'f', 2));
    if (minus_) minus_->setEnabled(p.intValue > int(p.min));
    if (plus_) plus_->setEnabled(p.intValue < int(p.max));
    if (swatch_)
        swatch_->setStyleSheet(QString("QToolButton { background: %1; border: 1px solid %2; border-radius: 4px; }")
                                   .arg(p.color.name(), Theme::border.name()));
    if (text_ && !text_->hasFocus() && text_->text() != p.stringValue) text_->setText(p.stringValue);
    updateOscillator();
}
