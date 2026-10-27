#pragma once

#include "EngineBridge.h"

#include <QLineEdit>
#include <QWidget>

class DSPicker;
class DSSlider;
class DSToggle;
class QLabel;
class QToolButton;
class QVBoxLayout;
class WaveformPreview;

// A flat numeric field: shows a value, edits on click, commits on Enter or
// focus loss (Esc cancels). Engine updates are ignored while editing.
class ValueField : public QLineEdit {
    Q_OBJECT
public:
    explicit ValueField(QWidget* parent = nullptr);
    void setValue(float value);
signals:
    void committed(float value);
protected:
    void focusOutEvent(QFocusEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
private:
    void commit();
    float value_ = 0;
};

// The oscillator card under a parameter: waveform, on/off, shape,
// frequency and output range.
class OscillatorPanel : public QWidget {
    Q_OBJECT
public:
    OscillatorPanel(const ParamInfo& p, QWidget* parent);
    void setInfo(const ParamInfo& p);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    void setOn(bool on);
    QString          paramId_;
    WaveformPreview* wave_;
    DSToggle*        toggle_;
    DSPicker*        shape_;
    DSSlider*        frequency_;
    DSSlider*        min_;
    DSSlider*        max_;
    bool             on_ = false;
};

// Assigns an audio feature to drive a parameter, with shift and scale.
class DriverPanel : public QWidget {
    Q_OBJECT
public:
    DriverPanel(const ParamInfo& p, QWidget* parent);
    void setInfo(const ParamInfo& p);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    QString      paramId_;
    QWidget*     active_;
    QWidget*     inactive_;
    QLabel*      name_;
    DSSlider*    shift_;
    DSSlider*    scale_;
    QToolButton* assign_;
    QLabel*      noSource_;
};

// MIDI learn / binding for a parameter. Opening it starts learning.
class MidiPanel : public QWidget {
    Q_OBJECT
public:
    MidiPanel(const ParamInfo& p, QWidget* parent);
    ~MidiPanel() override;
    void setInfo(const ParamInfo& p);
    void setLearning(bool on);
protected:
    void paintEvent(QPaintEvent*) override;
    void hideEvent(QHideEvent*) override;
private:
    void refreshState();
    QString      paramId_;
    QLabel*      text_;
    QToolButton* action_;
    bool         learning_ = false;
    bool         bound_ = false;
    QString      descriptor_;
};

// One parameter in the inspector, after the macOS ParameterControlView:
// the control for its type, reset and favourite buttons, and for numeric
// parameters toggles that expand oscillator, audio-driver and MIDI panels.
// Axis mode (for a position pad's X/Y) shows "X: 0.50" and those toggles
// without a slider.
class ParameterRow : public QWidget {
    Q_OBJECT
public:
    enum Mode { Full, Axis };
    ParameterRow(const ParamInfo& p, Mode mode = Full, const QString& axisLabel = {}, QWidget* parent = nullptr);

    const QString& paramId() const { return info_.id; }
    const ParamInfo& info() const { return info_; }
    // Starts typing a value (keyboard: a digit with the row selected).
    bool beginEdit(const QString& initialText);
    // Takes fresh engine state; skipped for controls being edited.
    void setInfo(const ParamInfo& p);

signals:
    // A change that may alter the graph (bools can add/remove pins).
    void graphMayChange();

private:
    void buildHeader(QVBoxLayout* layout);
    void addReactivityButtons(class QHBoxLayout* row);
    void addResetFavorite(class QHBoxLayout* row);
    void togglePanel(QWidget* panel);
    void updateButtons();
    void updateDrivenState();

    ParamInfo    info_;
    Mode         mode_;
    QString      axisLabel_;
    QLabel*      name_ = nullptr;
    ValueField*  valueField_ = nullptr;
    DSSlider*    slider_ = nullptr;
    QLabel*      intValue_ = nullptr;
    DSToggle*    toggle_ = nullptr;
    DSPicker*    picker_ = nullptr;
    QToolButton* swatch_ = nullptr;
    QLineEdit*   text_ = nullptr;
    QToolButton* oscButton_ = nullptr;
    QToolButton* driverButton_ = nullptr;
    QToolButton* midiButton_ = nullptr;
    QToolButton* favButton_ = nullptr;
    OscillatorPanel* oscPanel_ = nullptr;
    DriverPanel* driverPanel_ = nullptr;
    MidiPanel*   midiPanel_ = nullptr;
};
