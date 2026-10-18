#include "Controls.h"

#include "Icons.h"
#include "Theme.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <functional>

namespace {
constexpr int kThumb = 14;        // theme sliderThumbSize
constexpr qreal kTrack = 3;       // theme sliderTrackHeight
constexpr int kHeaderHeight = 16;
constexpr int kHeaderGap = 4;

QFont captionFont(const QFont& base, bool mono = false) {
    QFont f = base;
    f.setPixelSize(11);
    if (mono) { f.setFamily("Consolas"); f.setWeight(QFont::Medium); }
    return f;
}

// Commits on Enter or focus loss; Esc cancels.
class InlineEditor : public QLineEdit {
public:
    using QLineEdit::QLineEdit;
    std::function<void(bool commit)> done;
protected:
    void keyPressEvent(QKeyEvent* e) override {
        if (e->key() == Qt::Key_Escape) { finish(false); return; }
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) { finish(true); return; }
        QLineEdit::keyPressEvent(e);
    }
    void focusOutEvent(QFocusEvent* e) override {
        QLineEdit::focusOutEvent(e);
        finish(true);
    }
private:
    void finish(bool commit) {
        if (!done) return;
        auto callback = std::move(done);
        done = nullptr;
        callback(commit);
    }
};
} // anon

// ------------------------------------------------------------------ slider

DSSlider::DSSlider(QWidget* parent) : QWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
}

void DSSlider::setRange(float min, float max) {
    min_ = min; max_ = max;
    value_ = std::clamp(value_, std::min(min_, max_), std::max(min_, max_));
    update();
}

void DSSlider::setLabel(const QString& label) { label_ = label; updateGeometry(); update(); }
void DSSlider::setShowValue(bool on) { showValue_ = on; updateGeometry(); update(); }

void DSSlider::setValue(float value) {
    if (dragging_) return;  // the user's drag wins
    value_ = value;
    update();
}

QSize DSSlider::sizeHint() const {
    return QSize(120, kThumb + (hasHeader() ? kHeaderHeight + kHeaderGap : 0));
}

QRectF DSSlider::trackRect() const {
    const qreal top = hasHeader() ? kHeaderHeight + kHeaderGap : 0;
    return QRectF(0, top, width(), kThumb);
}

QRect DSSlider::valueRect() const {
    return QRect(width() - 60, 0, 60, kHeaderHeight);
}

void DSSlider::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    if (hasHeader()) {
        p.setFont(captionFont(font()));
        p.setPen(Theme::textSecondary);
        p.drawText(QRect(0, 0, width() - 60, kHeaderHeight), Qt::AlignLeft | Qt::AlignVCenter, label_);
        if (showValue_ && !editor_) {
            p.setFont(captionFont(font(), true));
            p.setPen(Theme::textTertiary);
            p.drawText(valueRect(), Qt::AlignRight | Qt::AlignVCenter, QString::number(value_, 'f', decimals_));
        }
    }
    const QRectF tr = trackRect();
    const qreal span = max_ - min_;
    const qreal frac = span != 0 ? std::clamp(qreal(value_ - min_) / span, 0.0, 1.0) : 0;
    const qreal usable = tr.width() - kThumb;
    const qreal thumbX = tr.left() + frac * usable;
    const qreal cy = tr.center().y();
    p.setPen(Qt::NoPen);
    p.setBrush(Theme::backgroundTertiary);
    p.drawRoundedRect(QRectF(tr.left(), cy - kTrack / 2, tr.width(), kTrack), kTrack / 2, kTrack / 2);
    p.setBrush(dragging_ ? Theme::accentHover : Theme::accent);
    p.drawRoundedRect(QRectF(tr.left(), cy - kTrack / 2, std::max(kTrack, thumbX - tr.left() + kThumb / 2.0), kTrack),
                      kTrack / 2, kTrack / 2);
    const qreal r = (dragging_ ? kThumb * 1.15 : kThumb) / 2;
    p.setBrush(QColor(0, 0, 0, dragging_ ? 75 : 40));
    p.drawEllipse(QPointF(thumbX + kThumb / 2.0, cy + 1), r, r);
    p.setBrush(Theme::accent);
    p.drawEllipse(QPointF(thumbX + kThumb / 2.0, cy), r, r);
}

void DSSlider::setFromX(qreal x) {
    const QRectF tr = trackRect();
    const qreal frac = std::clamp((x - tr.left() - kThumb / 2.0) / std::max(1.0, tr.width() - kThumb), 0.0, 1.0);
    float v = float(min_ + frac * (max_ - min_));
    if (step_ > 0) v = std::clamp(std::round(v / step_) * step_, std::min(min_, max_), std::max(min_, max_));
    if (v == value_) return;
    value_ = v;
    update();
    emit valueChanged(v);
}

void DSSlider::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;
    if (showValue_ && valueRect().contains(e->position().toPoint())) { editValue(); return; }
    if (e->position().y() < trackRect().top() - 2) return;
    dragging_ = true;
    setFromX(e->position().x());
    update();
}

void DSSlider::mouseMoveEvent(QMouseEvent* e) {
    if (dragging_) setFromX(e->position().x());
}

