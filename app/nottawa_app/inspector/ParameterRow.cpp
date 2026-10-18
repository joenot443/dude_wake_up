#include "ParameterRow.h"

#include "Icons.h"
#include "Theme.h"
#include "ui/Controls.h"
#include "ui/WaveformPreview.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

namespace {

const QColor kMidi(0xa8, 0x55, 0xf7);  // the macOS panels' purple
const QColor kDriver(0xf9, 0x73, 0x16); // ... and orange

QToolButton* smallButton(Icons::Kind icon, const QColor& color, const QString& tip, QWidget* parent) {
    auto* b = new QToolButton(parent);
    b->setIcon(Icons::icon(icon, color));
    b->setIconSize(QSize(12, 12));
    b->setFixedSize(18, 18);
    b->setAutoRaise(true);
    b->setToolTip(tip);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(QString("QToolButton { border: none; border-radius: 4px; background: transparent; }"
                             "QToolButton:hover { background: %1; }").arg(Theme::surfaceHover.name()));
    return b;
}

QLabel* caption(const QString& text, QWidget* parent, const QColor& color = Theme::textPrimary) {
    auto* l = new QLabel(text, parent);
    l->setStyleSheet(QString("color: %1; font-size: 8pt; background: transparent;").arg(color.name()));
    return l;
}

void paintTintedBox(QWidget* w, const QColor& tint, qreal alpha, bool border = false, const QColor& borderColor = {}) {
    QPainter p(w);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath box;
    box.addRoundedRect(QRectF(w->rect()).adjusted(0.5, 0.5, -0.5, -0.5), border ? 8 : 6, border ? 8 : 6);
    QColor fill = tint;
    fill.setAlphaF(alpha);
    p.fillPath(box, fill);
    if (border) { p.setPen(QPen(borderColor, 1)); p.drawPath(box); }
}

QString formatMidi(const QString& raw) {
    // portNum:portName:status:channel:control:$  ->  "CC 7 · Ch 1"
    const QStringList parts = raw.split(':');
    if (parts.size() < 5) return raw;
    return QString("CC %1 · Ch %2").arg(parts[4], parts[3]);
}

} // anon

// -------------------------------------------------------------- ValueField

ValueField::ValueField(QWidget* parent) : QLineEdit(parent) {
    setFixedWidth(56);
    setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setStyleSheet(QString("QLineEdit { background: transparent; border: 1px solid transparent; border-radius: 3px;"
                          " color: %1; font-family: Consolas; font-size: 8pt; padding: 0 3px; }"
                          "QLineEdit:hover { background: %2; }"
                          "QLineEdit:focus { background: %2; border-color: %3; color: %4; }")
                      .arg(Theme::textSecondary.name(), Theme::backgroundTertiary.name(), Theme::accent.name(),
                           Theme::textPrimary.name()));
}

void ValueField::setValue(float value) {
    value_ = value;
    if (!hasFocus()) setText(QString::number(value, 'f', 2));
}

void ValueField::commit() {
    bool ok = false;
    const float v = text().toFloat(&ok);
    if (ok && v != value_) { value_ = v; emit committed(v); }
    setText(QString::number(value_, 'f', 2));
}

void ValueField::focusOutEvent(QFocusEvent* e) {
    QLineEdit::focusOutEvent(e);
    commit();
}

void ValueField::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) { commit(); clearFocus(); return; }
    if (e->key() == Qt::Key_Escape) { setText(QString::number(value_, 'f', 2)); clearFocus(); return; }
    QLineEdit::keyPressEvent(e);
}

// --------------------------------------------------------- OscillatorPanel

