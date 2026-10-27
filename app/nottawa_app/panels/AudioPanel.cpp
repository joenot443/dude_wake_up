#include "AudioPanel.h"

#include "EngineBridge.h"
#include "Icons.h"
#include "Theme.h"
#include "ui/AudioGraphs.h"
#include "ui/Controls.h"

#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <cmath>

namespace {
constexpr int kHistory = 120;  // graph samples (8 s at 15 fps)

QLabel* title(const QString& text, QWidget* parent) {
    auto* l = new QLabel(text, parent);
    l->setStyleSheet(QString("color: %1; font-size: 8pt; font-weight: 600;").arg(Theme::textSecondary.name()));
    return l;
}

QFrame* vline(QWidget* parent) {
    auto* line = new QFrame(parent);
    line->setFixedWidth(1);
    line->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    return line;
}

QToolButton* smallButton(Icons::Kind icon, const QString& tip, QWidget* parent, const QColor& color = Theme::textPrimary) {
    auto* b = new QToolButton(parent);
    b->setIcon(Icons::icon(icon, color));
    b->setIconSize(QSize(11, 11));
    b->setFixedSize(22, 20);
    b->setToolTip(tip);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(QString("QToolButton { background: %1; border: 1px solid %2; border-radius: 4px; }"
                             "QToolButton:hover { background: %3; }")
                         .arg(Theme::surface.name(), Theme::border.name(), Theme::surfaceHover.name()));
    return b;
}

void push(QVector<float>& history, float v) {
    history.push_back(v);
    if (history.size() > kHistory) history.remove(0, int(history.size()) - kHistory);
}

QString formatTime(float seconds) {
    const int total = std::max(0, int(std::isfinite(seconds) ? seconds : 0));
    return QString("%1:%2").arg(total / 60).arg(total % 60, 2, 10, QChar('0'));
}
} // anon

AudioPanel::AudioPanel(QWidget* parent) : QWidget(parent) {
    setObjectName("AudioPanel");
    setStyleSheet(QString("#AudioPanel { background: %1; }").arg(Theme::backgroundSecondary.name()));
    setMinimumHeight(150);
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(buildTopBar());
    auto* line = new QFrame(this);
    line->setFixedHeight(1);
    line->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    layout->addWidget(line);

    auto* columns = new QHBoxLayout;
    columns->setContentsMargins(12, 8, 12, 8);
    columns->setSpacing(12);
    columns->addWidget(buildLoudness(), 1);
    columns->addWidget(vline(this));
    columns->addWidget(buildBpm(), 1);
    columns->addWidget(vline(this));
    columns->addWidget(buildFrequency(), 1);
    layout->addLayout(columns, 1);

    fast_.setInterval(1000 / 15);
    slow_.setInterval(1000 / 3);
    connect(&fast_, &QTimer::timeout, this, &AudioPanel::pollGraphs);
    connect(&slow_, &QTimer::timeout, this, &AudioPanel::pollControls);
}

void AudioPanel::showEvent(QShowEvent*) {
    pollControls();
    fast_.start();
    slow_.start();
}

void AudioPanel::hideEvent(QHideEvent*) {
    fast_.stop();
    slow_.stop();
}

// ---------------------------------------------------------------- top bar

