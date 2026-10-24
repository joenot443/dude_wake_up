#include "SourcePanels.h"

#include "Icons.h"
#include "PositionPad.h"
#include "Theme.h"
#include "ui/Controls.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPointer>
#include <QThreadPool>
#include <QToolButton>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

namespace {

QLabel* sectionLabel(const QString& text, QWidget* parent) {
    auto* l = new QLabel(text, parent);
    l->setStyleSheet(QString("color: %1; font-size: 8pt;").arg(Theme::textSecondary.name()));
    return l;
}

QString pillStyle() {
    return QString("QToolButton { background: %1; color: %2; border: none; border-radius: 10px; padding: 3px 9px;"
                   " font-size: 8pt; }"
                   "QToolButton:checked { background: %3; color: white; }"
                   "QToolButton:hover:!checked { background: %4; }")
        .arg(Theme::surface.name(), Theme::textPrimary.name(), Theme::accent.name(), Theme::surfaceHover.name());
}

// 48px icon thumbnails, decoded once per path off the GUI thread.
QHash<QString, QIcon>& thumbnailCache() {
    static QHash<QString, QIcon> cache;
    return cache;
}

} // anon

// ---------------------------------------------------------------- text

TextSourcePanel::TextSourcePanel(const QString& sourceId, const TextSourceState& state, QWidget* parent)
    : QWidget(parent), sourceId_(sourceId) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->setSpacing(10);
    auto* eng = EngineBridge::instance();

    layout->addWidget(sectionLabel("Text", this));
    text_ = new QPlainTextEdit(state.text, this);
    text_->setFixedHeight(72);
    text_->setStyleSheet(QString("QPlainTextEdit { background: %1; border: 1px solid %2; border-radius: 4px; color: %3; }")
                             .arg(Theme::surface.name(), Theme::border.name(), Theme::textPrimary.name()));
    connect(text_, &QPlainTextEdit::textChanged, this, [this, eng] {
        if (text_->hasFocus()) eng->setTextSourceText(sourceId_, text_->toPlainText());
    });
    layout->addWidget(text_);

    font_ = new DSPicker(this);
    font_->setOptions(state.fonts);
    font_->setCurrentIndex(state.fontIndex);
    connect(font_, &DSPicker::currentIndexChanged, this, [this, eng](int i) { eng->setTextSourceFont(sourceId_, i); });
    if (!state.fonts.isEmpty()) {
        auto* row = new QHBoxLayout;
        row->addWidget(sectionLabel("Font", this));
        row->addStretch(1);
        row->addWidget(font_);
        layout->addLayout(row);
    } else {
        font_->hide();
    }

    size_ = new DSSlider(this);
    size_->setLabel("Size");
    size_->setShowValue(true);
    size_->setDecimals(0);
    size_->setRange(8, 400);
    size_->setStep(1);
    size_->setValue(state.fontSize);
    connect(size_, &DSSlider::valueChanged, this, [this, eng](float v) { eng->setTextSourceFontSize(sourceId_, int(v)); });
    layout->addWidget(size_);

    auto* posRow = new QHBoxLayout;
    posRow->addWidget(sectionLabel("Position", this));
    posRow->addStretch(1);
    auto* center = new QToolButton(this);
    center->setText("Center");
    center->setCursor(Qt::PointingHandCursor);
    center->setStyleSheet(QString("QToolButton { background: %1; color: %2; border: 1px solid %3; border-radius: 9px;"
                                  " padding: 2px 8px; font-size: 8pt; }"
                                  "QToolButton:hover { background: %4; }")
                              .arg(Theme::surface.name(), Theme::textPrimary.name(), Theme::border.name(),
                                   Theme::surfaceHover.name()));
    connect(center, &QToolButton::clicked, this, [this, eng] { eng->centerTextSource(sourceId_); });
    coords_ = sectionLabel({}, this);
    coords_->setStyleSheet(QString("color: %1; font-size: 8pt; font-family: Consolas;").arg(Theme::textTertiary.name()));
    posRow->addWidget(center);
    posRow->addWidget(coords_);
    layout->addLayout(posRow);
    pad_ = new PositionPad(sourceId_, this);
    connect(pad_, &PositionPad::moved, this, [this, eng](float x, float y) {
        eng->setTextSourcePosition(sourceId_, x, y);
        coords_->setText(QString("%1, %2").arg(x, 0, 'f', 2).arg(y, 0, 'f', 2));
    });
    layout->addWidget(pad_);
    setState(state);
}

void TextSourcePanel::setState(const TextSourceState& s) {
    if (!text_->hasFocus() && text_->toPlainText() != s.text) text_->setPlainText(s.text);
    font_->setCurrentIndex(s.fontIndex);
    size_->setValue(s.fontSize);
    pad_->setPosition(s.x, s.y);
    if (!pad_->isDragging()) coords_->setText(QString("%1, %2").arg(s.x, 0, 'f', 2).arg(s.y, 0, 'f', 2));
}

// ---------------------------------------------------------------- file

