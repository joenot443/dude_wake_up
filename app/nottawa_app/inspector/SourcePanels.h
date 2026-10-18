#pragma once

#include "EngineBridge.h"

#include <QWidget>

class DSPicker;
class DSSlider;
class PositionPad;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;

// Text source controls, after the macOS TextSourceControlsView: the text,
// font, size, and a position pad with a Center button.
class TextSourcePanel : public QWidget {
    Q_OBJECT
public:
    TextSourcePanel(const QString& sourceId, const TextSourceState& state, QWidget* parent);
    void setState(const TextSourceState& state);  // from the refresh; skips fields being edited

private:
    QString         sourceId_;
    QPlainTextEdit* text_;
    DSPicker*       font_;
    DSSlider*       size_;
    QLabel*         coords_;
    PositionPad*    pad_;
};

// Video file playback, after the macOS FileSourceControlsView: play /
// pause, a position scrubber with times, and speed. (Volume and the frame
// cache are left out: the Windows player has neither yet.)
class FileSourcePanel : public QWidget {
    Q_OBJECT
public:
    FileSourcePanel(const QString& sourceId, const FileSourceState& state, QWidget* parent);
    void setState(const FileSourceState& state);

private:
    QString      sourceId_;
    FileSourceState state_;
    class QToolButton* play_;
    DSSlider*    position_;
    QLabel*      elapsed_;
    QLabel*      total_;
    QLabel*      speedLabel_;
    DSSlider*    speed_;
};

// Icon source picker, after the macOS IconSourceControlsView: category
// pills, search, and a grid of icon thumbnails (loaded off-thread).
class IconSourcePanel : public QWidget {
    Q_OBJECT
public:
    IconSourcePanel(const QString& sourceId, const IconSourceState& state, QWidget* parent);
    void setSelected(int index);

private:
    void rebuildGrid();

    QString         sourceId_;
    IconSourceState state_;
    QString         category_;  // empty: all
    QHBoxLayout*    pills_;
    QLineEdit*      search_;
    QListWidget*    grid_;
};
