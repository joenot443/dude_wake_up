#include "SettingsDialog.h"
#include "UpdateCheck.h"

#include "EngineBridge.h"
#include "Icons.h"
#include "Prefs.h"
#include "Screenshots.h"
#include "Theme.h"
#include "ui/Controls.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

namespace {
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

// A rounded group of rows (the macOS grouped form), with an optional heading.
QVBoxLayout* group(const QString& title, QWidget* content, QVBoxLayout* layout) {
    if (!title.isEmpty()) {
        auto* heading = new QLabel(title, content);
        heading->setStyleSheet(QString("color: %1; font-size: 9pt; font-weight: 600; padding: 8px 4px 2px 4px;")
                                   .arg(Theme::textSecondary.name()));
        layout->addWidget(heading);
    }
    auto* box = new QFrame(content);
    box->setObjectName("Group");
    auto* v = new QVBoxLayout(box);
    v->setContentsMargins(12, 10, 12, 10);
    v->setSpacing(10);
    layout->addWidget(box);
    return v;
}

void clear(QVBoxLayout* layout) {
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

QString formatValue(const SettingInfo& s, double v) {
    if (s.unit == "ms") return v >= 1000 ? QString::number(v / 1000, 'f', 2) + " s" : QString::number(qRound(v)) + " ms";
    const QString n = QString::number(v, 'f', s.decimals);
    return s.unit.isEmpty() ? n : s.unit == "%" ? n + "%" : n + " " + s.unit;
}

bool shown(const SettingInfo& s) {
    if (!Prefs::shown(s)) return false;
    if (s.id == "updates.check" && !UpdateCheck::supported()) return false;  // Store copies update through the Store
    return true;
}

QString appDataDir() { return qApp->property("appDataDir").toString(); }

// A row whose setting depends on another: greyed as a whole (labels too)
// while that one is off.
void setRowEnabled(QWidget* row, bool on) {
    row->setEnabled(on);
    auto* effect = qobject_cast<QGraphicsOpacityEffect*>(row->graphicsEffect());
    if (!effect) row->setGraphicsEffect(effect = new QGraphicsOpacityEffect(row));
    effect->setOpacity(on ? 1.0 : 0.4);
    effect->setEnabled(!on);
}
} // anon

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Settings");
    setObjectName("Settings");
    setStyleSheet(QString("#Settings { background: %1; }"
                          "QFrame#Group { background: %2; border-radius: 8px; }"
                          "QFrame#Sidebar { background: %2; border-right: 1px solid %3; }"
                          "QListWidget { background: transparent; border: none; outline: none; }"
                          "QListWidget::item { color: %4; padding: 6px 10px; border-radius: 6px; }"
                          "QListWidget::item:hover { background: %5; }"
                          "QListWidget::item:selected { background: %6; color: %4; }"
                          "QFrame#Restart { background: %6; border-radius: 8px; }")
                      .arg(Theme::background.name(), Theme::backgroundSecondary.name(), Theme::borderSubtle.name(),
                           Theme::textPrimary.name(), Theme::surfaceHover.name(), Theme::accentSubtle.name()));
    resize(760, 560);
    setMinimumSize(560, 400);

    auto* outer = new QHBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    // Sidebar: search and the sections.
    auto* sidebar = new QFrame(this);
    sidebar->setObjectName("Sidebar");
    sidebar->setFixedWidth(190);
    auto* side = new QVBoxLayout(sidebar);
    side->setContentsMargins(10, 12, 10, 12);
    side->setSpacing(8);
    search_ = new QLineEdit(sidebar);
    search_->setPlaceholderText("Search");
    search_->setClearButtonEnabled(true);
    side->addWidget(search_);
    sections_ = new QListWidget(sidebar);
    side->addWidget(sections_, 1);
    outer->addWidget(sidebar);

