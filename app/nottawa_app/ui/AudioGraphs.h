#pragma once

#include <QColor>
#include <QVector>
#include <QWidget>

// The audio panel's plots, after the macOS ScrollingGraphView, BarChartView
// and waveform canvas: a dark rounded card with faint grid lines.
class AudioGraph : public QWidget {
    Q_OBJECT
public:
    enum Mode { Lines, Bars, Waveform };
    explicit AudioGraph(Mode mode, QWidget* parent = nullptr);

    void setMode(Mode mode) { mode_ = mode; update(); }
    // Lines: one or more series (oldest first), each 0..1.
    void setSeries(const QVector<QVector<float>>& series, const QVector<QColor>& colors);
    // Bars (0..1) or Waveform (-1..1) values.
    void setValues(const QVector<float>& values, const QColor& color);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    Mode mode_;
    QVector<QVector<float>> series_;
    QVector<QColor> colors_;
};

// A ring and BPM readout that pulse on each detected beat.
class BeatIndicator : public QWidget {
    Q_OBJECT
public:
    explicit BeatIndicator(QWidget* parent = nullptr);
    void setBpm(float bpm) { bpm_ = bpm; }
    void setFlash(float flash) { flash_ = flash; update(); }

protected:
    void paintEvent(QPaintEvent*) override;

private:
    float bpm_ = 120, flash_ = 0;
};
