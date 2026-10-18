#include "SettingsDialog.h"

#include "EngineBridge.h"
#include "Icons.h"
#include "Screenshots.h"
#include "Theme.h"
#include "ui/Controls.h"

#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
const QStringList kResolutions = {"240p", "360p", "480p", "720p", "1080p", "1440p", "4K"};

QLabel* caption(const QString& text, QWidget* parent) {
    auto* l = new QLabel(text, parent);
    l->setWordWrap(true);
    l->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textSecondary.name()));
    return l;
}

// A labelled row: name on the left, control on the right.
QWidget* row(const QString& name, QWidget* control, QWidget* parent) {
    auto* w = new QWidget(parent);
    auto* h = new QHBoxLayout(w);
    h->setContentsMargins(0, 0, 0, 0);
    auto* label = new QLabel(name, w);
    label->setStyleSheet(QString("color: %1;").arg(Theme::textPrimary.name()));
    h->addWidget(label);
    h->addStretch(1);
    h->addWidget(control);
    return w;
}

void clear(QVBoxLayout* layout) {
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

QString formatMs(float ms) {
    return ms >= 1000 ? QString::number(ms / 1000, 'f', 2) + " s" : QString::number(int(ms)) + " ms";
}
} // anon

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Settings");
    setObjectName("Settings");
    setStyleSheet(QString("#Settings { background: %1; }"
                          "QFrame#Group { background: %2; border-radius: 8px; }")
                      .arg(Theme::background.name(), Theme::backgroundSecondary.name()));
    resize(420, 640);
    auto* eng = EngineBridge::instance();

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* content = new QWidget(scroll);
    content->setObjectName("Settings");
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(6);
    scroll->setWidget(content);
    outer->addWidget(scroll);

    // A titled, rounded group of rows (the macOS grouped form).
    auto group = [&](const QString& title) {
        auto* heading = new QLabel(title, content);
        heading->setStyleSheet(QString("color: %1; font-size: 9pt; font-weight: 600; padding: 8px 4px 2px 4px;")
                                   .arg(Theme::textSecondary.name()));
        layout->addWidget(heading);
        auto* box = new QFrame(content);
        box->setObjectName("Group");
        auto* v = new QVBoxLayout(box);
        v->setContentsMargins(12, 10, 12, 10);
        v->setSpacing(10);
        layout->addWidget(box);
        return v;
    };

    // Display.
    QVBoxLayout* display = group("Display");
    resolution_ = new DSPicker(content);
    resolution_->setOptions(kResolutions);
    connect(resolution_, &DSPicker::currentIndexChanged, this, [eng](int i) { eng->setResolutionIndex(i); });
    display->addWidget(row("Resolution", resolution_, content));
    hints_ = new DSToggle({}, content);
    connect(hints_, &DSToggle::toggled, this, [this](bool on) {
        QSettings().setValue("ui/showNodeHints", on);
        emit showHintsChanged(on);
    });
    display->addWidget(row("Show Node Hints", hints_, content));
    advance_ = new DSToggle({}, content);
    connect(advance_, &DSToggle::toggled, this, [](bool on) { QSettings().setValue("ui/autoAdvanceOutputWindow", on); });
    display->addWidget(row("Automatically advance output window", advance_, content));
    display->addWidget(caption("A preview window follows its node's output onto an effect you connect after it.", content));

    // Screenshots.
    QVBoxLayout* shots = group("Screenshots");
    auto* folderBox = new QWidget(content);
    auto* folderRow = new QHBoxLayout(folderBox);
    folderRow->setContentsMargins(0, 0, 0, 0);
    folder_ = new QLabel(folderBox);
    folder_->setMinimumWidth(120);
    auto* change = new QPushButton("Change…", folderBox);
    change->setAutoDefault(false);
    connect(change, &QPushButton::clicked, this, [this] { Screenshots::chooseFolder(this); refresh(); });
    folderRow->addWidget(folder_, 1);
    folderRow->addWidget(change);
    shots->addWidget(row("Folder", folderBox, content));
    format_ = new DSPicker(content);
    format_->setOptions({"PNG", "JPEG"});
    connect(format_, &DSPicker::currentIndexChanged, this, [](int i) { Screenshots::setFormat(i == 1 ? "jpg" : "png"); });
    shots->addWidget(row("Format", format_, content));
    shots->addWidget(caption("In a preview window: S to save, Shift+S to choose folder, C to copy.", content));

    // MIDI smoothing.
    QVBoxLayout* smoothing = group("MIDI Smoothing");
    smoothing_ = new DSToggle({}, content);
    smoothing->addWidget(row("Enable Smoothing", smoothing_, content));
    auto* durationBox = new QWidget(content);
    auto* durationRow = new QHBoxLayout(durationBox);
    durationRow->setContentsMargins(0, 0, 0, 0);
    smoothingMs_ = new DSSlider(durationBox);
    smoothingMs_->setRange(0, 4000);
    smoothingMs_->setMinimumWidth(160);
    smoothingLabel_ = new QLabel(durationBox);
    smoothingLabel_->setFixedWidth(56);
    smoothingLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    smoothingLabel_->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textSecondary.name()));
    durationRow->addWidget(smoothingMs_, 1);
    durationRow->addWidget(smoothingLabel_);
    smoothing->addWidget(row("Duration", durationBox, content));
    smoothing->addWidget(caption("Eases MIDI knob/fader changes over the chosen duration. Helpful for 7-bit CC stepping; "
                                 "bypassed for toggles.", content));
    connect(smoothing_, &DSToggle::toggled, this, [this, eng](bool on) {
        eng->setMidiSmoothingEnabled(on);
        smoothingMs_->setEnabled(on);
    });
    connect(smoothingMs_, &DSSlider::valueChanged, this, [this, eng](float ms) {
        eng->setMidiSmoothingMs(ms);
        smoothingLabel_->setText(formatMs(ms));
    });

    // MIDI devices and assignments, rebuilt when they change.
    devices_ = group("MIDI Devices");
    assignments_ = group("MIDI Assignments");
    layout->addStretch(1);

    midiPoll_.setInterval(1000);
    connect(&midiPoll_, &QTimer::timeout, this, &SettingsDialog::refreshMidi);
}