    auto* main = new QVBoxLayout;
    main->setContentsMargins(0, 0, 0, 0);
    main->setSpacing(0);
    restartBanner_ = new QFrame(this);
    restartBanner_->setObjectName("Restart");
    auto* banner = new QHBoxLayout(restartBanner_);
    banner->setContentsMargins(12, 8, 8, 8);
    auto* bannerText = new QLabel("The new theme is used after a restart.", restartBanner_);
    bannerText->setStyleSheet(QString("color: %1;").arg(Theme::textPrimary.name()));
    auto* restart = new QPushButton("Restart Now", restartBanner_);
    restart->setAutoDefault(false);
    connect(restart, &QPushButton::clicked, this, &SettingsDialog::restartRequested);
    banner->addWidget(bannerText, 1);
    banner->addWidget(restart);
    auto* bannerMargin = new QVBoxLayout;
    bannerMargin->setContentsMargins(16, 12, 16, 0);
    bannerMargin->addWidget(restartBanner_);
    main->addLayout(bannerMargin);
    scroll_ = new QScrollArea(this);
    scroll_->setWidgetResizable(true);
    scroll_->setFrameShape(QFrame::NoFrame);
    scroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    main->addWidget(scroll_, 1);
    outer->addLayout(main, 1);

    for (const SettingInfo& s : Prefs::instance()->settings())
        if (shown(s) && !sectionNames_.contains(s.section)) sectionNames_ << s.section;
    sections_->addItems(sectionNames_);
    sections_->setCurrentRow(0);
    // Picking a section ends a search; while searching no section is highlighted.
    auto pick = [this] {
        if (!search_->text().isEmpty()) search_->clear();  // (which builds)
        else build();
    };
    connect(sections_, &QListWidget::currentRowChanged, this, pick);
    connect(sections_, &QListWidget::itemPressed, this, [this] { if (!search_->text().isEmpty()) search_->clear(); });
    connect(search_, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (QListWidgetItem* item = sections_->currentItem()) item->setSelected(text.isEmpty());
        build();
    });
    connect(Prefs::instance(), &Prefs::changed, this, &SettingsDialog::settingChanged);

    midiPoll_.setInterval(1000);
    connect(&midiPoll_, &QTimer::timeout, this, &SettingsDialog::refreshMidi);
    build();
    updateRestartBanner();
}

void SettingsDialog::showEvent(QShowEvent*) {
    for (const auto& refresh : std::as_const(refreshers_)) refresh();
    for (auto it = dependents_.cbegin(); it != dependents_.cend(); ++it)
        for (QWidget* w : it.value()) setRowEnabled(w, Prefs::boolean(it.key()));
    updateRestartBanner();
    midiSignature_ = "-";
    refreshMidi();
    midiPoll_.start();
}

void SettingsDialog::hideEvent(QHideEvent*) { midiPoll_.stop(); }

void SettingsDialog::build() {
    refreshers_.clear();
    dependents_.clear();
    devices_ = assignments_ = nullptr;

    auto* content = new QWidget(scroll_);
    content->setObjectName("Settings");
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(16, 12, 16, 16);
    layout->setSpacing(6);

    const QStringList terms = search_->text().toLower().split(' ', Qt::SkipEmptyParts);
    const bool searching = !terms.isEmpty();
    auto matches = [&](const QString& text) {
        const QString t = text.toLower();
        for (const QString& term : terms) if (!t.contains(term)) return false;
        return true;
    };
    const QStringList shownSections = searching ? sectionNames_ : QStringList{sectionNames_.value(sections_->currentRow())};

    bool any = false;
    for (const QString& section : shownSections) {
        QVector<const SettingInfo*> rows;
        for (const SettingInfo& s : Prefs::instance()->settings()) {
            if (s.section != section || !shown(s)) continue;
            if (searching && !matches(QStringList{s.label, s.help, s.keywords, s.section, s.group}.join(' '))) continue;
            rows << &s;
        }
        const bool midiExtras = section == "MIDI" && (!searching || matches("midi devices assignments controller"));
        if (rows.isEmpty() && !midiExtras) continue;
        any = true;

        auto* title = new QLabel(section, content);
        title->setStyleSheet(searching ? QString("color: %1; font-size: 10pt; font-weight: 600; padding: 10px 2px 0 2px;").arg(Theme::textPrimary.name())
                                       : QString("color: %1; font-size: 14pt; font-weight: 600; padding: 4px 2px 4px 2px;").arg(Theme::textPrimary.name()));
        layout->addWidget(title);

        QVBoxLayout* box = nullptr;
        QString boxGroup;
        for (const SettingInfo* s : rows) {
            if (!box || s->group != boxGroup) {
                box = group(s->group, content, layout);
                boxGroup = s->group;
            }
            box->addWidget(rowFor(*s, content));
        }

        if (midiExtras) {
            devices_ = group("MIDI Devices", content, layout);
            assignments_ = group("MIDI Assignments", content, layout);
        }
        if (!searching && section == "Screenshots")
            layout->addWidget(caption("In a preview window: S to save, Shift+S to choose the folder, C to copy.", content));

        bool resettable = false;
        for (const SettingInfo* s : rows) resettable |= s->type != SettingInfo::Action && s->type != SettingInfo::Path;
        if (!searching && resettable) {
            auto* reset = new QPushButton(QString("Reset %1 to Defaults").arg(section), content);
            reset->setAutoDefault(false);
            connect(reset, &QPushButton::clicked, this, [section] { EngineBridge::instance()->resetSettings(section); });
            auto* h = new QHBoxLayout;
            h->setContentsMargins(0, 8, 0, 0);
            h->addStretch(1);
            h->addWidget(reset);
            layout->addLayout(h);
        }
    }
    if (!any) layout->addWidget(caption(QString("No settings match “%1”.").arg(search_->text().trimmed()), content));
    layout->addStretch(1);

    scroll_->setWidget(content);  // (deletes the previous page)
    for (auto it = dependents_.cbegin(); it != dependents_.cend(); ++it)
        for (QWidget* w : it.value()) setRowEnabled(w, Prefs::boolean(it.key()));
    if (devices_) {
        midiSignature_ = "-";
        refreshMidi();
    }
}

