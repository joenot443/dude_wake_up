#pragma once

#include <QTimer>
#include <QVector>
#include <QWidget>

class AudioGraph;
class BeatIndicator;
class DSPicker;
class DSSegmented;
class DSSlider;
class QLabel;
class QPushButton;
class QToolButton;

// The bottom audio panel, after the macOS AudioPanelView: a top bar to
// start/stop capture, pick the source (and drive sample-track playback),
// then Loudness, BPM and Frequency columns with live graphs and their
// analysis settings. Graphs poll at 15 fps, controls at 3 fps, only while
// the panel is shown.
class AudioPanel : public QWidget {
    Q_OBJECT
public:
    explicit AudioPanel(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

private:
    QWidget* buildTopBar();
    QWidget* buildLoudness();
    QWidget* buildBpm();
    QWidget* buildFrequency();
    void pollGraphs();
    void pollControls();

    QTimer fast_, slow_;

    // Top bar
    QPushButton* startStop_;
    DSPicker*    source_;
    QStringList  sourceIds_;
    QLabel*      status_;
    QWidget*     fileControls_;
    DSPicker*    track_;
    QToolButton* filePause_;
    DSSlider*    volume_;
    QLabel*      fileElapsed_;
    DSSlider*    filePosition_;
    QLabel*      fileTotal_;

    // Loudness
    AudioGraph* loudness_;
    DSSlider*   loudRelease_;
    DSSlider*   loudScale_;
    // BPM
    BeatIndicator* beat_;
    QToolButton*   bpmLock_;
    QToolButton*   bpmPause_;
    DSSegmented*   bpmMode_;
    QWidget*       manualRow_;
    QPushButton*   tap_ = nullptr;
    DSSlider*      bpmValue_;
    QLabel*        nudge_;
    // Frequency
    AudioGraph*  frequency_;
    DSSegmented* freqView_;
    DSPicker*    smoothing_;
    DSSlider*    freqRelease_;
    DSSlider*    freqScale_;

    // Histories (oldest first, at most kHistory each)
    QVector<float> rms_, lows_, mids_, highs_;
    quint32 lastBeatCount_ = 0;
    qint64  beatStartMs_ = 0;
    bool    bpmLocked_ = false, bpmEnabled_ = true;
    float   bpm_ = 120;
};