OscillatorPanel::OscillatorPanel(const ParamInfo& p, QWidget* parent) : QWidget(parent), paramId_(p.id) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);
    wave_ = new WaveformPreview(this);
    layout->addWidget(wave_);
    auto* row = new QHBoxLayout;
    toggle_ = new DSToggle("Oscillator", this);
    toggle_->setSmall(true);
    connect(toggle_, &DSToggle::toggled, this, [this](bool on) {
        EngineBridge::instance()->setOscillatorEnabled(paramId_, on);
        setOn(on);
    });
    shape_ = new DSPicker(this);
    shape_->setOptions(WaveformPreview::shapeNames());
    shape_->setFixedWidth(100);
    connect(shape_, &DSPicker::currentIndexChanged, this, [this](int s) {
        EngineBridge::instance()->setOscillatorWaveShape(paramId_, s);
        wave_->setShape(s);
    });
    row->addWidget(toggle_);
    row->addStretch(1);
    row->addWidget(shape_);
    layout->addLayout(row);
    frequency_ = new DSSlider(this);
    frequency_->setLabel("Frequency");
    frequency_->setShowValue(true);
    frequency_->setRange(0, 12);
    connect(frequency_, &DSSlider::valueChanged, this, [this](float f) {
        EngineBridge::instance()->setOscillatorFrequency(paramId_, f);
        wave_->setFrequency(f);
    });
    min_ = new DSSlider(this);
    min_->setLabel("Min");
    min_->setShowValue(true);
    max_ = new DSSlider(this);
    max_->setLabel("Max");
    max_->setShowValue(true);
    auto sendRange = [this] {
        EngineBridge::instance()->setOscillatorRange(paramId_, min_->value(), max_->value());
        wave_->setOutputRange(min_->value(), max_->value());
    };
    connect(min_, &DSSlider::valueChanged, this, sendRange);
    connect(max_, &DSSlider::valueChanged, this, sendRange);
    layout->addWidget(frequency_);
    layout->addWidget(min_);
    layout->addWidget(max_);
    setInfo(p);
}

void OscillatorPanel::setInfo(const ParamInfo& p) {
    toggle_->setChecked(p.oscEnabled);
    shape_->setCurrentIndex(p.oscShape);
    frequency_->setValue(p.oscFrequency);
    min_->setRange(p.min, p.max);
    max_->setRange(p.min, p.max);
    min_->setValue(p.oscMin);
    max_->setValue(p.oscMax);
    wave_->setShape(p.oscShape);
    wave_->setFrequency(frequency_->value());
    wave_->setOutputRange(min_->value(), max_->value());
    wave_->setParamRange(p.min, p.max);
    if (p.oscEnabled != on_) setOn(p.oscEnabled);
}

void OscillatorPanel::setOn(bool on) {
    on_ = on;
    wave_->setOn(on);
    for (QWidget* w : std::initializer_list<QWidget*>{shape_, frequency_, min_, max_}) w->setVisible(on);
    update();
}

void OscillatorPanel::paintEvent(QPaintEvent*) {
    QColor border = on_ ? Theme::accent : Theme::borderSubtle;
    if (on_) border.setAlphaF(0.3);
    paintTintedBox(this, Theme::surface, 1.0, true, border);
}

// ------------------------------------------------------------- DriverPanel

