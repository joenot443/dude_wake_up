#pragma once

#include <QDialog>
#include <QHash>
#include <QTimer>

#include <functional>

struct SettingInfo;
class QFrame;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QScrollArea;
class QVBoxLayout;

// Settings, after the macOS SettingsView: a sidebar of sections and a search
// field, the rows built from the engine's settings schema (Prefs; see
// docs/SETTINGS.md). Rows follow changes made anywhere (menus, the Mac
// app's shared settings file, resets). MIDI adds the connected devices and
// the MIDI assignments (removable).
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

signals:
    // The user chose to restart to apply a new theme.
    void restartRequested();

protected:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

private:
    void     build();                     // the selected section, or the search's matches
    QWidget* rowFor(const SettingInfo& s, QWidget* parent);
    void     perform(const QString& action);
    void     settingChanged(const QString& id);
    void     updateRestartBanner();
    void     refreshMidi();

    QLineEdit*   search_;
    QListWidget* sections_;
    QScrollArea* scroll_;
    QFrame*      restartBanner_;
    QStringList  sectionNames_;
    // For the rows on show: each one's refresh, and the rows a setting enables.
    QHash<QString, std::function<void()>> refreshers_;
    QHash<QString, QList<QWidget*>>       dependents_;

    QVBoxLayout* devices_ = nullptr;      // when MIDI is on show
    QVBoxLayout* assignments_ = nullptr;
    QString      midiSignature_;
    QTimer       midiPoll_;
};