QWidget* AudioPanel::buildTopBar() {
    auto* eng = EngineBridge::instance();
    auto* bar = new QWidget(this);
    auto* row = new QHBoxLayout(bar);
    row->setContentsMargins(12, 6, 12, 6);
    row->setSpacing(12);

    startStop_ = new QPushButton(bar);
    startStop_->setCursor(Qt::PointingHandCursor);
    connect(startStop_, &QPushButton::clicked, this, [this, eng] {
        eng->toggleAudio();
        QTimer::singleShot(150, this, &AudioPanel::pollControls);
    });
    row->addWidget(startStop_);

    source_ = new DSPicker(bar);
    source_->setFixedWidth(180);
    connect(source_, &DSPicker::currentIndexChanged, this, [this, eng](int i) {
        if (i >= 0 && i < sourceIds_.size()) eng->selectAudioSource(sourceIds_[i]);
    });
    row->addWidget(source_);
    status_ = new QLabel(bar);
    row->addWidget(status_);

    // Sample-track playback, when the source is a file.
    fileControls_ = new QWidget(bar);
    auto* file = new QHBoxLayout(fileControls_);
    file->setContentsMargins(0, 0, 0, 0);
    file->setSpacing(8);
    auto* sep = new QFrame(fileControls_);
    sep->setFixedSize(1, 20);
    sep->setStyleSheet(QString("background: %1;").arg(Theme::border.name()));
    file->addWidget(sep);
    track_ = new DSPicker(fileControls_);
    track_->setFixedWidth(160);
    connect(track_, &DSPicker::currentIndexChanged, this, [eng](int i) { eng->selectSampleTrack(i); });
    file->addWidget(track_);
    filePause_ = smallButton(Icons::Pause, "Pause / resume", fileControls_);
    connect(filePause_, &QToolButton::clicked, this, [this, eng] {
        eng->toggleFileAudioPause();
        QTimer::singleShot(100, this, &AudioPanel::pollControls);
    });
    file->addWidget(filePause_);
    auto* speaker = new QLabel(fileControls_);
    speaker->setPixmap(Icons::icon(Icons::Note, Theme::textSecondary).pixmap(12, 12));
    file->addWidget(speaker);
    volume_ = new DSSlider(fileControls_);
    volume_->setRange(0, 1);
    volume_->setFixedWidth(80);
    connect(volume_, &DSSlider::valueChanged, this, [eng](float v) { eng->setFileAudioVolume(v); });
    file->addWidget(volume_);
    fileElapsed_ = new QLabel(fileControls_);
    fileTotal_ = new QLabel(fileControls_);
    for (QLabel* l : {fileElapsed_, fileTotal_})
        l->setStyleSheet(QString("color: %1; font-size: 8pt; font-family: Consolas;").arg(Theme::textSecondary.name()));
    filePosition_ = new DSSlider(fileControls_);
    filePosition_->setRange(0, 1);
    filePosition_->setFixedWidth(120);
    connect(filePosition_, &DSSlider::valueChanged, this, [eng](float p) { eng->setFileAudioPosition(p); });
    file->addWidget(fileElapsed_);
    file->addWidget(filePosition_);
    file->addWidget(fileTotal_);
    row->addWidget(fileControls_);
    row->addStretch(1);
    return bar;
}

// ------------------------------------------------------------------ columns

QWidget* AudioPanel::buildLoudness() {
    auto* eng = EngineBridge::instance();
    auto* col = new QWidget(this);
    auto* layout = new QVBoxLayout(col);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(title("Loudness", col));
    loudness_ = new AudioGraph(AudioGraph::Lines, col);
    layout->addWidget(loudness_, 1);
    loudRelease_ = new DSSlider(col);
    loudRelease_->setLabel("Release");
    loudRelease_->setShowValue(true);
    loudRelease_->setRange(0.01f, 1.0f);
    connect(loudRelease_, &DSSlider::valueChanged, this, [eng](float v) { eng->setLoudnessRelease(v); });
    loudScale_ = new DSSlider(col);
    loudScale_->setLabel("Scale");
    loudScale_->setShowValue(true);
    loudScale_->setDecimals(1);
    loudScale_->setRange(0.1f, 5.0f);
    connect(loudScale_, &DSSlider::valueChanged, this, [eng](float v) { eng->setLoudnessScale(v); });
    layout->addWidget(loudRelease_);
    layout->addWidget(loudScale_);
    return col;
}