DriverPanel::DriverPanel(const ParamInfo& p, QWidget* parent) : QWidget(parent), paramId_(p.id) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    active_ = new QWidget(this);
    auto* activeLayout = new QVBoxLayout(active_);
    activeLayout->setContentsMargins(0, 0, 0, 0);
    activeLayout->setSpacing(6);
    auto* header = new QHBoxLayout;
    auto* icon = new QLabel(active_);
    icon->setPixmap(Icons::icon(Icons::Waveform, kDriver).pixmap(12, 12));
    name_ = caption({}, active_);
    name_->setStyleSheet(name_->styleSheet() + " font-weight: 600;");
    auto* remove = smallButton(Icons::Close, Theme::textSecondary, "Remove audio driver", active_);
    connect(remove, &QToolButton::clicked, this, [this] { EngineBridge::instance()->removeParameterDriver(paramId_); });
    header->addWidget(icon);
    header->addWidget(name_);
    header->addStretch(1);
    header->addWidget(remove);
    activeLayout->addLayout(header);
    shift_ = new DSSlider(active_);
    shift_->setLabel("Shift");
    shift_->setShowValue(true);
    shift_->setRange(0, 1);
    connect(shift_, &DSSlider::valueChanged, this, [this](float v) { EngineBridge::instance()->setParameterDriverShift(paramId_, v); });
    scale_ = new DSSlider(active_);
    scale_->setLabel("Scale");
    scale_->setShowValue(true);
    scale_->setRange(0, 2);
    connect(scale_, &DSSlider::valueChanged, this, [this](float v) { EngineBridge::instance()->setParameterDriverScale(paramId_, v); });
    activeLayout->addWidget(shift_);
    activeLayout->addWidget(scale_);
    layout->addWidget(active_);

    inactive_ = new QWidget(this);
    auto* row = new QHBoxLayout(inactive_);
    row->setContentsMargins(0, 0, 0, 0);
    row->addWidget(caption("Audio Driver", inactive_));
    row->addStretch(1);
    noSource_ = caption("No audio source", inactive_, Theme::textSecondary);
    assign_ = new QToolButton(inactive_);
    assign_->setText("Assign");
    assign_->setIcon(Icons::icon(Icons::Plus, Theme::accent));
    assign_->setIconSize(QSize(10, 10));
    assign_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    assign_->setAutoRaise(true);
    assign_->setCursor(Qt::PointingHandCursor);
    assign_->setStyleSheet(QString("QToolButton { color: %1; font-size: 8pt; border: none; background: transparent; }")
                               .arg(Theme::accent.name()));
    connect(assign_, &QToolButton::clicked, this, [this] {
        QMenu menu(this);
        for (const QString& feature : EngineBridge::instance()->audioDriverNames())
            menu.addAction(feature, this, [this, feature] { EngineBridge::instance()->setParameterDriver(paramId_, feature); });
        menu.exec(assign_->mapToGlobal(QPoint(0, assign_->height())));
    });
    row->addWidget(noSource_);
    row->addWidget(assign_);
    layout->addWidget(inactive_);

    const bool haveDrivers = !EngineBridge::instance()->audioDriverNames().isEmpty();
    noSource_->setVisible(!haveDrivers);
    assign_->setVisible(haveDrivers);
    setInfo(p);
}

void DriverPanel::setInfo(const ParamInfo& p) {
    active_->setVisible(p.hasDriver);
    inactive_->setVisible(!p.hasDriver);
    name_->setText(p.driverName);
    shift_->setValue(p.driverShift);
    scale_->setValue(p.driverScale);
}

void DriverPanel::paintEvent(QPaintEvent*) { paintTintedBox(this, kDriver, 0.06); }

// --------------------------------------------------------------- MidiPanel

MidiPanel::MidiPanel(const ParamInfo& p, QWidget* parent) : QWidget(parent), paramId_(p.id) {
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(8, 8, 8, 8);
    text_ = caption({}, this);
    action_ = new QToolButton(this);
    action_->setAutoRaise(true);
    action_->setCursor(Qt::PointingHandCursor);
    action_->setStyleSheet(QString("QToolButton { color: %1; font-size: 8pt; border: none; background: transparent; }")
                               .arg(kMidi.name()));
    connect(action_, &QToolButton::clicked, this, [this] {
        if (learning_) {
            setLearning(false);
        } else if (bound_) {
            EngineBridge::instance()->removeMidiBinding(paramId_);
            bound_ = false;
            refreshState();
        } else {
            setLearning(true);
        }
    });
    row->addWidget(text_);
    row->addStretch(1);
    row->addWidget(action_);
    connect(EngineBridge::instance(), &EngineBridge::midiLearned, this, [this](const QString& id, const QString& desc) {
        if (id != paramId_) return;
        learning_ = false;
        bound_ = true;
        descriptor_ = desc;
        refreshState();
    });
    bound_ = p.hasMidi;
    descriptor_ = p.midiDescriptor;
    refreshState();
}

MidiPanel::~MidiPanel() { if (learning_) EngineBridge::instance()->stopMidiLearning(); }

void MidiPanel::setInfo(const ParamInfo& p) {
    if (p.hasMidi == bound_ && p.midiDescriptor == descriptor_) return;
    bound_ = p.hasMidi;
    descriptor_ = p.midiDescriptor;
    if (bound_) learning_ = false;
    refreshState();
}

void MidiPanel::setLearning(bool on) {
    if (on == learning_) return;
    learning_ = on;
    if (on) EngineBridge::instance()->beginMidiLearning(paramId_);
    else EngineBridge::instance()->stopMidiLearning();
    refreshState();
}

// Closing the panel cancels a learn in progress, as on macOS.
void MidiPanel::hideEvent(QHideEvent*) { setLearning(false); }