QWidget* SettingsDialog::rowFor(const SettingInfo& s, QWidget* parent) {
    auto* w = new QWidget(parent);
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(3);
    const QString id = s.id;
    std::function<void()> refresh;

    switch (s.type) {
    case SettingInfo::Bool: {
        auto* toggle = new DSToggle({}, w);
        connect(toggle, &DSToggle::toggled, this, [id](bool on) { Prefs::set(id, on ? 1.0 : 0.0); });
        refresh = [toggle, id] { toggle->setChecked(Prefs::boolean(id)); };
        v->addWidget(row(s.label, toggle, w));
        break;
    }
    case SettingInfo::Int:
    case SettingInfo::Float: {
        auto* box = new QWidget(w);
        auto* h = new QHBoxLayout(box);
        h->setContentsMargins(0, 0, 0, 0);
        auto* slider = new DSSlider(box);
        slider->setRange(float(s.min), float(s.max));
        slider->setStep(float(s.type == SettingInfo::Int ? std::max(1.0, s.step) : s.step));
        slider->setDecimals(s.decimals);
        slider->setMinimumWidth(180);
        auto* value = new QLabel(box);
        value->setFixedWidth(60);
        value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        value->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textSecondary.name()));
        h->addWidget(slider, 1);
        h->addWidget(value);
        connect(slider, &DSSlider::valueChanged, this, [id, value, info = s](float x) {
            value->setText(formatValue(info, x));
            Prefs::set(id, double(x));
        });
        refresh = [slider, value, id, info = s] {
            const double x = Prefs::number(id);
            if (!slider->isDragging()) slider->setValue(float(x));
            value->setText(formatValue(info, x));
        };
        v->addWidget(row(s.label, box, w));
        break;
    }
    case SettingInfo::Choice: {
        auto* picker = new DSPicker(w);
        picker->setOptions(s.choiceLabels);
        connect(picker, &DSPicker::currentIndexChanged, this, [id, ids = s.choiceIds](int i) {
            if (i >= 0 && i < ids.size()) Prefs::set(id, ids[i]);
        });
        refresh = [picker, id, ids = s.choiceIds] { picker->setCurrentIndex(int(ids.indexOf(Prefs::string(id)))); };
        v->addWidget(row(s.label, picker, w));
        break;
    }
    case SettingInfo::Path: {
        auto* box = new QWidget(w);
        auto* h = new QHBoxLayout(box);
        h->setContentsMargins(0, 0, 0, 0);
        auto* path = new QLabel(box);
        path->setMinimumWidth(140);
        auto* change = new QPushButton("Change…", box);
        change->setAutoDefault(false);
        connect(change, &QPushButton::clicked, this, [this, id] {
            if (id == "screenshots.folder") { Screenshots::chooseFolder(this); return; }
            const QString f = QFileDialog::getExistingDirectory(this, "Choose a folder", Prefs::string(id));
            if (!f.isEmpty()) Prefs::set(id, f);
        });
        h->addWidget(path, 1);
        h->addWidget(change);
        refresh = [path, id] {
            const QString folder = id == "screenshots.folder" ? Screenshots::folder() : Prefs::string(id);
            const QString none = id == "screenshots.folder" ? QStringLiteral("Not set, chosen on first capture") : QStringLiteral("Not set");
            const QString shown = folder.isEmpty() ? none : QDir::toNativeSeparators(folder);
            path->setText(path->fontMetrics().elidedText(shown, folder.isEmpty() ? Qt::ElideRight : Qt::ElideLeft, 220));
            path->setToolTip(folder.isEmpty() ? QString() : shown);
            path->setStyleSheet(QString("color: %1; font-size: 8pt;").arg((folder.isEmpty() ? Theme::textSecondary : Theme::textPrimary).name()));
        };
        v->addWidget(row(s.label, box, w));
        break;
    }
    case SettingInfo::String: {
        auto* edit = new QLineEdit(w);
        edit->setMinimumWidth(180);
        connect(edit, &QLineEdit::editingFinished, this, [edit, id] { Prefs::set(id, edit->text()); });
        refresh = [edit, id] { if (!edit->hasFocus()) edit->setText(Prefs::string(id)); };
        v->addWidget(row(s.label, edit, w));
        break;
    }
    case SettingInfo::Action: {
        auto* button = new QPushButton(s.label, w);
        button->setAutoDefault(false);
        connect(button, &QPushButton::clicked, this, [this, id] { perform(id); });
        auto* h = new QHBoxLayout;
        h->setContentsMargins(0, 0, 0, 0);
        h->addWidget(button);
        h->addStretch(1);
        v->addLayout(h);
        break;
    }
    }

    QString help = s.help;
    if (s.restart & 2) help += (help.isEmpty() ? "" : " ") + QStringLiteral("Applies after a restart.");
    if (!help.isEmpty()) v->addWidget(caption(help, w));
    if (refresh) {
        refresh();
        refreshers_.insert(id, refresh);
    }
    if (!s.dependsOn.isEmpty()) dependents_[s.dependsOn] << w;
    return w;
}