QWidget* AudioPanel::buildBpm() {
    auto* eng = EngineBridge::instance();
    auto* col = new QWidget(this);
    auto* layout = new QVBoxLayout(col);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(title("BPM", col));

    beat_ = new BeatIndicator(col);
    auto* overlay = new QHBoxLayout;
    overlay->setContentsMargins(8, 8, 8, 8);
    auto* beatLayout = new QVBoxLayout(beat_);
    beatLayout->setContentsMargins(0, 0, 0, 0);
    bpmLock_ = new QToolButton(beat_);
    bpmPause_ = new QToolButton(beat_);
    for (QToolButton* b : {bpmLock_, bpmPause_}) {
        b->setAutoRaise(true);
        b->setIconSize(QSize(12, 12));
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet("QToolButton { background: transparent; border: none; }");
    }
    connect(bpmLock_, &QToolButton::clicked, this, [this, eng] { eng->setBpmLocked(!bpmLocked_); QTimer::singleShot(100, this, &AudioPanel::pollControls); });
    connect(bpmPause_, &QToolButton::clicked, this, [this, eng] { eng->setBpmEnabled(!bpmEnabled_); QTimer::singleShot(100, this, &AudioPanel::pollControls); });
    overlay->addWidget(bpmLock_);
    overlay->addStretch(1);
    overlay->addWidget(bpmPause_);
    beatLayout->addLayout(overlay);
    beatLayout->addStretch(1);
    layout->addWidget(beat_, 1);

    bpmMode_ = new DSSegmented({"Auto", "Manual", "Link"}, col);
    connect(bpmMode_, &DSSegmented::currentIndexChanged, this, [this, eng](int m) {
        eng->setBpmMode(m);
        manualRow_->setVisible(m == 1);
        tap_->setVisible(m != 2);
    });
    // Tap along: in Auto it corrects the detected tempo and beat, in Manual
    // it sets them. Taps count at the press, not the release.
    auto* modeRow = new QHBoxLayout;
    modeRow->setSpacing(6);
    modeRow->addWidget(bpmMode_, 1);
    tap_ = new QPushButton("Tap", col);
    tap_->setCursor(Qt::PointingHandCursor);
    tap_->setToolTip("Tap along to the beat (T)");
    tap_->setStyleSheet("QPushButton { padding: 1px 8px; font-size: 8pt; }");
    connect(tap_, &QPushButton::pressed, this, [eng] { eng->tapBpm(); });
    modeRow->addStretch(0);
    modeRow->addWidget(tap_);
    layout->addLayout(modeRow);

    manualRow_ = new QWidget(col);
    auto* manual = new QHBoxLayout(manualRow_);
    manual->setContentsMargins(0, 0, 0, 0);
    manual->setSpacing(6);
    auto* minus = smallButton(Icons::Minus, "Slower", manualRow_);
    auto* plus = smallButton(Icons::Plus, "Faster", manualRow_);
    bpmValue_ = new DSSlider(manualRow_);
    bpmValue_->setRange(1, 300);
    connect(minus, &QToolButton::clicked, this, [this, eng] { eng->setBpm(bpm_ - 1); });
    connect(plus, &QToolButton::clicked, this, [this, eng] { eng->setBpm(bpm_ + 1); });
    connect(bpmValue_, &DSSlider::valueChanged, this, [eng](float v) { eng->setBpm(v); });
    manual->addWidget(minus);
    manual->addWidget(bpmValue_, 1);
    manual->addWidget(plus);
    manualRow_->hide();
    layout->addWidget(manualRow_);

    auto* nudgeRow = new QHBoxLayout;
    nudgeRow->setSpacing(4);
    auto* nudgeLabel = new QLabel("Nudge", col);
    nudgeLabel->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textSecondary.name()));
    auto* nudgeDown = smallButton(Icons::Minus, "Nudge earlier", col);
    auto* nudgeUp = smallButton(Icons::Plus, "Nudge later", col);
    nudge_ = new QLabel(col);
    nudge_->setFixedWidth(36);
    nudge_->setAlignment(Qt::AlignCenter);
    nudge_->setStyleSheet(QString("color: %1; font-size: 8pt; font-family: Consolas;").arg(Theme::textPrimary.name()));
    connect(nudgeDown, &QToolButton::clicked, this, [this, eng] { eng->nudgeBpm(-0.1f); QTimer::singleShot(100, this, &AudioPanel::pollControls); });
    connect(nudgeUp, &QToolButton::clicked, this, [this, eng] { eng->nudgeBpm(0.1f); QTimer::singleShot(100, this, &AudioPanel::pollControls); });
    nudgeRow->addWidget(nudgeLabel);
    nudgeRow->addWidget(nudgeDown);
    nudgeRow->addWidget(nudge_);
    nudgeRow->addWidget(nudgeUp);
    nudgeRow->addStretch(1);
    layout->addLayout(nudgeRow);
    return col;
}