void MidiPanel::refreshState() {
    if (learning_) {
        text_->setText(QString("<span style='color:%1'>Listening for MIDI...</span>").arg(kMidi.name()));
        action_->setIcon(Icons::icon(Icons::Close, Theme::textSecondary));
        action_->setText({});
        action_->setToolTip("Stop listening");
        action_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    } else if (bound_) {
        text_->setText(QString("<b>%1</b>").arg(formatMidi(descriptor_).toHtmlEscaped()));
        action_->setIcon(Icons::icon(Icons::Close, Theme::textSecondary));
        action_->setText({});
        action_->setToolTip("Remove MIDI binding");
        action_->setToolButtonStyle(Qt::ToolButtonIconOnly);
    } else {
        text_->setText("MIDI");
        action_->setIcon(Icons::icon(Icons::Keys, kMidi));
        action_->setText("Learn");
        action_->setToolTip("Move a MIDI control to bind it");
        action_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
}

void MidiPanel::paintEvent(QPaintEvent*) { paintTintedBox(this, kMidi, 0.06); }

// ------------------------------------------------------------ ParameterRow

ParameterRow::ParameterRow(const ParamInfo& p, Mode mode, const QString& axisLabel, QWidget* parent)
    : QWidget(parent), info_(p), mode_(mode), axisLabel_(axisLabel) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(4);
    buildHeader(layout);

    const bool numeric = p.type == ParamInfo::Standard || (p.type == ParamInfo::Int && p.options.isEmpty());
    if (numeric) {
        if (p.hasOscillator) {
            oscPanel_ = new OscillatorPanel(p, this);
            oscPanel_->hide();
            layout->addWidget(oscPanel_);
        }
        driverPanel_ = new DriverPanel(p, this);
        driverPanel_->setVisible(p.hasDriver);
        layout->addWidget(driverPanel_);
        midiPanel_ = new MidiPanel(p, this);
        midiPanel_->setVisible(p.hasMidi);
        layout->addWidget(midiPanel_);
    }
    updateButtons();
    updateDrivenState();
}