namespace {
QString formatTime(float seconds) {
    const int total = std::max(0, int(std::isfinite(seconds) ? seconds : 0));
    return QString("%1:%2").arg(total / 60).arg(total % 60, 2, 10, QChar('0'));
}
}

FileSourcePanel::FileSourcePanel(const QString& sourceId, const FileSourceState& state, QWidget* parent)
    : QWidget(parent), sourceId_(sourceId), state_(state) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->setSpacing(10);
    auto* eng = EngineBridge::instance();

    auto* top = new QHBoxLayout;
    top->setSpacing(8);
    play_ = new QToolButton(this);
    play_->setFixedSize(24, 24);
    play_->setIconSize(QSize(14, 14));
    play_->setAutoRaise(true);
    play_->setCursor(Qt::PointingHandCursor);
    connect(play_, &QToolButton::clicked, this, [this, eng] {
        state_.playing = !state_.playing;
        eng->setFileSourcePlaying(sourceId_, state_.playing);
        setState(state_);
    });
    auto* scrub = new QVBoxLayout;
    scrub->setSpacing(2);
    position_ = new DSSlider(this);
    position_->setRange(0, 1);
    connect(position_, &DSSlider::valueChanged, this, [this, eng](float p) {
        eng->setFileSourcePosition(sourceId_, p);
        elapsed_->setText(formatTime(p * state_.duration));
    });
    auto* times = new QHBoxLayout;
    elapsed_ = sectionLabel({}, this);
    total_ = sectionLabel({}, this);
    times->addWidget(elapsed_);
    times->addStretch(1);
    times->addWidget(total_);
    scrub->addWidget(position_);
    scrub->addLayout(times);
    top->addWidget(play_);
    top->addLayout(scrub, 1);
    layout->addLayout(top);

    // Volume, with a mute toggle (videos start muted).
    auto* volumeRow = new QHBoxLayout;
    volumeRow->setSpacing(8);
    mute_ = new QToolButton(this);
    mute_->setFixedSize(24, 24);
    mute_->setIconSize(QSize(16, 16));
    mute_->setAutoRaise(true);
    mute_->setCursor(Qt::PointingHandCursor);
    connect(mute_, &QToolButton::clicked, this, [this, eng] {
        state_.muted = !state_.muted;
        eng->setFileSourceMuted(sourceId_, state_.muted);
        setState(state_);
    });
    volume_ = new DSSlider(this);
    volume_->setRange(0, 1);
    connect(volume_, &DSSlider::valueChanged, this, [this, eng](float v) {
        state_.volume = v;
        eng->setFileSourceVolume(sourceId_, v);
    });
    volumeRow->addWidget(mute_);
    volumeRow->addWidget(volume_, 1);
    layout->addLayout(volumeRow);

    auto* speedRow = new QHBoxLayout;
    speedRow->setSpacing(8);
    speedLabel_ = sectionLabel({}, this);
    speedLabel_->setFixedWidth(32);
    speedLabel_->setAlignment(Qt::AlignCenter);
    speed_ = new DSSlider(this);
    speed_->setRange(0, 2);
    connect(speed_, &DSSlider::valueChanged, this, [this, eng](float s) {
        eng->setFileSourceSpeed(sourceId_, s);
        speedLabel_->setText(QString("%1x").arg(s, 0, 'f', 1));
    });
    speedRow->addWidget(speedLabel_);
    speedRow->addWidget(speed_, 1);
    layout->addLayout(speedRow);

    // The frame cache (built in the background; scrubbing, looping and
    // reverse are instant once it's complete).
    auto* cacheRow = new QHBoxLayout;
    cacheRow->setSpacing(8);
    cache_ = new DSToggle("Use frame cache", this);
    cache_->setSmall(true);
    connect(cache_, &DSToggle::toggled, this, [this, eng](bool on) {
        state_.cacheEnabled = on;
        eng->setFileSourceUseCache(sourceId_, on);
    });
    cacheStatus_ = sectionLabel({}, this);
    cacheRow->addWidget(cache_);
    cacheRow->addStretch(1);
    cacheRow->addWidget(cacheStatus_);
    layout->addLayout(cacheRow);
    setState(state);
}

void FileSourcePanel::setState(const FileSourceState& s) {
    state_ = s;
    play_->setIcon(Icons::icon(s.playing ? Icons::Pause : Icons::Play, Theme::textPrimary));
    play_->setToolTip(s.playing ? "Pause" : "Play");
    position_->setValue(s.position);
    if (!position_->isDragging()) elapsed_->setText(formatTime(s.position * s.duration));
    total_->setText(formatTime(s.duration));
    speed_->setValue(s.speed);
    if (!speed_->isDragging()) speedLabel_->setText(QString("%1x").arg(s.speed, 0, 'f', 1));
    mute_->setIcon(Icons::icon(s.muted ? Icons::SpeakerMuted : Icons::Speaker, s.muted ? Theme::textSecondary : Theme::textPrimary));
    mute_->setToolTip(s.muted ? "Unmute" : "Mute");
    volume_->setValue(s.volume);
    volume_->setEnabled(!s.muted);
    cache_->setChecked(s.cacheEnabled);
    cache_->setEnabled(s.cacheAvailable);
    cacheStatus_->setText(s.cacheAvailable ? QString("%1 frames cached").arg(s.cachedFrames)
                          : s.cachedFrames > 0 ? QString("Caching… %1 frames").arg(s.cachedFrames)
                                               : QStringLiteral("No cache"));
}