QWidget* AudioPanel::buildFrequency() {
    auto* eng = EngineBridge::instance();
    auto* col = new QWidget(this);
    auto* layout = new QVBoxLayout(col);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(title("Frequency", col));
    frequency_ = new AudioGraph(AudioGraph::Lines, col);
    layout->addWidget(frequency_, 1);
    freqView_ = new DSSegmented({"Bands", "Bars", "Waveform"}, col);
    connect(freqView_, &DSSegmented::currentIndexChanged, this, [this](int i) {
        frequency_->setMode(i == 0 ? AudioGraph::Lines : i == 1 ? AudioGraph::Bars : AudioGraph::Waveform);
        pollGraphs();
    });
    layout->addWidget(freqView_);
    auto* controls = new QHBoxLayout;
    controls->setSpacing(8);
    smoothing_ = new DSPicker(col);
    smoothing_->setOptions({"Exponential", "Mov. Avg", "None", "Peak Hold"});
    smoothing_->setFixedWidth(110);
    connect(smoothing_, &DSPicker::currentIndexChanged, this, [eng](int m) { eng->setSmoothingMode(m); });
    freqRelease_ = new DSSlider(col);
    freqRelease_->setLabel("Release");
    freqRelease_->setShowValue(true);
    freqRelease_->setRange(0, 2);
    connect(freqRelease_, &DSSlider::valueChanged, this, [eng](float v) { eng->setFrequencyRelease(v); });
    freqScale_ = new DSSlider(col);
    freqScale_->setLabel("Scale");
    freqScale_->setShowValue(true);
    freqScale_->setRange(0.01f, 2);
    connect(freqScale_, &DSSlider::valueChanged, this, [eng](float v) { eng->setFrequencyScale(v); });
    controls->addWidget(smoothing_);
    controls->addWidget(freqRelease_, 1);
    controls->addWidget(freqScale_, 1);
    layout->addLayout(controls);
    return col;
}

// ------------------------------------------------------------------ polling

void AudioPanel::pollGraphs() {
    const AudioAnalysis a = EngineBridge::instance()->audioAnalysis();
    push(rms_, a.rms);
    push(lows_, a.lows);
    push(mids_, a.mids);
    push(highs_, a.highs);
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (a.beatCount != lastBeatCount_) {
        lastBeatCount_ = a.beatCount;
        beatStartMs_ = now;
    }
    beat_->setBpm(a.bpm);
    beat_->setFlash(float(std::exp(-8.0 * (now - beatStartMs_) / 1000.0)));  // ~350 ms decay
    loudness_->setSeries({rms_}, {QColor(0x30, 0xd1, 0x58)});
    switch (freqView_->currentIndex()) {
    case 0:
        frequency_->setSeries({lows_, mids_, highs_}, {QColor(0xbf, 0x5a, 0xf2), QColor(0xff, 0xd6, 0x0a), QColor(0x30, 0xd1, 0x58)});
        break;
    case 1: frequency_->setValues(a.melSpectrum, QColor(0x64, 0xd2, 0xff)); break;
    default: frequency_->setValues(a.waveform, QColor(0x00, 0xc7, 0xbe)); break;
    }
}