void SettingsDialog::perform(const QString& action) {
    if (action == "advanced.openLogs") {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QDir(appDataDir()).filePath("logs")));
    } else if (action == "advanced.openData") {
        QDesktopServices::openUrl(QUrl::fromLocalFile(appDataDir()));
    } else if (action == "advanced.resetAll") {
        const auto answer = QMessageBox::question(this, "Reset All Settings",
                                                  "Put every setting back to its default? Workspaces aren't affected.",
                                                  QMessageBox::Reset | QMessageBox::Cancel, QMessageBox::Cancel);
        if (answer == QMessageBox::Reset) EngineBridge::instance()->resetSettings();
    }
}

void SettingsDialog::settingChanged(const QString& id) {
    if (auto it = refreshers_.constFind(id); it != refreshers_.cend()) it.value()();
    if (auto it = dependents_.constFind(id); it != dependents_.cend())
        for (QWidget* w : it.value()) setRowEnabled(w, Prefs::boolean(id));
    if (id == "screenshots.folder" || id == "screenshots.format")  // (the folder row shows the folder if it exists)
        if (auto f = refreshers_.constFind("screenshots.folder"); f != refreshers_.cend()) f.value()();
    updateRestartBanner();
}

void SettingsDialog::updateRestartBanner() { restartBanner_->setVisible(Prefs::instance()->restartPending()); }

void SettingsDialog::refreshMidi() {
    if (!devices_ || !assignments_) return;
    auto* eng = EngineBridge::instance();
    const auto inputs = eng->midiInputs();
    const auto assignments = eng->midiAssignments();
    QString signature;
    for (const auto& i : inputs) signature += i.name + (i.open ? "+" : "-") + ';';
    signature += '|';
    for (const auto& a : assignments) signature += a.paramId + a.descriptor + ';';
    if (signature == midiSignature_) return;
    midiSignature_ = signature;

    QWidget* page = scroll_->widget();
    clear(devices_);
    if (inputs.isEmpty()) devices_->addWidget(caption("No MIDI devices connected", page));
    for (const auto& input : inputs) {
        auto* dot = new QLabel(input.open ? "●" : "○", page);
        dot->setStyleSheet(QString("color: %1;").arg((input.open ? Theme::success : Theme::textSecondary).name()));
        devices_->addWidget(row(input.name, dot, page));
    }

    clear(assignments_);
    if (assignments.isEmpty()) assignments_->addWidget(caption("No MIDI assignments", page));
    for (const auto& a : assignments) {
        auto* w = new QWidget(page);
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