void ParameterRow::buildHeader(QVBoxLayout* layout) {
    const ParamInfo& p = info_;
    auto* header = new QHBoxLayout;
    header->setSpacing(2);

    if (mode_ == Axis) {
        name_ = caption({}, this, Theme::textTertiary);
        name_->setText(QString("%1: %2").arg(axisLabel_).arg(p.value, 0, 'f', 2));
        header->addWidget(name_);
        header->addStretch(1);
        addReactivityButtons(header);
        layout->addLayout(header);
        return;
    }

    switch (p.type) {
    case ParamInfo::Bool:
        toggle_ = new DSToggle(p.name, this);
        toggle_->setSmall(true);
        toggle_->setChecked(p.boolValue);
        connect(toggle_, &DSToggle::toggled, this, [this](bool on) {
            EngineBridge::instance()->setParameter(info_.id, on);
            emit graphMayChange();
        });
        header->addWidget(toggle_);
        header->addStretch(1);
        addResetFavorite(header);
        layout->addLayout(header);
        return;
    case ParamInfo::Color: {
        header->addWidget(caption(p.name, this));
        header->addStretch(1);
        swatch_ = new QToolButton(this);
        swatch_->setFixedSize(34, 18);
        swatch_->setCursor(Qt::PointingHandCursor);
        swatch_->setToolTip("Choose colour");
        connect(swatch_, &QToolButton::clicked, this, [this] {
            auto* dialog = new QColorDialog(info_.color, this);
            dialog->setOption(QColorDialog::ShowAlphaChannel);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            const QColor original = info_.color;
            connect(dialog, &QColorDialog::currentColorChanged, this, [this](const QColor& c) {
                EngineBridge::instance()->setParameterColor(info_.id, c);
            });
            connect(dialog, &QColorDialog::rejected, this, [this, original] {
                EngineBridge::instance()->setParameterColor(info_.id, original);
            });
            dialog->open();
        });
        header->addWidget(swatch_);
        addResetFavorite(header);
        layout->addLayout(header);
        setInfo(p);
        return;
    }
    case ParamInfo::Text:
        layout->addWidget(caption(p.name, this));
        text_ = new QLineEdit(p.stringValue, this);
        connect(text_, &QLineEdit::textEdited, this, [this](const QString& t) {
            EngineBridge::instance()->setParameter(info_.id, t);
        });
        layout->addWidget(text_);
        return;
    case ParamInfo::Int:
        if (!p.options.isEmpty()) {
            header->addWidget(caption(p.name, this));
            header->addStretch(1);
            addResetFavorite(header);
            layout->addLayout(header);
            picker_ = new DSPicker(this);
            picker_->setOptions(p.options);
            picker_->setCurrentIndex(p.intValue);
            connect(picker_, &DSPicker::currentIndexChanged, this, [this](int i) {
                EngineBridge::instance()->setParameter(info_.id, i);
                emit graphMayChange();
            });
            auto* row = new QHBoxLayout;
            row->addWidget(picker_);
            row->addStretch(1);
            layout->addLayout(row);
            return;
        }
        {
            name_ = caption(p.name, this);
            header->addWidget(name_);
            header->addStretch(1);
            addReactivityButtons(header);
            addResetFavorite(header);
            layout->addLayout(header);
            auto* row = new QHBoxLayout;
            row->setSpacing(6);
            auto* minus = smallButton(Icons::Minus, Theme::textPrimary, "Decrease", this);
            auto* plus = smallButton(Icons::Plus, Theme::textPrimary, "Increase", this);
            for (QToolButton* b : {minus, plus})
                b->setStyleSheet(QString("QToolButton { border: none; border-radius: 4px; background: %1; }"
                                         "QToolButton:hover { background: %2; }")
                                     .arg(Theme::surface.name(), Theme::surfaceHover.name()));
            slider_ = new DSSlider(this);
            slider_->setRange(p.min, p.max);
            slider_->setStep(1);
            slider_->setValue(p.intValue);
            intValue_ = caption(QString::number(p.intValue), this, Theme::textSecondary);
            intValue_->setMinimumWidth(28);
            intValue_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            auto setInt = [this](int v) {
                v = std::clamp(v, int(info_.min), int(info_.max));
                info_.intValue = v;
                slider_->setValue(v);
                intValue_->setText(QString::number(v));
                EngineBridge::instance()->setParameter(info_.id, v);
            };
            connect(slider_, &DSSlider::valueChanged, this, [setInt](float v) { setInt(int(std::lround(v))); });
            connect(minus, &QToolButton::clicked, this, [this, setInt] { setInt(info_.intValue - 1); });
            connect(plus, &QToolButton::clicked, this, [this, setInt] { setInt(info_.intValue + 1); });
            row->addWidget(minus);
            row->addWidget(slider_, 1);
            row->addWidget(plus);
            row->addWidget(intValue_);
            layout->addLayout(row);
        }
        return;
    default: {  // Standard
        name_ = caption(p.name, this);
        header->addWidget(name_);
        header->addStretch(1);
        valueField_ = new ValueField(this);
        valueField_->setValue(p.value);
        connect(valueField_, &ValueField::committed, this, [this](float v) {
            v = std::clamp(v, std::min(info_.min, info_.max), std::max(info_.min, info_.max));
            slider_->setValue(v);
            EngineBridge::instance()->setParameter(info_.id, v);
        });
        header->addWidget(valueField_);
        addReactivityButtons(header);
        addResetFavorite(header);
        layout->addLayout(header);
        slider_ = new DSSlider(this);
        slider_->setRange(p.min, p.max);
        slider_->setValue(p.value);
        connect(slider_, &DSSlider::valueChanged, this, [this](float v) {
            valueField_->setValue(v);
            EngineBridge::instance()->setParameter(info_.id, v);
        });
        layout->addWidget(slider_);
        return;
    }
    }
}

