#pragma once

#include <QDialog>
#include <QTimer>

class DSPicker;
class DSSlider;
class DSToggle;
class QLabel;
class QVBoxLayout;

// Settings, after the macOS SettingsView: display (resolution, node hints,
// output-window advance), screenshots (folder, format), MIDI smoothing,
// connected MIDI devices and MIDI assignments (removable).
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

signals:
    void showHintsChanged(bool on);

protected:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

private:
    void refresh();
    void refreshMidi();

    DSPicker*    resolution_;
    DSToggle*    hints_;
    DSToggle*    advance_;
    DSSlider*    fade_;
    QLabel*      fadeLabel_;
    QLabel*      folder_;
    DSPicker*    format_;
    DSToggle*    smoothing_;
    DSSlider*    smoothingMs_;
    QLabel*      smoothingLabel_;
    QVBoxLayout* devices_;
    QVBoxLayout* assignments_;
    QString      midiSignature_;
    QTimer       midiPoll_;
};