void SettingsDialog::showEvent(QShowEvent*) {
    refresh();
    midiSignature_ = "-";
    refreshMidi();
    midiPoll_.start();
}

void SettingsDialog::hideEvent(QHideEvent*) { midiPoll_.stop(); }

void SettingsDialog::refresh() {
    auto* eng = EngineBridge::instance();
    QSettings settings;
    resolution_->setCurrentIndex(eng->resolutionIndex());
    hints_->setChecked(settings.value("ui/showNodeHints", true).toBool());
    advance_->setChecked(settings.value("ui/autoAdvanceOutputWindow", true).toBool());
    const QString folder = Screenshots::folder();
    const QString shown = folder.isEmpty() ? QStringLiteral("Not set — chosen on first capture") : QDir::toNativeSeparators(folder);
    folder_->setText(folder_->fontMetrics().elidedText(shown, folder.isEmpty() ? Qt::ElideRight : Qt::ElideLeft, 200));
    folder_->setToolTip(folder.isEmpty() ? QString() : shown);
    folder_->setStyleSheet(QString("color: %1; font-size: 8pt;").arg((folder.isEmpty() ? Theme::textSecondary : Theme::textPrimary).name()));
    format_->setCurrentIndex(Screenshots::format() == "jpg" ? 1 : 0);
    const bool smoothing = eng->midiSmoothingEnabled();
    smoothing_->setChecked(smoothing);
    smoothingMs_->setValue(eng->midiSmoothingMs());
    smoothingMs_->setEnabled(smoothing);
    smoothingLabel_->setText(formatMs(eng->midiSmoothingMs()));
}

void SettingsDialog::refreshMidi() {
    auto* eng = EngineBridge::instance();
    const auto inputs = eng->midiInputs();
    const auto assignments = eng->midiAssignments();
    QString signature;
    for (const auto& i : inputs) signature += i.name + (i.open ? "+" : "-") + ';';
    signature += '|';
    for (const auto& a : assignments) signature += a.paramId + a.descriptor + ';';
    if (signature == midiSignature_) return;
    midiSignature_ = signature;

    clear(devices_);
    if (inputs.isEmpty()) devices_->addWidget(caption("No MIDI devices connected", this));
    for (const auto& input : inputs) {
        auto* dot = new QLabel(input.open ? "●" : "○", this);
        dot->setStyleSheet(QString("color: %1;").arg((input.open ? Theme::success : Theme::textSecondary).name()));
        devices_->addWidget(row(input.name, dot, this));
    }

    clear(assignments_);
    if (assignments.isEmpty()) assignments_->addWidget(caption("No MIDI assignments", this));
    for (const auto& a : assignments) {
        auto* w = new QWidget(this);
        auto* h = new QHBoxLayout(w);
        h->setContentsMargins(0, 0, 0, 0);
        auto* names = new QVBoxLayout;
        names->setSpacing(2);
        auto* title = new QLabel(a.ownerName + " / " + a.paramName, w);
        title->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textPrimary.name()));
        names->addWidget(title);
        names->addWidget(caption(a.descriptor, w));
        h->addLayout(names, 1);
        auto* remove = new QToolButton(w);
        remove->setIcon(Icons::icon(Icons::Trash, Theme::destructive));
        remove->setToolTip("Remove this assignment");
        remove->setStyleSheet("QToolButton { background: transparent; border: none; }");
        connect(remove, &QToolButton::clicked, this, [this, id = a.paramId] {
            EngineBridge::instance()->removeMidiBinding(id);
            refreshMidi();
        });
        h->addWidget(remove);
        assignments_->addWidget(w);
    }
}