void DSSlider::mouseReleaseEvent(QMouseEvent*) {
    if (!dragging_) return;
    dragging_ = false;
    update();
}

void DSSlider::wheelEvent(QWheelEvent* e) {
    // Fine adjustment: a hundredth of the range per notch (step if set).
    const float notch = e->angleDelta().y() / 120.0f;
    if (notch == 0) { e->ignore(); return; }
    const float delta = (step_ > 0 ? step_ : (max_ - min_) / 100.0f) * notch;
    const float v = std::clamp(value_ + delta, std::min(min_, max_), std::max(min_, max_));
    if (v != value_) { value_ = v; update(); emit valueChanged(v); }
    e->accept();
}

void DSSlider::editValue() {
    auto* editor = new InlineEditor(QString::number(value_, 'f', decimals_), this);
    editor_ = editor;
    editor->setAlignment(Qt::AlignRight);
    editor->setFont(captionFont(font(), true));
    editor->setStyleSheet(QString("QLineEdit { background: %1; border: 1px solid %2; border-radius: 4px;"
                                  " color: %3; padding: 0 3px; }")
                              .arg(Theme::backgroundTertiary.name(), Theme::accent.name(), Theme::textPrimary.name()));
    editor->setGeometry(valueRect().adjusted(4, -1, 0, 1));
    editor->selectAll();
    editor->show();
    editor->setFocus();
    editor->done = [this, editor](bool commit) {
        if (commit) {
            bool ok = false;
            const float v = editor->text().toFloat(&ok);
            if (ok) {
                // Typed values may exceed the slider's range, as on macOS.
                value_ = v;
                emit valueChanged(v);
            }
        }
        editor_ = nullptr;
        editor->deleteLater();
        update();
    };
    update();
}

// ------------------------------------------------------------------ toggle

DSToggle::DSToggle(const QString& label, QWidget* parent) : QWidget(parent), label_(label) {
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void DSToggle::setChecked(bool on) { checked_ = on; update(); }

QSize DSToggle::sizeHint() const {
    const int trackW = small_ ? 28 : 36, trackH = small_ ? 16 : 20;
    const int labelW = label_.isEmpty() ? 0 : QFontMetrics(captionFont(font())).horizontalAdvance(label_) + 8;
    return QSize(labelW + trackW, trackH);
}

void DSToggle::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const int trackW = small_ ? 28 : 36, trackH = small_ ? 16 : 20, thumb = small_ ? 12 : 16;
    int x = 0;
    if (!label_.isEmpty()) {
        p.setFont(captionFont(font()));
        p.setPen(Theme::textSecondary);
        const int w = QFontMetrics(p.font()).horizontalAdvance(label_);
        p.drawText(QRect(0, 0, w, height()), Qt::AlignVCenter | Qt::AlignLeft, label_);
        x = w + 8;
    }
    const QRectF track(x, (height() - trackH) / 2.0, trackW, trackH);
    p.setPen(Qt::NoPen);
    p.setBrush(checked_ ? Theme::accent : Theme::backgroundTertiary);
    p.drawRoundedRect(track, trackH / 2.0, trackH / 2.0);
    const qreal inset = (trackH - thumb) / 2.0;
    const qreal tx = checked_ ? track.right() - inset - thumb : track.left() + inset;
    p.setBrush(QColor(0, 0, 0, 50));
    p.drawEllipse(QRectF(tx, track.top() + inset + 1, thumb, thumb));
    p.setBrush(Qt::white);
    p.drawEllipse(QRectF(tx, track.top() + inset, thumb, thumb));
}

void DSToggle::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton || !rect().contains(e->position().toPoint())) return;
    checked_ = !checked_;
    update();
    emit toggled(checked_);
}

// ------------------------------------------------------------------ picker

DSPicker::DSPicker(QWidget* parent) : QToolButton(parent) {
    setCursor(Qt::PointingHandCursor);
    setIcon(Icons::icon(Icons::ChevronDown, Theme::textTertiary));
    setIconSize(QSize(9, 9));
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setLayoutDirection(Qt::RightToLeft);
    setStyleSheet(QString("QToolButton { border: none; border-radius: 4px; padding: 2px 6px; color: %1;"
                          " background: transparent; font-size: 8pt; }"
                          "QToolButton:hover { background: %2; }")
                      .arg(Theme::textPrimary.name(), Theme::surfaceHover.name()));
    connect(this, &QToolButton::clicked, this, [this] {
        QMenu menu(this);
        for (int i = 0; i < options_.size(); ++i) {
            QAction* a = menu.addAction(options_[i], this, [this, i] {
                setCurrentIndex(i);
                emit currentIndexChanged(i);
            });
            a->setCheckable(true);
            a->setChecked(i == index_);
        }
        menu.exec(mapToGlobal(QPoint(0, height())));
    });
}

void DSPicker::setOptions(const QStringList& options) {
    options_ = options;
    setCurrentIndex(index_);
}

void DSPicker::setCurrentIndex(int index) {
    index_ = index;
    setText(index >= 0 && index < options_.size() ? options_[index] : QString());
}
