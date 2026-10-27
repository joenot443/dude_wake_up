#pragma once

#include <QTimer>
#include <QWidget>

// A scrolling plot of an oscillator's output, after the macOS
// WaveformPreviewView: the waveform (computed here with the engine's
// WaveformOscillator maths) between dashed min/max lines, over the
// parameter's full range. Shows "Oscillator Off" when disabled.
class WaveformPreview : public QWidget {
    Q_OBJECT
public:
    explicit WaveformPreview(QWidget* parent = nullptr);

    void setShape(int waveShape) { shape_ = waveShape; update(); }
    void setFrequency(float f) { frequency_ = f; update(); }
    void setOutputRange(float min, float max) { min_ = min; max_ = max; update(); }
    void setParamRange(float min, float max) { paramMin_ = min; paramMax_ = max; update(); }
    void setOn(bool on);  // off: "Oscillator Off" placeholder

    QSize sizeHint() const override { return QSize(200, 80); }

    // WaveformOscillator::tick's value in [-1, 1] for one shape.
    static double value(int shape, double freqT, double t, double freq, double randOffset);
    static QStringList shapeNames();

protected:
    void paintEvent(QPaintEvent*) override;
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

private:
    QTimer timer_;
    int    shape_ = 0;
    float  frequency_ = 1, min_ = 0, max_ = 1, paramMin_ = 0, paramMax_ = 1;
    bool   enabled_ = true;
    double randOffset_;
};