void AudioPanel::pollControls() {
    auto* eng = EngineBridge::instance();
    const AudioAnalysis a = eng->audioAnalysis();
    const FileAudioState file = eng->fileAudioState();

    startStop_->setText(a.audioActive ? "  Stop" : "  Start");
    startStop_->setIcon(Icons::icon(a.audioActive ? Icons::Stop : Icons::Play, a.audioActive ? Theme::destructive : Theme::success));
    const QColor tint = a.audioActive ? Theme::destructive : Theme::success;
    QColor bg = tint;
    bg.setAlphaF(0.2);
    startStop_->setStyleSheet(QString("QPushButton { background: %1; color: %2; border: none; border-radius: 6px; padding: 4px 10px; }")
                                  .arg(bg.name(QColor::HexArgb), tint.name()));

    const QVector<AudioSourceItem> sources = eng->audioSources();
    const QString selected = eng->selectedAudioSource();
    QStringList names;
    sourceIds_.clear();
    int current = -1;
    QString selectedName;
    for (const AudioSourceItem& s : sources) {
        if (s.id == selected) { current = int(names.size()); selectedName = s.name; }
        names << s.name;
        sourceIds_ << s.id;
    }
    source_->setOptions(names);
    source_->setCurrentIndex(current);
    if (a.audioActive) {
        status_->setText(QString("<span style='color:%1'>●</span> <span style='color:%2'>Listening</span> "
                                 "<span style='color:%3'>via %4</span>")
                             .arg(Theme::destructive.name(), Theme::textSecondary.name(), Theme::textTertiary.name(),
                                  selectedName.toHtmlEscaped()));
    } else {
        status_->setText(QString("<span style='color:%1'>● Inactive</span>").arg(Theme::textTertiary.name()));
    }
    status_->setStyleSheet("font-size: 8pt;");

    fileControls_->setVisible(file.isFileSource);
    if (file.isFileSource) {
        const QVector<AudioTrack> tracks = eng->sampleTracks();
        QStringList trackNames;
        for (const AudioTrack& t : tracks) trackNames << t.name;
        track_->setOptions(trackNames);
        track_->setCurrentIndex(file.track);
        track_->setVisible(!tracks.isEmpty());
        filePause_->setIcon(Icons::icon(file.paused ? Icons::Play : Icons::Pause, Theme::textPrimary));
        volume_->setValue(file.volume);
        filePosition_->setValue(file.position);
        fileElapsed_->setText(formatTime(file.position * file.duration));
        fileTotal_->setText(formatTime(file.duration));
    }

    loudRelease_->setValue(a.loudnessRelease);
    loudScale_->setValue(a.loudnessScale);
    bpmLocked_ = a.bpmLocked;
    bpmEnabled_ = a.bpmEnabled;
    bpm_ = a.bpm;
    bpmLock_->setIcon(Icons::icon(a.bpmLocked ? Icons::Lock : Icons::Unlock, QColor(255, 255, 255, a.bpmLocked ? 180 : 100)));
    bpmLock_->setToolTip(a.bpmLocked ? "Unlock BPM" : "Lock BPM");
    bpmPause_->setIcon(Icons::icon(a.bpmEnabled ? Icons::Pause : Icons::Play, QColor(255, 255, 255, 100)));
    bpmPause_->setToolTip(a.bpmEnabled ? "Pause BPM" : "Resume BPM");
    bpmMode_->setVisible(!file.isFileSource);
    bpmMode_->setCurrentIndex(a.bpmMode);
    manualRow_->setVisible(a.bpmMode == 1);
    tap_->setVisible(a.bpmMode != 2);  // Link sets the tempo
    bpmValue_->setValue(a.bpm);
    nudge_->setText(QString::number(a.bpmNudge, 'f', 2));
    smoothing_->setCurrentIndex(a.smoothingMode);
    freqRelease_->setValue(a.frequencyRelease);
    freqScale_->setValue(a.frequencyScale);
}
