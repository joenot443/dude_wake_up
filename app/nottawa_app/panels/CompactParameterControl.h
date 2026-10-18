#pragma once

#include "EngineBridge.h"

#include <QWidget>

class DSPicker;
class DSSlider;
class DSToggle;
class QLabel;
class QLineEdit;
class QToolButton;
class WaveformPreview;

// A stage-strip parameter control, after the macOS CompactParameterControl:
// a switch, an options picker, an int slider with -/+ steppers, a float
// slider, a colour swatch or a text field, with an oscillator toggle (and,
// while it runs, a mini waveform with shape / frequency / min / max) and a
// star that removes the parameter from favourites. Clicking it selects the
// parameter for keyboard stepping.
class CompactParameterControl : public QWidget {
    Q_OBJECT
public:
    CompactParameterControl(const ParamInfo& p, QWidget* parent);

    const QString& paramId() const { return info_.id; }
    // Takes fresh engine state; controls being dragged keep theirs.
    void setInfo(const ParamInfo& p);

signals:
    // Favourites changed (the strip's list of shown parameters may shrink).
    void favoritesChanged();

private:
    QToolButton* smallButton(const QString& text, QWidget* parent);
    void buildOscillator(class QVBoxLayout* layout);
    void updateOscillator();

    ParamInfo    info_;
    DSToggle*    toggle_ = nullptr;
    DSPicker*    picker_ = nullptr;
    DSSlider*    slider_ = nullptr;
    QLabel*      value_ = nullptr;
    QToolButton* minus_ = nullptr;
    QToolButton* plus_ = nullptr;
    QToolButton* swatch_ = nullptr;
    QLineEdit*   text_ = nullptr;
    QToolButton* oscToggle_ = nullptr;
    QWidget*     oscBox_ = nullptr;
    WaveformPreview* wave_ = nullptr;
    DSPicker*    shape_ = nullptr;
    DSSlider*    freq_ = nullptr;
    DSSlider*    min_ = nullptr;
    DSSlider*    max_ = nullptr;
};