void ParameterRow::addReactivityButtons(QHBoxLayout* row) {
    if (info_.hasOscillator) {
        oscButton_ = smallButton(Icons::Waveform, Theme::textSecondary, "Oscillator", this);
        connect(oscButton_, &QToolButton::clicked, this, [this] {
            if (!oscPanel_) return;
            const bool opening = oscPanel_->isHidden();
            togglePanel(oscPanel_);
            if (opening && !info_.oscEnabled) {
                // Opening the panel turns the oscillator on, as on macOS.
                EngineBridge::instance()->setOscillatorEnabled(info_.id, true);
                info_.oscEnabled = true;
                oscPanel_->setInfo(info_);
                updateButtons();
                updateDrivenState();
            }
        });
        row->addWidget(oscButton_);
    }
    driverButton_ = smallButton(Icons::Note, Theme::textSecondary, "Audio driver", this);
    connect(driverButton_, &QToolButton::clicked, this, [this] { togglePanel(driverPanel_); });
    row->addWidget(driverButton_);
    midiButton_ = smallButton(Icons::Keys, Theme::textSecondary, "MIDI", this);
    connect(midiButton_, &QToolButton::clicked, this, [this] {
        const bool opening = midiPanel_->isHidden();
        togglePanel(midiPanel_);
        if (opening && !info_.hasMidi) midiPanel_->setLearning(true);
    });
    row->addWidget(midiButton_);
}

void ParameterRow::addResetFavorite(QHBoxLayout* row) {
    auto* reset = smallButton(Icons::Reset, Theme::textSecondary, "Reset to default", this);
    connect(reset, &QToolButton::clicked, this, [this] {
        EngineBridge::instance()->resetParameter(info_.id);
        emit graphMayChange();
    });
    row->addWidget(reset);
    favButton_ = smallButton(Icons::Star, Theme::textSecondary, "Favorite", this);
    connect(favButton_, &QToolButton::clicked, this, [this] {
        EngineBridge::instance()->toggleParameterFavorite(info_.id);
        info_.favorite = !info_.favorite;
        updateButtons();
    });
    row->addWidget(favButton_);
}

// One reactivity panel open at a time, as on macOS.
void ParameterRow::togglePanel(QWidget* panel) {
    if (!panel) return;
    const bool show = panel->isHidden();
    for (QWidget* w : std::initializer_list<QWidget*>{oscPanel_, driverPanel_, midiPanel_})
        if (w) w->setVisible(show && w == panel);
}

void ParameterRow::updateButtons() {
    if (oscButton_) oscButton_->setIcon(Icons::icon(Icons::Waveform, info_.oscEnabled ? Theme::accent : Theme::textSecondary));
    if (driverButton_) driverButton_->setIcon(Icons::icon(Icons::Note, info_.hasDriver ? Theme::warning : Theme::textSecondary));
    if (midiButton_) midiButton_->setIcon(Icons::icon(Icons::Keys, info_.hasMidi ? kMidi : Theme::textSecondary));
    if (favButton_)
        favButton_->setIcon(Icons::icon(info_.favorite ? Icons::StarFilled : Icons::Star,
                                        info_.favorite ? Theme::warning : Theme::textSecondary));
}

// Oscillator- or audio-driven values can't be dragged.
void ParameterRow::updateDrivenState() {
    if (!slider_) return;
    slider_->setEnabled(!info_.driven());
    if (valueField_) valueField_->setReadOnly(info_.driven());
}

void ParameterRow::setInfo(const ParamInfo& p) {
    const bool reactivityChanged = p.oscEnabled != info_.oscEnabled || p.hasDriver != info_.hasDriver
                                   || p.hasMidi != info_.hasMidi || p.favorite != info_.favorite;
    info_ = p;
    if (mode_ == Axis && name_) name_->setText(QString("%1: %2").arg(axisLabel_).arg(p.value, 0, 'f', 2));
    if (valueField_) valueField_->setValue(p.value);
    if (slider_) slider_->setValue(p.type == ParamInfo::Int ? float(p.intValue) : p.value);
    if (intValue_) intValue_->setText(QString::number(p.intValue));
    if (toggle_) toggle_->setChecked(p.boolValue);
    if (picker_) picker_->setCurrentIndex(p.intValue);
    if (text_ && !text_->hasFocus() && text_->text() != p.stringValue) text_->setText(p.stringValue);
    if (swatch_)
        swatch_->setStyleSheet(QString("QToolButton { background: %1; border: 1px solid %2; border-radius: 4px; }")
                                   .arg(p.color.name(QColor::HexArgb), Theme::border.name()));
    if (oscPanel_) oscPanel_->setInfo(p);
    if (driverPanel_) driverPanel_->setInfo(p);
    if (midiPanel_) midiPanel_->setInfo(p);
    if (reactivityChanged) { updateButtons(); updateDrivenState(); }
}