// ---------------------------------------------------------------- icon

IconSourcePanel::IconSourcePanel(const QString& sourceId, const IconSourceState& state, QWidget* parent)
    : QWidget(parent), sourceId_(sourceId), state_(state) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 8, 16, 8);
    layout->setSpacing(8);

    // Category pills.
    auto* pillBox = new QWidget(this);
    pills_ = new QHBoxLayout(pillBox);
    pills_->setContentsMargins(0, 0, 0, 0);
    pills_->setSpacing(4);
    auto* group = new QButtonGroup(this);
    QStringList categories;
    for (const IconEntry& e : state.icons) if (!categories.contains(e.category)) categories << e.category;
    categories.sort(Qt::CaseInsensitive);
    categories.prepend(QString());
    for (const QString& c : categories) {
        auto* pill = new QToolButton(pillBox);
        pill->setText(c.isEmpty() ? "All" : c.left(1).toUpper() + c.mid(1));
        pill->setCheckable(true);
        pill->setChecked(c.isEmpty());
        pill->setCursor(Qt::PointingHandCursor);
        pill->setStyleSheet(pillStyle());
        group->addButton(pill);
        connect(pill, &QToolButton::clicked, this, [this, c] { category_ = c; rebuildGrid(); });
        pills_->addWidget(pill);
    }
    pills_->addStretch(1);
    layout->addWidget(pillBox);

    search_ = new QLineEdit(this);
    search_->setPlaceholderText("Search icons...");
    search_->setClearButtonEnabled(true);
    connect(search_, &QLineEdit::textChanged, this, &IconSourcePanel::rebuildGrid);
    layout->addWidget(search_);

    grid_ = new QListWidget(this);
    grid_->setViewMode(QListView::IconMode);
    grid_->setIconSize(QSize(40, 40));
    grid_->setGridSize(QSize(52, 52));
    grid_->setResizeMode(QListView::Adjust);
    grid_->setMovement(QListView::Static);
    grid_->setUniformItemSizes(true);
    grid_->setFixedHeight(200);
    grid_->setStyleSheet(QString("QListWidget { background: transparent; border: none; }"
                                 "QListWidget::item { background: %1; border-radius: 6px; margin: 2px; }"
                                 "QListWidget::item:selected { background: %1; border: 2px solid %2; }"
                                 "QListWidget::item:hover { background: %3; }")
                             .arg(Theme::surface.name(), Theme::accent.name(), Theme::surfaceHover.name()));
    connect(grid_, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        const int index = item->data(Qt::UserRole).toInt();
        EngineBridge::instance()->setIconSourceIcon(sourceId_, index);
        state_.selected = index;
    });
    layout->addWidget(grid_);
    rebuildGrid();
}

void IconSourcePanel::setSelected(int index) {
    if (index == state_.selected) return;
    state_.selected = index;
    for (int i = 0; i < grid_->count(); ++i)
        grid_->item(i)->setSelected(grid_->item(i)->data(Qt::UserRole).toInt() == index);
}

void IconSourcePanel::rebuildGrid() {
    grid_->clear();
    const QString text = search_->text().trimmed();
    QPointer<IconSourcePanel> self(this);
    for (int i = 0; i < state_.icons.size(); ++i) {
        const IconEntry& e = state_.icons[i];
        if (!category_.isEmpty() && e.category != category_) continue;
        if (!text.isEmpty() && !e.name.contains(text, Qt::CaseInsensitive)) continue;
        auto* item = new QListWidgetItem(grid_);
        item->setToolTip(e.name);
        item->setData(Qt::UserRole, i);
        item->setSelected(i == state_.selected);
        const QString path = e.path;
        auto& cache = thumbnailCache();
        if (cache.contains(path)) {
            item->setIcon(cache.value(path));
            continue;
        }
        cache.insert(path, QIcon());
        QThreadPool::globalInstance()->start([self, path] {
            QImageReader reader(path);
            QSize size = reader.size();
            if (size.isValid()) reader.setScaledSize(size.scaled(80, 80, Qt::KeepAspectRatio));
            const QImage image = reader.read();
            QMetaObject::invokeMethod(QCoreApplication::instance(), [self, path, image] {
                const QIcon icon(QPixmap::fromImage(image));
                thumbnailCache().insert(path, icon);
                if (!self) return;
                for (int r = 0; r < self->grid_->count(); ++r) {
                    QListWidgetItem* it = self->grid_->item(r);
                    if (self->state_.icons.value(it->data(Qt::UserRole).toInt()).path == path) it->setIcon(icon);
                }
            }, Qt::QueuedConnection);
        });
    }
}
